#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <texttool.h>
#include <sgtty.h>
#include <fcntl.h>
#include <gno/gno.h>
#include "screen.h"

#pragma stacksize 2048

struct pane pane;

int numchars;
char bufr[1024];

void doflush(void)
{
    write(STDOUT_FILENO, bufr, numchars);
    numchars=0;
}

void usage(void)
{
    fprintf(stderr,"usage: evt ttyname\n");
    exit(1);
}

main(int argc, char *argv[])
{
struct sgttyb s,ms;
int done = 0,oa,gnoq;
char b[2];
byte *kbm =(byte *) 0xE1C025l;
int chars;
int mdfd;

    if (!_INITGNOSTDIO()) {
	 fprintf(stderr,"evt requires GNO/ME\n");
         exit(1);
    }
    if (argc < 2) usage();

    ioctl(STDIN_FILENO,TIOCGETP,&s);
    s.sg_flags |= RAW;
    s.sg_flags &= ~CRMOD;
    ioctl(STDIN_FILENO,TIOCSETP,&s);
    
    mdfd = open(argv[1],O_RDWR);
    if (mdfd < 0) { fprintf(stderr,"evt: can't open tty %s\n",argv[1]);
	    exit(1);
    }

    ioctl(mdfd,TIOCGETP,&ms);
    ms.sg_flags |= RAW;
    ioctl(mdfd,TIOCSETP,&ms);
    
    b[1] = 0;
    tcap_init();
    em_init(&pane);
    while (!done) {
	ioctl(STDIN_FILENO,FIONREAD,&chars);
        if (chars) {
            read(STDIN_FILENO, b, 1);
            oa = *kbm & 0x80;
            if ((oa) && (toupper(b[0]) == 'Q')) done = 1;
            else
                write(mdfd,b,1);
        }
	ioctl(mdfd,FIONREAD,&chars);
        if (chars) {
            read(mdfd, b, 1);
            if ((*b < 5) || (*b > 6))
                em_write(&pane,b,1);
	    doflush();
        }
    }

    s.sg_flags &= ~RAW;
    s.sg_flags |= CRMOD;
    ioctl(STDIN_FILENO,TIOCSETP,&s);
    ms.sg_flags &= ~RAW;
    ioctl(mdfd,TIOCSETP,&ms);
}
