#include <stdio.h>
#include <fcntl.h>
#include <sgtty.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <types.h>
#include <unistd.h>
#include <texttool.h>
#include <sys/signal.h>
#include <gno/gno.h>

char inbuf[32];
char obf[64];

#define FD_SET(f,r) (r.fds_bits[0] |= (1 << f))
#define FD_ISSET(f,r) (r.fds_bits[0] & (1 << f))

byte  *CH = (byte *)0x57bl,*CV = (byte *)0x25l;

int portfd;

void gotoxy(int x,int y)
{
    WriteChar(0x1E);
    WriteChar(x+32);
    WriteChar(y+32);
}

void setport(int l, int r, int t, int b)
{
    WriteChar(3);
    WriteChar('[');
    WriteChar(l+32);
    WriteChar(r+32);
    WriteChar(t+32);
    WriteChar(b+32);
}

void proc1(void)
{
int x;
static int cx = 0, cy = 0;
static int oind,i;

        ioctl(portfd,FIONREAD,&x);
        if (!x) x = 1;
        if (x > 32) x = 32;
        x = read(portfd,inbuf,x);
	setport(0,79,0,19);
	gotoxy(cx,cy);
	oind = 0;
        for (i = 0; i < x; i++) {
            switch (inbuf[i]) {
#if 0
                case '\n': write(STDOUT_FILENO,obf,oind); oind = 0;
                           cy = MIN(19,++cy);
                	   gotoxy(cx,cy);
                           break;
                case '\r': write(STDOUT_FILENO,obf,oind); oind = 0;
                	   cx = 0;
                	   gotoxy(cx,cy);
                           break;        
#endif
                case '\n': break;
                case '\r': obf[oind++] = inbuf[i];
                           obf[oind++] = '\n';
	                   break;
                default:   if (inbuf[i] < 32) break;
                	   /*cx++; if (cx > 79) { cx = 0; cy = MIN(19,++cy); } */
	                   obf[oind++] = inbuf[i];
                           break;
            }
        }
        write(STDOUT_FILENO,obf,oind);
        cx = *CH; cy = *CV;
}

char kbuf[240];

void proc2(void)
{
int x;
static int cx = 0;
char ch;

    gotoxy(0,21);
        x = read(STDIN_FILENO,&ch,1);
	setport(0,79,21,23);
        gotoxy(cx % 80, (cx / 80)+21);
        switch (ch) {
            case 8:
            case 0x7f:  if (cx) {
		          cx--;
                          WriteChar(8);
                          WriteChar(' ');
                          WriteChar(8);
                        } else WriteChar(7);
                        break;
            case '\n':
            case '\r':  kbuf[cx++] = '\r';
         		write(portfd,kbuf,cx);
                       	cx = 0;
                       	gotoxy(0,21);
                       	WriteChar(0xb);
                       	break;
            default:	if (cx < 239) {
            		    kbuf[cx++] = ch;
		            WriteChar(ch);
			} else WriteChar(7);
                        break;
        }
}

void sig(int a, int b)
{
struct sgttyb s;
    setport(0,79,0,23);
    gotoxy(0,23);
    gtty(STDIN_FILENO,&s);
    s.sg_flags &= ~CBREAK;
    s.sg_flags |= ECHO;
    s.sg_flags |= CRMOD;
    stty(STDIN_FILENO,&s);
    exit(0);
}

char xline[] = "____________________";

int main(int argc, char *argv[])
{
int cpid;
struct sgttyb s;
unsigned quitChat = 0;
fd_set a;
int xx;

    signal(SIGINT, sig);
    portfd = open(argv[1],O_RDWR);
    gtty(portfd,&s);
    s.sg_flags &= ~ECHO;
    s.sg_flags |= CBREAK;
    s.sg_flags &= ~CRMOD;
    stty(portfd,&s);
    gtty(STDIN_FILENO,&s);
    s.sg_flags |= CBREAK;
    s.sg_flags &= ~ECHO;
    s.sg_flags &= ~CRMOD;
    stty(STDIN_FILENO,&s);

    WriteChar(12);
    gotoxy(0,20);
    for (cpid = 0; cpid < 4; cpid++)
        write(STDOUT_FILENO,xline,20);

    while (!quitChat) {
	a.fds_bits[0] = 0l;
	FD_SET(STDIN_FILENO,a);
	FD_SET(portfd,a);
	xx = select(32,&a,NULL,NULL,NULL);
	if (xx == -1) {
	    fprintf(stderr,"select() failed\n");
	    sig(0,0);
	}
	if (FD_ISSET(portfd,a)) {
	    proc1();
	}
	if (FD_ISSET(STDIN_FILENO,a)) {
            proc2();
	}
    }

}
