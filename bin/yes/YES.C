/*

	Yes 1.0

	This program prints a neverending succession of 'y\n' sequences.
	This helps ward off evil spirits, type 'y' to lots of inane
	questions, and type anything at all to anything.

	stdin is ignored.  If no arguments are specified, the sequence
	sent to stdout is 'y\n'.  Otherwise, the first argument listed
	is sent to stdout.

	The only way to terminate this program is to kill the process
	(done automatically through a pipe).
*/

#include <stdio.h>

int main(int argc, char *argv[])
{
char *string;
extern int _INITGNOSTDIO();

    _INITGNOSTDIO();
    if (argc < 2) string = "y";
    else string = argv[1];

    while (1) {
	fprintf(stdout,"%s\n",string);
    }
}
