/*
 *	remlf.c
 *
 *	Remove all linefeeds from a file.  This should be useful for
 *	converting IBM PC style CRLF text files to the more reasonable
 *	CR files employed by Apple equipment
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#define X_BUFSIZE 16384

#pragma optimize -1

int main(int argc, char *argv[])
{
char *inbuf,*outbuf;
int infd, outfd;
char *inptr, *outptr;
unsigned cnt, xfer;
char c;

    inbuf = malloc(X_BUFSIZE);
    outbuf = malloc(X_BUFSIZE);

    infd = open(argv[1],O_RDONLY);
    outfd = open(argv[2],O_WRONLY|O_CREAT,0666);

    while (1) {
    	cnt = xfer = read(infd,inbuf,X_BUFSIZE);
 	inptr = inbuf; outptr = outbuf;
	while (cnt--) {
	    if ((c = *inptr++) != '\n')
	        *outptr++ = c;
	}
	write(outfd,outbuf,outptr-outbuf);
	if (xfer != X_BUFSIZE) break;
    }
    close(infd);
    close(outfd);
}
