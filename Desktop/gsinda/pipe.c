#include "pipe.h"

#include <orca.h>
#include <gsos.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sgtty.h>
#include <shell.h>
#include <string.h>
#include <misctool.h>
#include <texttool.h>
#include <fcntl.h>
#include <QuickDraw.h>

#pragma lint -1

extern int errno;
int fd[2], fd1[2], fd2[2];
int cpid,pid1,status,status1,status2;
int master, slave;
char dumb[256];
int ptyno;

char intToHex(int i)
{
    if (i < 10) return ('0'+i);
    else return ('a'+i-10);
}

/* the shell is executed here */
#pragma databank 1
void producer(void)
{
int cl[2];
char *ptyname = ".ttyq0";

    /* we must not have access to ANY other ttys */
    close(0); /* close ALL open files */
    ptyname[5] = intToHex(ptyno);
    slave = open(ptyname,O_RDWR); /* file descriptor 1 */
    dup(slave); /* fd 2 */
    dup(slave); /* fd 3 */
    SetOutputDevice(3,2l);
    SetErrorDevice(3,3l);
    SetInputDevice(3,1l);

    /* Set up Job Control on this terminal */
    tcnewpgrp(1);
    settpgrp(1);

    WriteCString("Welcome to GNO GSI\r\n");
    /*printf("Welcome to ");
    fprintf(stderr,"GNO GSI\n");*/
    execve(":bin:gsh","gsh -f");
    printf("Could not locate :bin:gsh : %d",errno);
}
#pragma databank 0

char ringBuf[1024];
extern myTEFormat teF;

void toOut(char *thisBuf, long size, CtlRecHndl teH) {
/* should also make sure that we are at end of TE selection first */
    teF.theStyleList[0].styleFontID.fidRec.fontStyle = 0x00;
    teF.theStyleList[0].foreColor = 0x0000;
    teF.theStyles[0].dataLength = size;
    TEInsert(0x0001,thisBuf,size,0,(Ref)&teF,teH);
}

void toError(char *thisBuf, long size, CtlRecHndl teH) {
/* should also make sure that we are at end of TE selection first */
    teF.theStyleList[0].styleFontID.fidRec.fontStyle = 0x01;
    teF.theStyleList[0].foreColor = 0x2222;
    teF.theStyles[0].dataLength = size;
    TEInsert(0x0001,thisBuf,size,0,(Ref)&teF,teH);
}

void updateWind(void) {
}

byte *busy = (byte *)0xE100FFl;
void updateWind1(int fio,int fio1) {
static unsigned total = 0;


#if 0
    sprintf(dumb,"%d<-%d   %d<-%d   %d<-%d     ",fd[0],fd[1],fd1[0],fd1[1],
      fd2[0],fd2[1]);
    MoveTo(5,22);
    DrawCString(dumb);
#endif
    total += fio;
    sprintf(dumb,"%d %d (%d) %d  ",fio,fio1,total,*busy);
    MoveTo(5,22);
    DrawCString(dumb);
}

void consume(CtlRecHndl teH)
{
char ch;
int fio,fio1,i;

    ioctl(master,FIONREAD,&fio);
    if (fio) {
        if (fio > 256) fio = 256;
        fio1 = read(master,ringBuf,fio);
        if (fio != fio1) SysBeep();
    	ringBuf[fio] = 0;
    	toOut(ringBuf,fio,teH);
    	updateWind1(fio,fio1);
    }
}

void initPipe(void)
{
int cl[2];
long l;
struct sgttyb sb;
char *ptyname = ".ptyq0";
unsigned i;

/* We have to open the master first, to init the structures - opening an
   unopened pty no longer puts the caller to sleep */

    for (i = 0; i < 2; i++) {
        ptyname[5] = intToHex(i);
        master = open(ptyname,O_RDWR);
        if (master > 0) break;
    }
    if (master < 1) { SysBeep(); return; }
    ptyno = i;
    pid1 = fork(producer);
    /* The Master side of a PTY is pretty darn raw. The master read
       is standard, but we won't tell them that, eh? */
}

void sendchar(char *charPtr)
{
    write(master,charPtr,1);
}

void closePipe(void)
{
int cl[2];

    close(master);
    /*kill(pid1,9);*/
}
