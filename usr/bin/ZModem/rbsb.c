/*
 *
 *  Rev 10-30-91
 *  This file contains Unix specific code for setting terminal modes,
 *  very little is specific to ZMODEM or YMODEM per se (that code is in
 *  sz.c and rz.c).  The CRC-16 routines used by XMODEM, YMODEM, and ZMODEM
 *  are also in this file, a fast table driven macro version
 *
 *	V7/BSD HACKERS:  SEE NOTES UNDER mode(2) !!!
 *
 *   This file is #included so the main file can set parameters such as HOWMANY.
 *   See the main files (rz.c/sz.c) for compile instructions.
 */

#include "zmodem.h"
#include <unistd.h>
#include <gno/gno.h>

#undef READCHECK

#ifdef __ORCAC__
#pragma noroot
#pragma lint -1
#endif

/*#ifdef ICANON
struct termio oldtty, tty;
#else
struct sgttyb oldtty, tty;
struct tchars oldtch, tch;
#endif */

/*------------------------------------------------------------------*/

/*
 *  The following uses an external rdchk() routine if available,
 *  otherwise defines the function for BSD or fakes it for SYSV.
 */

#ifndef READCHECK
#ifdef FIONREAD
#define READCHECK
/*
 *  Return non 0 iff something to read from io descriptor f
 */
int rdchk(int f)
{
unsigned lf;
/*	static long lf;*/

	ioctl(f, FIONREAD, &lf);
	return (lf);
}

#else		/* FIONREAD */

#ifdef SV
#define READCHECK

int checked = 0;
/*
 * Nonblocking I/O is a bit different in System V, Release 2
 *  Note: this rdchk vsn throws away a byte, OK for ZMODEM
 *  sender because protocol design anticipates this problem.
 */
#define EATSIT
int rdchk(int f)
{
	int lf, savestat;
	static char bchecked;

	savestat = fcntl(f, F_GETFL) ;
	fcntl(f, F_SETFL, savestat | O_NDELAY) ;
	lf = read(f, &bchecked, 1) ;
	fcntl(f, F_SETFL, savestat) ;
	checked = bchecked & 0377;	/* force unsigned byte */
	return(lf) ;
}
#endif
#endif
#endif


static unsigned
getspeed(int code)
{
	register n;

	for (n=0; speeds[n].baudr; ++n)
		if (speeds[n].speedcode == code)
			return speeds[n].baudr;
	return 38400;	/* Assume fifo if ioctl failed */
}

/*
 * mode(n)
 *  3: save old tty stat, set raw mode with flow control
 *  2: set XON/XOFF for sb/sz with ZMODEM or YMODEM-g
 *  1: save old tty stat, set raw mode 
 *  0: restore original tty mode
 */
int mode(int n)
{
	static did0 = FALSE;

	vfile("mode:%d", n);
	switch(n) {
#ifdef USG
	case 2:		/* Un-raw mode used by sz, sb when -g detected */
		if(!did0)
			(void) ioctl(Tty, TCGETA, &oldtty);
		tty = oldtty;

		tty.c_iflag = BRKINT|IXON;

		tty.c_oflag = 0;	/* Transparent output */

		tty.c_cflag &= ~(PARENB|HUPCL);		/* Disable parity */
		tty.c_cflag |= (CREAD|CS8);	/* Set character size = 8 */
		if (Twostop)
			tty.c_cflag |= CSTOPB;	/* Set two stop bits */


#ifdef READCHECK
		tty.c_lflag = Zmodem ? 0 : ISIG;
		tty.c_cc[VINTR] = Zmodem ? -1:030;	/* Interrupt char */
#else
		tty.c_lflag = ISIG;
		tty.c_cc[VINTR] = Zmodem ? 03:030;	/* Interrupt char */
#endif
		tty.c_cc[VQUIT] = -1;			/* Quit char */
#ifdef NFGVMIN
		tty.c_cc[VMIN] = 1;
#else
		tty.c_cc[VMIN] = 3;	 /* This many chars satisfies reads */
#endif
		tty.c_cc[VTIME] = 1;	/* or in this many tenths of seconds */

		(void) ioctl(Tty, TCSETAW, &tty);
		did0 = TRUE;
		return OK;
	case 1:
	case 3:
		if(!did0)
			(void) ioctl(Tty, TCGETA, &oldtty);
		tty = oldtty;

		tty.c_iflag = n==3 ? (IGNBRK|IXOFF) : IGNBRK;

		 /* No echo, crlf mapping, INTR, QUIT, delays, no erase/kill */
		tty.c_lflag &= ~(ECHO | ICANON | ISIG);

		tty.c_oflag = 0;	/* Transparent output */

		tty.c_cflag &= ~PARENB;	/* Same baud rate, disable parity */
		tty.c_cflag |= CS8;	/* Set character size = 8 */
		if (Twostop)
			tty.c_cflag |= CSTOPB;	/* Set two stop bits */
#ifdef NFGVMIN
		tty.c_cc[VMIN] = 1; /* This many chars satisfies reads */
#else
		tty.c_cc[VMIN] = howmany; /* This many chars satisfies reads */
#endif
		tty.c_cc[VTIME] = 1;	/* or in this many tenths of seconds */
		(void) ioctl(Tty, TCSETAW, &tty);
		did0 = TRUE;
		Effbaud = Baudrate = getspeed(tty.c_cflag & CBAUD);
		return OK;
#endif
#ifdef V7
	/*
	 *  NOTE: this should transmit all 8 bits and at the same time
	 *   respond to XOFF/XON flow control.  If no FIONREAD or other
	 *   rdchk() alternative, also must respond to INTRRUPT char
	 *   This doesn't work with V7.  It should work with LLITOUT,
	 *   but LLITOUT was broken on the machine I tried it on.
	 */
	case 2:		/* Un-raw mode used by sz, sb when -g detected */
		if(!did0) {
                        ioctl(Tty, TIOCEXCL, 0);
			ioctl(Tty, TIOCGETP, &oldtty);
			ioctl(Tty, TIOCGETC, &oldtch);
#ifdef LLITOUT
			ioctl(Tty, TIOCLGET, &Locmode);
#endif
		}
		tty = oldtty;
		tch = oldtch;
#ifdef READCHECK
		tch.t_intrc = Zmodem ? -1:030;	/* Interrupt char */
#else
		tch.t_intrc = Zmodem ? 03:030;	/* Interrupt char */
#endif
		tty.sg_flags |= (ODDP|EVENP|CBREAK);
		tty.sg_flags &= ~(ALLDELAY|CRMOD|ECHO|LCASE);
		ioctl(Tty, TIOCSETP, &tty);
		ioctl(Tty, TIOCSETC, &tch);
#ifdef LLITOUT
		ioctl(Tty, TIOCLBIS, &Locbit);
#endif
		bibi(99,-1);	/* un-raw doesn't work w/o lit out */
		did0 = TRUE;
		return OK;
	case 1:
	case 3:
		if(!did0) {
                        ioctl(Tty, TIOCEXCL, 0);
                        vfile("TIOCEXCL done");
			ioctl(Tty, TIOCGETP, &oldtty);
                        vfile("TIOCGETP done");
                        ioctl(Tty, TIOCGETC, &oldtch);
                        vfile("TIOCGETC done");
#ifdef LLITOUT
			ioctl(Tty, TIOCLGET, &Locmode);
#endif
		}
		tty = oldtty;
		tty.sg_flags |= (RAW|TANDEM);
		tty.sg_flags &= ~ECHO;
		ioctl(Tty, TIOCSETP, &tty);
		did0 = TRUE;
		Effbaud = Baudrate = getspeed(tty.sg_ospeed);
		return OK;
#endif
	case 0:
		if(!did0)
			return ERROR;
#ifdef USG
		(void) ioctl(Tty, TCSBRK, 1);	/* Wait for output to drain */
		(void) ioctl(Tty, TCFLSH, 1);	/* Flush input queue */
		(void) ioctl(Tty, TCSETAW, &oldtty);	/* Restore modes */
		(void) ioctl(Tty, TCXONC,1);	/* Restart output */
#endif
#ifdef V7
		ioctl(Tty, TIOCSETP, &oldtty);
		ioctl(Tty, TIOCSETC, &oldtch);
		ioctl(Tty, TIOCNXCL, 0);
#ifdef LLITOUT
		ioctl(Tty, TIOCLSET, &Locmode);
#endif
#endif

		return OK;
	default:
		return ERROR;
	}
}

void sendbrk(void)
{
#ifdef V7
#ifdef TIOCSBRK
#define CANBREAK
	sleep(1);
	ioctl(Tty, TIOCSBRK, 0);
	sleep(1);
	ioctl(Tty, TIOCCBRK, 0);
#endif
#endif
#ifdef USG
#define CANBREAK
	ioctl(Tty, TCSBRK, 0);
#endif
}

/* Initialize tty device for serial file xfer */
void inittty(void)
{
	/*Tty = open(DEVTTY, O_RDWR);*/
	Tty = dup(STDIN_FILENO);
	if (Tty < 0) {
		perror(DEVTTY);  exit(2);
	}
	Ttystream = fdopen(Tty, "wb");
	setbuf(Ttystream, xXbuf);
}

void flushmoc(void)
{
#ifdef FASTOUT
        write(Tty,outbuf,outbufInd);
        outbufInd = 0;
#else
	fflush(Ttystream);
#endif
}
void flushmo(void)
{
#ifdef FASTOUT
        write(Tty,outbuf,outbufInd);
        outbufInd = 0;
#else
	fflush(Ttystream);
#endif
}

/*
 * This version of readline is reasoably well suited for
 * reading many characters.
 *
 * timeout is in tenths of seconds
 */
void alrm(int a, int b)
{
	longjmp(tohere, -1);
}

int readline(int timeout)
{
	int n, numchars;
	static char *cdq;	/* pointer for removing chars from linbuf */

	if (--Lleft >= 0) {
		if (Verbose > 8) {
			fprintf(stderr, "%02x ", *cdq&0377);
		}
		return (*cdq++ & 0377);
	}
	n = timeout/10;
	if (n < 2)
		n = 3;
	if (Verbose > 5)
		fprintf(stderr, "Calling read: alarm=%d  Readnum=%d ",
		  n, Readnum);
	if (setjmp(tohere)) {
#ifdef TIOCFLUSH
  		ioctl(Tty, TIOCFLUSH, 0);
#endif
		Lleft = 0;
		if (Verbose>1)
			fprintf(stderr, "Readline:TIMEOUT\n");
		return TIMEOUT;
	}
        signal(SIGALRM, alrm); alarm(n);
	errno = 0;
	Lleft=read(Tty, cdq=linbuf, Readnum);
	alarm(0);
	if (Verbose > 5) {
		fprintf(stderr, "Read returned %d bytes errno=%d\n",
		  Lleft, errno);
	}
	if (Lleft < 1)
		return TIMEOUT;
	--Lleft;
	if (Verbose > 8) {
		fprintf(stderr, "%02x ", *cdq&0377);
	}
	return (*cdq++ & 0377);
}



/*
 * Purge the modem input queue of all characters
 */
void purgeline(void)
{
	vfile("purgeline()");
    Lleft = 0;
#ifdef __ORCAC__
	ioctl(Tty, TIOCFLUSH, 0);
#else
#ifdef USG
	ioctl(Tty, TCFLSH, 0);
#else
	lseek(Tty, 0L, 2);
#endif
#endif
}


/* send cancel string to get the other end to shut up */
void canit(void)
{
	static char canistr[] = {
	 24,24,24,24,24,24,24,24,24,24,8,8,8,8,8,8,8,8,8,8,0
	};

	zmputs(canistr);
	Lleft=0;	/* Do read next time ... */
}

/*
 * Send a string to the modem, processing for \336 (sleep 1 sec)
 *   and \335 (break signal)
 */
void zmputs(char *s)
{
	register c;

	while (*s) {
		switch (c = *s++) {
		case '\336':
			sleep(1); continue;
		case '\335':
			sendbrk(); continue;
		default:
			sendline(c);
		}
	}
	flushmo();
}


static char dumpit[256];

/* VARARGS1 */
void vfile(char *fmt, ...)
{
va_list list;

    va_start(list,fmt);
    if (Verbose > 2) {
    	vfprintf(stderr,fmt,list);
        fprintf(stderr,"\n");
    }
    else vsprintf(dumpit,fmt,list);
    va_end(list);
    fflush(stderr);
}

FILE *popen(char *name)
{
return;
}

int pclose(FILE *x)
{
	return;
}

int unlink(char *name)
{
	remove(name);
}

/* End of rbsb.c */
