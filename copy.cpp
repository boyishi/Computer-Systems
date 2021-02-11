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
    FIFORequestChannel chan("control", FIFORequestChannel::CLIENT_SIDE);

    /* Sending a datamsg request to the server */
    datamsg d(1, 0, 1);
    chan.cwrite(&d, sizeof(datamsg));

    double r;
    chan.cread(&r, sizeof(double));
    cout << r << endl;

    /* Sending a file request to the server */
    // Step 1. Getting the size of the file
    filemsg f(0, 0);
    char *filename = "10.csv";
    int size_total = sizeof(filemsg) + strlen(filename) + 1;
    char *buf = new char[size_total];

    memcpy(buf, &f, sizeof(filemsg));
    strcpy(buf + sizeof(filemsg), filename);
    chan.cwrite(buf, size_total);
    __int64_t size_file;

    chan.cread(&size_file, sizeof(__int64_t));
    cout << size_file << endl;

    // Step 2. Requesting the data
    // a. Create output file
    string output_path = string("received/") + string(filename);
    FILE *outputfile = fopen(output_path.c_str(), "wb");
    // b. Sets up the buffer and the number of bits transferred each call
    char *rec_buf = new char[MAX_MESSAGE];
    filemsg *fm = (filemsg *)buf;
    fm->length = 256;
    // c. Write to file
    int fullCalls = size_file / fm->length;
    for (int i = 0; i < fullCalls; i++)
    {
        chan.cwrite(buf + (fm->length * i), size_total);
        chan.cread(rec_buf, MAX_MESSAGE);
        fwrite(rec_buf, 1, MAX_MESSAGE, outputfile);
    }

    int bitsRemaining = size_file - (fm->length * fullCalls);
    chan.cwrite(buf + (fm->length * fullCalls), bitsRemaining);
    chan.cread(rec_buf, MAX_MESSAGE);
    fwrite(rec_buf, 1, MAX_MESSAGE, outputfile);
}
