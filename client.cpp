/*
    Tanzir Ahmed
    Department of Computer Science & Engineering
    Texas A&M University
    Date  : 2/8/20
 */
#include "common.h"
#include "FIFOreqchannel.h"
#include <sys/wait.h>
#include <bits/stdc++.h>

using namespace std;

double getPoint(FIFORequestChannel &chan, int person, double time, int ecg_no)
{
	double result;
	datamsg d(person, time, ecg_no);

	chan.cwrite(&d, sizeof(datamsg));
	chan.cread(&result, sizeof(double));

	return result;
}

int main(int argc, char *argv[])
{
	char *filename;
	char *buffer_size = "256";
	__int64_t BUFFER_SIZE = MAX_MESSAGE;
	bool sendPoint = false, sendPoints = false, sendFile = false, newChannel = false;
	struct timeval start, end;
	int opt, person, ecg_no;
	double secs;

	while ((opt = getopt(argc, argv, "cp:t:e:f:m:")) != -1)
	{
		switch (opt)
		{
		case 'c':
			newChannel = true;
			break;
		case 'p':
			person = atoi(optarg);
			sendPoints = true;
			break;
		case 't':
			secs = atof(optarg);
			break;
		case 'e':
			ecg_no = atoi(optarg);
			sendPoint = true;
			sendPoints = false;
			break;
		case 'f':
			filename = optarg;
			sendFile = true;
			break;
		case 'm':
			buffer_size = optarg;
			BUFFER_SIZE = atoi(buffer_size);
			break;
		}
	}

	pid_t pid = fork();
	if (pid == 0)
	{
		char *arglist[] = {"./server", "-m", buffer_size, NULL};
		execvp(arglist[0], arglist);
	}
	else
	{
		FIFORequestChannel chan("control", FIFORequestChannel::CLIENT_SIDE);

		cout << "Client buffer size: " << BUFFER_SIZE << endl;
		if (newChannel)
		{
			cout << "Second Channel Created..." << endl;
			MESSAGE_TYPE msg = NEWCHANNEL_MSG;
			chan.cwrite(&msg, sizeof(MESSAGE_TYPE));
			char _name[BUFFER_SIZE];
			chan.cread(&_name, BUFFER_SIZE);

			FIFORequestChannel secondChan(_name, FIFORequestChannel::CLIENT_SIDE);
			double result = getPoint(secondChan, 1, .004, 2);
			cout << "(1, .004, 2): " << result << endl;
			result = getPoint(secondChan, 2, 12.008, 2);
			cout << "(2, 12.008, 1): " << result << endl;

			MESSAGE_TYPE quitmsg = QUIT_MSG;
			secondChan.cwrite(&quitmsg, sizeof(MESSAGE_TYPE));
		}

		if (sendPoint)
		{
			double result = getPoint(chan, person, secs, ecg_no);
			cout << result << endl;
		}

		if (sendPoints)
		{ // Creating the output file
			ofstream of("received/x1.csv");

			// Starting the timer
			gettimeofday(&start, NULL);
			ios_base::sync_with_stdio(false);

			double curr_time = 0, result = 0;
			for (int iter = 0; iter < 500; iter++)
			{
				for (int ecg_no = 1; ecg_no <= 2; ecg_no++)
				{
					result = getPoint(chan, person, curr_time, ecg_no);
					of << result << " ";
				}
				of << endl;
				curr_time += .004;
			}

			gettimeofday(&end, NULL);

			double time_taken = (end.tv_sec - start.tv_sec) * 1e6;
			time_taken = (time_taken + (end.tv_usec - start.tv_usec)) * 1e-6;
			cout << "Time taken by the program is: " << fixed << time_taken << setprecision(6);
			cout << " sec" << endl;
		}

		if (sendFile)
		{
			gettimeofday(&start, NULL);

			filemsg f(0, 0);
			int size_total = sizeof(filemsg) + strlen(filename) + 1;
			char *buf = new char[size_total];

			memcpy(buf, &f, sizeof(filemsg));
			strcpy(buf + sizeof(filemsg), filename);

			__int64_t fileSize;
			chan.cwrite(buf, size_total);
			chan.cread(&fileSize, sizeof(__int64_t));

			// Creating the buffer
			char rec_buf[BUFFER_SIZE];

			// Creating the output file
			string path = "received/" + string(filename);
			FILE *outputfile = fopen(path.c_str(), "wb");

			// Updating filemsg length
			filemsg *fm = (filemsg *)buf;
			fm->length = BUFFER_SIZE;
			// Writing to the file
			__int64_t windowEnd;
			for (windowEnd = BUFFER_SIZE; windowEnd <= fileSize; windowEnd += BUFFER_SIZE)
			{
				fm->offset = windowEnd - BUFFER_SIZE;
				chan.cwrite(buf, size_total);
				chan.cread(rec_buf, BUFFER_SIZE);
				fwrite(rec_buf, 1, BUFFER_SIZE, outputfile);
			}

			int remaining = fileSize % BUFFER_SIZE;

			if (remaining > 0)
			{
				fm->offset = windowEnd - BUFFER_SIZE;
				fm->length = remaining;

				chan.cwrite(buf, size_total);
				chan.cread(rec_buf, remaining);
				fwrite(rec_buf, 1, remaining, outputfile);
			}

			gettimeofday(&end, NULL);

			double time_taken = (end.tv_sec - start.tv_sec) * 1e6;
			time_taken = (time_taken + (end.tv_usec - start.tv_usec)) * 1e-6;
			cout << "Time taken by the program is: " << fixed << time_taken << setprecision(6);
			cout << " sec" << endl;
		}

		MESSAGE_TYPE m = QUIT_MSG;
		chan.cwrite(&m, sizeof(MESSAGE_TYPE));
		wait(NULL);
	}
}
