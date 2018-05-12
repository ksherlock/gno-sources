/*
 *	connect.c
 *
 *	Utilize a transaction script to connect to a remote host
 *	via GNO TTY's.
 *	Compile with ORCA/C 1.3 or C 2.0.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sgtty.h>

int tfd; /* terminal file descriptor */

void usage(void)
{
    fprintf(stderr,"usage: connect conn-script\n");
    exit(1);
}

waitmark(char *s)
{
int len = strlen(s);
static char inbuf[256];
unsigned mark = 0;
int needed,got;
char *found;
char beep[1] = {7};

    needed = len;
    while (needed--) {
        read(tfd,inbuf+mark,1);
        write(2,inbuf+mark,1);
        mark++;
    }
again:
    if (!strncmp(inbuf,s,len)) {
        write(2,&beep,1);
        return;
    }
    memmove(inbuf,inbuf+1,len-1);
    read(tfd,inbuf+len-1,1);
    write(2,inbuf+len-1,1);
    goto again;
}

/* wait until the specified string has come across the tty */
oldwaitmark(char *s)
{
int len = strlen(s);
static char inbuf[256];
static unsigned mark = 0;
int needed,got;
char *found;

    while (1) {
again:
	needed = len-mark;
        got = read(tfd,inbuf+mark,needed);
        write(2,inbuf+mark,got);
        printf("needed: %d got: %d\n",needed,got);
        mark += got;
        if (got < needed) goto again;
        inbuf[mark] = 0;
        /*write(2,inbuf,mark);*/
        if ((found = strstr(inbuf,s)) != NULL) {
	    memmove(inbuf,found+len,(mark - (found-inbuf) + len));
            mark -= ((found-inbuf)+len);
	    return;
	} else {
	    memmove(inbuf,inbuf+(mark-len),len);
            mark -= len;
        }
    }
}

void doPrint(char *s)
{
static char buf[80];
int bufind = 0;

    while (*s) {
        if (*s == '\\') {
            s++;
            switch (*s) {
            	case '\\': buf[bufind++] = '\\'; break;
                case 'n' : buf[bufind++] = 10; break;
                case 'r' : buf[bufind++] = 13; break;
                case 0 : continue; /* we hit the end of the string */
                default: break;
            }
            s++;
        } else {
            buf[bufind++] = *s;
            s++;
        }
    }
    write(tfd,buf,bufind);
}

int doCommand(char *cmd)
{
int cmdStatus = 1;
int ssecs;

    switch (cmd[0]) {
	case 'w': waitmark(cmd+2);
	          cmdStatus = 1;
	          break;
        case 'p': doPrint(cmd+2);
	          cmdStatus = 1;
        	  break;
	case 's': sscanf(cmd+2,"%d",&ssecs);
		  sleep(ssecs);
		  break;
    }
}

int main(int argc, char *argv[])
{
FILE *f;
static char line[128];
static struct sgttyb s;

    if (argc < 2) usage();
    f = fopen(argv[1],"r");
    tfd = open(argv[2],O_RDWR);
    gtty(tfd,&s);
    s.sg_flags |= RAW;
    s.sg_flags &= ~CRMOD;
    s.sg_flags &= ~ECHO;
    stty(tfd,&s);

    while (!feof(f)) {
        fgets(line,127,f);
        line[strlen(line)-1] = 0; /* remove the nl */
        doCommand(line);
    }
    s.sg_flags |= CRMOD;
    s.sg_flags |= ECHO;
    s.sg_flags &= ~RAW;
    stty(tfd,&s);
    close(tfd);
    fclose(f);
    exit(0);
}
