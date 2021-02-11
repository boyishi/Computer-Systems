/*
    Tanzir Ahmed
    Department of Computer Science & Engineering
    Texas A&M University
    Date  : 2/8/20
 */
#include "common.h"
#include "FIFOreqchannel.h"

using namespace std;

int main(int argc, char *argv[])
{
	pid_t pid = fork();
	if (pid == 0)
	{
		char *arglist[] = {"./server", NULL};
		execvp(arglist[0], arglist);
	}
	else
	{
		bool sendPoint = false, sendFile = false;
		int opt, person, ecg_no;
		double secs;
		char *filename;
		while ((opt = getopt(argc, argv, "p:t:e:f:")) != -1)
		{
			switch (opt)
			{
			case 'p':
				person = atoi(optarg);
				break;
			case 't':
				secs = atof(optarg);
				break;
			case 'e':
				ecg_no = atoi(optarg);
				sendPoint = true;
				break;
			case 'f':
				filename = optarg;
				sendFile = true;
				break;
			}
		}

		FIFORequestChannel chan("control", FIFORequestChannel::CLIENT_SIDE);
		if (sendPoint)
		{
			datamsg d(person, secs, ecg_no);
			chan.cwrite(&d, sizeof(datamsg));

			double result;
			chan.cread(&result, sizeof(double));
			cout << result << endl;
		}

		MESSAGE_TYPE m = QUIT_MSG;
		chan.cwrite(&m, sizeof(MESSAGE_TYPE));
	}
}
