/*
 *  date.c
 *
 *	usage: date
 *  A trivial little program to display the current date and time.
 */

#pragma stacksize 512
/*#include <stdio.h>*/
#include <time.h>
#include <texttool.h>

void usage(void)
{
    ErrWriteCString("usage: date\r\n");
    exit(1);
}

int main(int argc, char *argv[])
{
time_t t;

    if (argc != 1) usage();
    time(&t);
    WriteCString(ctime(&t));
    WriteChar('\r');
    exit(0);
}
