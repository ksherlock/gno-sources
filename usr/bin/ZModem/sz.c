#pragma stacksize 3072
#pragma optimize 9

#define fileno(x) (x->_file)

/*% cc -compat -M2 -Ox -K -i -DTXBSIZE=16384  -DNFGVMIN -DREADCHECK sz.c -lx -o sz; size sz

<-xtx-*> cc -Osal -DTXBSIZE=32768  -DSV sz.c -lx -o $B/sz; size $B/sz

 ****************************************************************************
 *
 * sz.c By Chuck Forsberg,  Omen Technology INC
 *
 ****************************************************************************
 *
 * Typical Unix/Xenix/Clone compiles:
 *
 *	cc -O sz.c -o sz		USG (SYS III/V) Unix
 *	cc -O -DSV sz.c -o sz		Sys V Release 2 with non-blocking input
 *					Define to allow reverse channel checking
 *	cc -O -DV7  sz.c -o sz		Unix Version 7, 2.8 - 4.3 BSD
 *
 *	cc -O -K -i -DNFGVMIN -DREADCHECK sz.c -lx -o sz	Classic Xenix
 *
 *	ln sz sb			**** All versions ****
 *	ln sz sx			**** All versions ****
 *
 ****************************************************************************
 ****************************************************************************
 *
 *
 * A program for Unix to send files and commands to computers running
 *  Professional-YAM, PowerCom, YAM, IMP, or programs supporting Y/XMODEM.
 *    Copyright 1991 Omen Technology Inc All Rights Reserved
 *
 *  Sz uses buffered I/O to greatly reduce CPU time compared to UMODEM.
 *
 *  USG UNIX (3.0) ioctl conventions courtesy Jeff Martin
 *
 * 
 *	This version implements numerous enhancements including ZMODEM
 *	Run Length Encoding and variable length headers.  These
 *	features were not funded by the original Telenet development
 *	contract.
 * 
 * This software may be freely used for non commercial and
 * educational (didactic only) purposes.  This software may also
 * be freely used to support file transfer operations to or from
 * licensed Omen Technology products.  Any programs which use
 * part or all of this software must be provided in source form
 * with this notice intact except by written permission from Omen
 * Technology Incorporated.
 * 
 * Use of this software for commercial or administrative purposes
 * except when exclusively limited to interfacing Omen Technology
 * products requires a per port license payment of $20.00 US per
 * port (less in quantity).  Use of this code by inclusion,
 * decompilation, reverse engineering or any other means
 * constitutes agreement to these conditions and acceptance of
 * liability to license the materials and payment of reasonable
 * legal costs necessary to enforce this license agreement.
 *
 *
 *		Omen Technology Inc		FAX: 503-621-3745
 *		Post Office Box 4681
 *		Portland OR 97208
 *
 *	This code is made available in the hope it will be useful,
 *	BUT WITHOUT ANY WARRANTY OF ANY KIND OR LIABILITY FOR ANY
 *	DAMAGES OF ANY KIND.
 */


#include "zmodem.h"
char *substr();

/*#ifdef __ORCAC__*/
#include <gno/gno.h>
#include <sys/signal.h>
#include <ctype.h>
#include <unistd.h>
#include <gsbug.h>
#pragma lint -1
#define STATIC
/*#endif*/

#define PATHLEN 256
#define FALSE 0
#ifdef TRUE
#undef TRUE
#endif
#define TRUE 1

STATIC int Zmodem=0;		/* ZMODEM protocol requested by receiver */
unsigned Baudrate=4800;		/* Default, set by first mode() call */
STATIC unsigned Effbaud = 4800;
STATIC unsigned Txwindow;	/* Control the size of the transmitted window */
STATIC unsigned Txwspac;	/* Spacing between zcrcq requests */
STATIC unsigned Txwcnt;	/* Counter used to space ack requests */
STATIC long Lrxpos;		/* Receiver's last reported offset */
STATIC int errors;
char endmsg[80] = {0};	/* Possible message to display on exit */

#pragma lint 0
/*#include "rbsb.c"	/* most of the system dependent stuff here */
/*#include "crctab.c"*/

#pragma lint -1
STATIC int Filesleft;
STATIC long Totalleft;

/*
 * Attention string to be executed by receiver to interrupt streaming data
 *  when an error is detected.  A pause (0336) may be needed before the
 *  ^C (03) or after it.
 *  READCHECK is assumed to be defined
 */

#ifdef USG
STATIC char Myattn[] = { 03, 0336, 0 };
#else
STATIC char Myattn[] = { 0 };
#endif

FILE *in;

#ifdef BADSEEK
STATIC int Canseek = 0;	/* 1: Can seek 0: only rewind -1: neither (pipe) */
#ifndef TXBSIZE
#define TXBSIZE 16384		/* Must be power of two, < MAXINT */
#endif
#else
STATIC int Canseek = 1;	/* 1: Can seek 0: only rewind -1: neither (pipe) */
#endif

#ifdef TXBSIZE
#define TXBMASK (TXBSIZE-1)
STATIC char Txb[TXBSIZE];		/* Circular buffer for file reads */
STATIC char *txbuf = Txb;		/* Pointer to current file segment */
#else
STATIC char txbuf[1024];
#endif
STATIC long vpos = 0;			/* Number of bytes read from file */

STATIC char Lastrx;
STATIC char Crcflg;
STATIC int IsXMODEM=0;		/* XMODEM Protocol - don't send pathnames */
STATIC int Restricted=0;	/* restricted; no /.. or ../ in filenames */
STATIC int Fullname=0;		/* transmit full pathname */
STATIC int Unlinkafter=0;	/* Unlink file after it is sent */
STATIC int Dottoslash=0;	/* Change foo.bar.baz to foo/bar/baz */
STATIC int firstsec;
STATIC int errcnt=0;		/* number of files unreadable */
STATIC int blklen=128;		/* length of transmitted records */
STATIC int Optiong;		/* Let it rip no wait for sector ACK's */
STATIC int Eofseen;		/* EOF seen on input set by zfilbuf */
STATIC int BEofseen;		/* EOF seen on input set by fooseek */
STATIC int Totsecs;		/* total number of sectors this file */
STATIC int Filcnt=0;		/* count of number of files opened */
STATIC unsigned Rxbuflen=16384;	/* Receiver's max buffer length */
STATIC int Tframlen = 0;	/* Override for tx frame length */
STATIC int blkopt=0;		/* Override value for zmodem blklen */
STATIC int Rxflags = 0;
STATIC long bytcnt;
STATIC int Wantfcs32 = TRUE;	/* want to send 32 bit FCS */
STATIC char Lzconv;	/* Local ZMODEM file conversion request */
STATIC char Lzmanag;	/* Local ZMODEM file management request */
STATIC int Lskipnocor;
STATIC char Lztrans;
STATIC int Command;		/* Send a command, then exit. */
STATIC char *Cmdstr;		/* Pointer to the command string */
STATIC int Cmdtries = 11;
STATIC int Cmdack1;		/* Rx ACKs command, then do it */
STATIC int Exitcode;
STATIC int Test;		/* 1= Force receiver to send Attn, etc with qbf. */
			/* 2= Character transparency test */
STATIC char *qbf=
 "The quick brown fox jumped over the lazy dog's back 1234567890\r\n";
STATIC long Lastsync;		/* Last offset to which we got a ZRPOS */
STATIC int Beenhereb4;		/* How many times we've been ZRPOS'd same place */

/*STATIC jmp_buf tohere;  */	/* For the interrupt on RX timeout */
STATIC jmp_buf intrjmp;	/* For the interrupt on RX CAN */

#ifdef XARGSFILE
char *
mystrsave(s)
char *s;
{
	register char *p;
	char *malloc();

	if (p = malloc(strlen(s)+1) ) {
		strcpy(p, s); return p;
	}
	fprintf(stderr, "No memory for mystrsave!\n");
	exit(1);
}

/* Remove (presumably) terminating CR and/or LF from string */
uncrlf(s)
register char *s;
{
	for ( ; *s; ++s)
		switch (*s) {
		case '\r':
		case '\n':
			*s = 0;  return;
		}
}
#endif


/* called by signal interrupt or terminate to clean things up */
void bibi(int n, int code)
{
	canit(); fflush(stdout); mode(0);
	fprintf(stderr, "sz: caught signal %d; exiting\n", n);
	if (n == SIGQUIT)
		abort();
	if (n == 99)
		fprintf(stderr, "mode(2) in rbsb.c not implemented!!\n");
	exit(3);
}
/* Called when ZMODEM gets an interrupt (^X) */
void onintr(int sig, int code)
{
	signal(SIGINT, SIG_IGN);
	longjmp(intrjmp, -1);
}

segment "main      ";

#ifdef XARGSFILE
#define XARGSMAX 256
char *xargv[XARGSMAX+1];
#endif

int main(int argc, char **argv)
{
	register char *cp;
	register npats;
	int dm;
	char **patts;
	extern int howmany;

        Readnum = howmany = 2;

        if ((cp = getenv("ZNULLS")) && *cp)
		Znulls = atoi(cp);
	if ((cp=getenv("SHELL")) && (substr(cp, "rsh") || substr(cp, "rksh")))
		Restricted=TRUE;
	inittty();
	chkinvok(argv[0]);

	Rxtimeout = 600;
	npats=0;
	if (argc<2)
		usage();
	while (--argc) {
		cp = *++argv;
		if (*cp++ == '-' && *cp) {
			while ( *cp) {
				switch(*cp++) {
				case '\\':
					 *cp = toupper(*cp);  continue;
				case '+':
					Lzmanag = ZMAPND; break;
#ifdef CSTOPB
				case '2':
					Twostop = TRUE; break;
#endif
				case 'a':
					if (IsYMODEM || IsXMODEM)
						usage;
					Lzconv = ZCNL;  break;
				case 'b':
					Lzconv = ZCBIN; break;
				case 'C':
					if (--argc < 1) {
						usage();
					}
					Cmdtries = atoi(*++argv);
					break;
				case 'c':
					Lzmanag = ZMCHNG;  break;
				case 'd':
					++Dottoslash;
					/* **** FALL THROUGH TO **** */
				case 'f':
					Fullname=TRUE; break;
				case 'e':
					Zctlesc = 1; break;
				case 'k':
					blklen=1024; break;
				case 'L':
					if (--argc < 1) {
						usage();
					}
					blkopt = atoi(*++argv);
					if (blkopt<24 || blkopt>1024)
						usage();
					break;
				case 'l':
					if (--argc < 1) {
						usage();
					}
					Tframlen = atoi(*++argv);
					if (Tframlen<32 || Tframlen>1024)
						usage();
					break;
				case 'N':
					Lzmanag = ZMNEWL;  break;
				case 'n':
					Lzmanag = ZMNEW;  break;
				case 'o':
					Wantfcs32 = FALSE; break;
				case 'p':
					Lzmanag = ZMPROT;  break;
				case 'r':
					if (Lzconv == ZCRESUM)
						Lzmanag = (Lzmanag & ZMMASK) | ZMCRC;
					Lzconv = ZCRESUM; break;
				case 't':
					if (--argc < 1) {
						usage();
					}
					Rxtimeout = atoi(*++argv);
					if (Rxtimeout<10 || Rxtimeout>1000)
						usage();
					break;
				case 'T':
					if (++Test > 1) {
						chartest(1); chartest(2);
						mode(0);  exit(0);
					}
					break;
				case 'u':
					++Unlinkafter; break;
				case 'v':
					++Verbose; break;
				case 'w':
					if (--argc < 1) {
						usage();
					}
					Txwindow = atoi(*++argv);
					if (Txwindow < 256)
						Txwindow = 256;
					Txwindow = (Txwindow/64) * 64;
					Txwspac = Txwindow/4;
					if (blkopt > Txwspac
					 || (!blkopt && Txwspac < 1024))
						blkopt = Txwspac;
					break;
				case 'Y':
					Lskipnocor = TRUE;
					/* **** FALLL THROUGH TO **** */
				case 'y':
					Lzmanag = ZMCLOB; break;
				case 'Z':
				case 'z':
					Lztrans = ZTRLE;  break;
				default:
					usage();
				}
			}
		}
		else if (Command) {
			if (argc != 1) {
				usage();
			}
			Cmdstr = *argv;
		}
		else if ( !npats && argc>0) {
			if (argv[0][0]) {
				npats=argc;
				patts=argv;
			}
		}
	}
	if (npats < 1 && !Command && !Test) 
		usage();
	if (Verbose) {
	 /*	if (freopen(LOGFILE, "w", stderr)==NULL) {
			printf("Can't open log file %s\n",LOGFILE);
			exit(2);
		}  */
		setvbuf(stderr, NULL,_IONBF,0l);
	}
	vfile("%s %s for %s\n", Progname, VERSION, OS);

#ifdef XARGSFILE
	vfile("npats=%ld *patts=%s", (long)npats, *patts);
	if (npats == 1 && !strcmp(XARGSFILE, *patts)) {
		in = fopen(XARGSFILE, "r");
		if (!in) {
			printf(stderr, "Can't open / control file!\n");
			exit(2);
		}
		for (npats=0,argv=patts=xargv; npats<XARGSMAX; ++npats,++argv) {
			if (fgets(txbuf, 1024, in) <= 0)
				break;
			uncrlf(txbuf);
			*argv = mystrsave(txbuf);
		}
		fclose(in);
	}
#endif

	mode(1);

	if (signal(SIGINT, bibi) == SIG_IGN) {
		signal(SIGINT, SIG_IGN); signal(SIGKILL, SIG_IGN);
	} else {
		signal(SIGINT, bibi); signal(SIGKILL, bibi);
	}
#ifdef SIGQUIT
	signal(SIGQUIT, SIG_IGN);
#endif
#ifdef SIGTERM
	signal(SIGTERM, bibi);
#endif

	if ( !IsXMODEM) {
		if (!IsYMODEM) {
			printf("rz\r");  fflush(stdout);
		}
		countem(npats, patts);
		if (!IsYMODEM) {
			stohdr(0L);
			if (Command)
				Txhdr[ZF0] = ZCOMMAND;
			zshhdr(4, ZRQINIT, Txhdr);
		}
	}
	fflush(stdout);

	if (Command) {
		if (getzrxinit()) {
			Exitcode=1; canit();
		}
		else if (zsendcmd(Cmdstr, 1+strlen(Cmdstr))) {
			Exitcode=1; canit();
		}
	} else if (wcsend(npats, patts)==ERROR) {
		Exitcode=1;
		canit();
	}
	if (endmsg[0])
		printf("  %s: %s\r\n", Progname, endmsg);
	printf("%s %s finished.\r\n", Progname, VERSION);
	fflush(stdout);
	mode(0);
        vfile("hey, what the heck?!?!\n");
        if(errcnt || Exitcode) {
		exit(1);
	}
	exit(0);
	/*NOTREACHED*/
}

int wcsend(int argc, char *argp[])
{
	register n;

	Crcflg=FALSE;
	firstsec=TRUE;
	bytcnt = -1;
	if (IsYMODEM) {
		printf("Start your YMODEM receive. ");  fflush(stdout);
	}
        vfile("argc: %d\n",argc);
        for (n=0; n<argc; ++n) {
		Totsecs = 0;
		if (wcs(argp[n])==ERROR) {
		    vfile("bad news kilroy\n");
                        return ERROR;
                }
	}
	Totsecs = 0;
	if (Filcnt==0) {	/* bitch if we couldn't open ANY files */
		if (!IsYMODEM && !IsXMODEM) {
			Command = TRUE;
			Cmdstr = "echo \"sz: Can't open any requested files\"";
			if (getnak()) {
				Exitcode=1; canit();
			}
			if (!Zmodem)
				canit();
			else if (zsendcmd(Cmdstr, 1+strlen(Cmdstr))) {
				Exitcode=1; canit();
			}
			Exitcode = 1; return OK;
		}
		canit();
		sprintf(endmsg, "Can't open any requested files");
                return ERROR;
	}
	if (Zmodem)
		saybibi();
	else if ( !IsXMODEM)
		wctxpn("");
        return OK;
}

int wcs(char *oname)
{
	register c;
	register char *p, *q;
	struct stat f;
	char name[PATHLEN];

	strcpy(name, oname);

	if (Restricted) {
		/* restrict pathnames to current tree or uucppublic */
		if ( substr(name, "../")
		 || (name[0]== '/' && strncmp(name, PUBDIR, strlen(PUBDIR))) ) {
			canit();  sprintf(endmsg,"Security Violation");
                        return ERROR;
		}
	}
	in=fopen(oname, ROPMODE);

#ifdef __ORCAC__
        setvbuf(in,NULL,_IOFBF,(size_t)32768);
#endif
	if (in==NULL) {
		++errcnt;
                return OK;	/* pass over it, there may be others */
	}
	BEofseen = Eofseen = 0;  vpos = 0;

	/* Check for directory or block special files */
	fstat(fileno(in), &f);
	c = f.st_mode & S_IFMT;
	if (c == S_IFDIR || c == S_IFBLK) {
		fclose(in);
                return OK;
	}

	++Filcnt;
	switch (wctxpn(name)) {
	case ERROR:
                vfile("wctxpn failed\n");
                return ERROR;
	case ZSKIP:
		return OK;
	}
	if (!Zmodem && wctx(f.st_size)==ERROR) {
	    vfile("zmodem: %d or wctx failed\n",Zmodem);
	    return ERROR;
        }
	if (Unlinkafter)
		unlink(oname);

	return 0;
}

/*
 * generate and transmit pathname block consisting of
 *  pathname (null terminated),
 *  file length, mode time and file mode in octal
 *  as provided by the Unix fstat call.
 *  N.B.: modifies the passed name, may extend it!
 */
int wctxpn(char *name)
{
	register char *p, *q;
	char name2[PATHLEN];
	struct stat f;
	int x;

	if (IsXMODEM) {
		if (*name && fstat(fileno(in), &f)!= -1) {
			fprintf(stderr, "Sending %s, %ld XMODEM blocks. ",
			  name, (127+f.st_size)>>7);
		}
		fprintf(stderr, "Give your local XMODEM receive command now.\r\n");
		fflush(stderr);
		return OK;
	}
	zperr("Awaiting pathname nak for %s", *name?name:"<END>");
	if ( !Zmodem)
		if (getnak()) {
		        vfile("getnak() failed\n");
			return ERROR;
        }
        q = (char *) 0;
	if (Dottoslash) {		/* change . to . */
		for (p=name; *p; ++p) {
			if (*p == '/')
				q = p;
			else if (*p == '.')
				*(q=p) = '/';
		}
		if (q && strlen(++q) > 8) {	/* If name>8 chars */
			q += 8;			/*   make it .ext */
			strcpy(name2, q);	/* save excess of name */
			*q = '.';
			strcpy(++q, name2);	/* add it back */
		}
	}

	for (p=name, q=txbuf ; *p; )
		if ((*q++ = *p++) == '/' && !Fullname)
			q = txbuf;
	*q++ = 0;
	p=q;
	/*while (q < (txbuf + 1024))
		*q++ = 0;  */
	fprintf(stderr,"memcpy: %08lX\n",(size_t)((txbuf+1024)-q));
        memset(q,0,(txbuf+1024)-q);
        if (*name) {
		if (fstat(fileno(in), &f)!= -1)
			sprintf(p, "%lu %lo %o 0 %d %ld", f.st_size, f.st_mtime,
			  f.st_mode, Filesleft, Totalleft);
		Totalleft -= f.st_size;
	}
	if (--Filesleft <= 0)
		Filesleft = Totalleft = 0;
	if (Totalleft < 0)
		Totalleft = 0;

	/* force 1k blocks if name won't fit in 128 byte block */
	if (txbuf[125])
		blklen=1024;
	else {		/* A little goodie for IMP/KMD */
		txbuf[127] = (f.st_size + 127) >>7;
		txbuf[126] = (f.st_size + 127) >>15;
	}
	if (Zmodem) {
               x = zsendfile(txbuf, 1+strlen(p)+(p-txbuf));
               if (x) vfile("zsendfile failed\n");
               return x;
        }
        if (wcputsec(txbuf, 0, 128)==ERROR)
		return ERROR;
	return OK;
}

int getnak(void)
{
	register firstch;

	Lastrx = 0;
	for (;;) {
		switch (firstch = readline(800)) {
		case ZPAD:
			if (getzrxinit())
				return ERROR;
			return FALSE;
		case TIMEOUT:
			sprintf(endmsg, "Timeout waiting for ZRINIT");
			return TRUE;
		case WANTG:
#ifdef MODE2OK
			mode(2);	/* Set cbreak, XON/XOFF, etc. */
#endif
			Optiong = TRUE;
			blklen=1024;
		case WANTCRC:
			Crcflg = TRUE;
		case NAK:
			return FALSE;
		case CAN:
			if ((firstch = readline(20)) == CAN && Lastrx == CAN) {
				sprintf(endmsg, "Got CAN waiting to send file");
				return TRUE;
			}
		default:
			break;
		}
		Lastrx = firstch;
	}
}


int wctx(long int flen)
{
	register int thisblklen;
	register int sectnum, attempts, firstch;
	long charssent;

	charssent = 0;  firstsec=TRUE;  thisblklen = blklen;
	vfile("wctx:file length=%ld", flen);

	while ((firstch=readline(Rxtimeout))!=NAK && firstch != WANTCRC
	  && firstch != WANTG && firstch!=TIMEOUT && firstch!=CAN)
		;
	if (firstch==CAN) {
		zperr("Receiver CANcelled");
		return ERROR;
	}
	if (firstch==WANTCRC)
		Crcflg=TRUE;
	if (firstch==WANTG)
		Crcflg=TRUE;
	sectnum=0;
	for (;;) {
		if (flen <= (charssent + 896L))
			thisblklen = 128;
		if ( !filbuf(txbuf, thisblklen))
			break;
		if (wcputsec(txbuf, ++sectnum, thisblklen)==ERROR)
			return ERROR;
		charssent += thisblklen;
	}
	fclose(in);
	attempts=0;
	do {
		purgeline();
		sendline(EOT);
		flushmo();
		++attempts;
	}
		while ((firstch=(readline(Rxtimeout)) != ACK) && attempts < RETRYMAX);
	if (attempts == RETRYMAX) {
		zperr("No ACK on EOT");
		return ERROR;
	}
	else
		return OK;
}

/* data length of this sector to send */
int wcputsec(char *buf, int sectnum, int cseclen)
{
	unsigned int checksum;
	int wcj;
    register char *cp;
	long oldcrc;
	int firstch;
	int attempts;

	firstch=0;	/* part of logic to detect CAN CAN */

	if (Verbose>2)
		fprintf(stderr, "Sector %3d %2dk\n", Totsecs, Totsecs/8 );
	else if (Verbose>1)
		fprintf(stderr, "\rSector %3d %2dk ", Totsecs, Totsecs/8 );
	for (attempts=0; attempts <= RETRYMAX; attempts++) {
		Lastrx= firstch;
		sendline(cseclen==1024?STX:SOH);
		sendline(sectnum);
		sendline(sectnum ^ 0xFF);
        /*sendline(-sectnum -1);*/
		oldcrc=checksum=0;
		for (wcj=cseclen,cp=buf; --wcj>=0; ) {
			sendline(*cp);
			oldcrc=updcrc((0377& *cp), oldcrc);
			checksum += (byte) *cp;
            cp++;
		}
		if (Crcflg) {
			oldcrc=updcrc(0,updcrc(0,oldcrc));
			sendline((int)oldcrc>>8);
			sendline((int)oldcrc);
		}
		else
            sendline(checksum);
		flushmo();

		if (Optiong) {
			firstsec = FALSE; return OK;
		}
		firstch = readline(Rxtimeout);
gotnak:
		switch (firstch) {
		case CAN:
			if(Lastrx == CAN) {
cancan:
				zperr("Cancelled");  return ERROR;
			}
			break;
		case TIMEOUT:
			zperr("Timeout on sector ACK"); continue;
		case WANTCRC:
			if (firstsec)
				Crcflg = TRUE;
		case NAK:
			zperr("NAK on sector"); continue;
		case ACK: 
			firstsec=FALSE;
			Totsecs += (cseclen>>7);
			return OK;
		case ERROR:
			zperr("Got burst for sector ACK"); break;
		default:
			zperr("Got %02lx for sector ACK", (long)firstch); break;
		}
		for (;;) {
			Lastrx = firstch;
			if ((firstch = readline(Rxtimeout)) == TIMEOUT)
				break;
			if (firstch == NAK || firstch == WANTCRC)
				goto gotnak;
			if (firstch == CAN && Lastrx == CAN)
				goto cancan;
		}
	}
	zperr("Retry Count Exceeded");
	return ERROR;
}

/* fill buf with count chars padding with ^Z for CPM */
int filbuf(char *buf, int count)
{
	register int c, m;

	m = read(fileno(in), buf, count);
	if (m <= 0)
		return 0;
	while (m < count)
		buf[m++] = 032;
	return count;
}

/* Fill buffer with blklen chars */
int zfilbuf(void)
{
	int n;

#ifdef TXBSIZE
	vfile("zfilbuf: bytcnt =%lu vpos=%lu blklen=%ld", bytcnt, vpos, (long)blklen);
	/* We assume request is within buffer, or just beyond */
	txbuf = Txb + (bytcnt & TXBMASK);
	if (vpos <= bytcnt) {
		n = fread(txbuf, 1, blklen, in);

		vpos += n;
		if (n < blklen)
			Eofseen = 1;
		vfile("zfilbuf: n=%ld vpos=%lu Eofseen=%ld", (long)n, vpos, (long)Eofseen);
		return n;
	}
	if (vpos >= (bytcnt+blklen))
		return blklen;
	/* May be a short block if crash recovery etc. */
	Eofseen = BEofseen;
	return (vpos - bytcnt);
#else
	n = fread(txbuf, 1, blklen, in);
	if (n < blklen)
		Eofseen = 1;
	return n;
#endif
}

#ifdef TXBSIZE
#ifdef NOTDEFINEDFUCKER
/* Replacement for brain damaged fseek function.  Returns 0==success */
int fooseek(FILE *fptr, long pos, int whence)
{
	long m, n;

	vfile("fooseek: pos =%lu vpos=%lu Canseek=%ld", pos, vpos, (long)Canseek);
	/* Seek offset < current buffer */
	if (pos < (vpos -TXBSIZE +1024)) {
		BEofseen = 0;
		if (Canseek > 0) {
			vpos = pos & ~TXBMASK;
			if (vpos >= pos)
				vpos -= TXBSIZE;
			if (fseek(fptr, vpos, 0))
				return 1;
		}
		else if (Canseek == 0) {
			if (fseek(fptr, vpos = 0L, 0))
				return 1;
		} else
			return 1;
		while (vpos < pos) {
			n = fread(Txb, (size_t) 1, (size_t) TXBSIZE, fptr);
			vpos += n;
			vfile("n=%ld vpos=%ld", (long)n, vpos);
			if (n < TXBSIZE) {
				BEofseen = 1;
				break;
			}
		}
		vfile("vpos=%ld", vpos);
		return 0;
	}
	/* Seek offset > current buffer (Crash Recovery, etc.) */
	if (pos > vpos) {
		if (Canseek)
			if (fseek(fptr, vpos = (pos & ~TXBMASK), 0))
				return 1;
		while (vpos <= pos) {
			txbuf = Txb + (vpos & TXBMASK);
			m = TXBSIZE - (vpos & TXBMASK);
			vfile("m=%ld vpos=%ld", m,vpos);
			n = fread(txbuf, (size_t) 1, (size_t) m, fptr);
			vfile("n=%ld vpos=%ld", n,vpos);
			vpos += n;
			vfile("bo=%ld m=%ld vpos=%ld", (long)(txbuf-Txb),m,vpos);
			if (n < m) {
				BEofseen = 1;
				break;
			}
		}
		return 0;
	}
	/* Seek offset is within current buffer */
	vfile("within buffer: vpos=%ld", vpos);
	return 0;
}
#define fseek fooseek
#endif
#endif

/*
 * Log an error
 */
/*VARARGS1*/
void zperr(char *fmt, ...)
{
va_list list;

    va_start(list, fmt);
    fprintf(stderr, "Retry %d: ", errors);
    vfprintf(stderr,fmt,list);
    fprintf(stderr, "\n");
    fflush(stderr);
    va_end(list);
}

/*
 * substr(string, token) searches for token in string s
 * returns pointer to token within string if found, NULL otherwise
 */
char *
substr(char *s, char *t)
{
	register char *ss,*tt;
	/* search for first char of token */
	for (ss=s; *s; s++)
		if (*s == *t)
			/* compare token with substring */
			for (ss=s,tt=t; ;) {
				if (*tt == 0)
					return s;
				if (*ss++ != *tt++)
					break;
			}
	return NULL;
}

char *usinfo[] = {
	"Send Files and Commands with ZMODEM/YMODEM/XMODEM Protocol\n",
	"Usage:	sz [-2+abcdefklLnNuvwyY] [-] file ...",
	"\t	zcommand [-2Cev] COMMAND",
	"\t	zcommandi [-2Cev] COMMAND",
	"\t	sb [-2adfkuv] [-] file ...",
	"\t	sx [-2akuv] [-] file",
	"\nSee sz.doc for option descriptions and licensing information.",
	""
};

void usage(void)
{
	char **pp;

	for (pp=usinfo; **pp; ++pp)
		fprintf(stderr, "%s\n", *pp);
	fprintf(stderr, "\n%s %s for %s by Chuck Forsberg, Omen Technology INC\n",
	 Progname, VERSION, OS);
	fprintf(stderr, "\t\t\042The High Reliability Software\042\n");
	fprintf(stderr,"\tCopyright 1991 Omen Technology INC All Rights Reserved\n");
	exit(3);
}

/*
 * Get the receiver's init parameters
 */
int getzrxinit(void)
{
	register n;
	struct stat f;

        for (n=10; --n>=0; ) {
		switch (zgethdr(Rxhdr, 1)) {
		case ZCHALLENGE:	/* Echo receiver's challenge numbr */
			stohdr(Rxpos);
			zshhdr(4, ZACK, Txhdr);
			continue;
		case ZCOMMAND:		/* They didn't see our ZRQINIT */
			stohdr(0L);
			zshhdr(4, ZRQINIT, Txhdr);
			continue;
		case ZRINIT:
			Rxflags = 0377 & Rxhdr[ZF0];
			Usevhdrs = Rxhdr[ZF1] & CANVHDR;
			Txfcs32 = (Wantfcs32 && (Rxflags & CANFC32));
			Zctlesc |= Rxflags & TESCCTL;
			Rxbuflen = (0377 & Rxhdr[ZP0])+((0377 & Rxhdr[ZP1])<<8);
			if ( !(Rxflags & CANFDX))
				Txwindow = 0;
			vfile("Rxbuflen=%d Tframlen=%d",Rxbuflen,Tframlen);
			signal(SIGINT, SIG_IGN);
#ifdef MODE2OK
			mode(2);	/* Set cbreak, XON/XOFF, etc. */
#endif

#ifndef READCHECK
#ifndef USG
			/* Use 1024 byte frames if no sample/interrupt */
			if (Rxbuflen < 32 || Rxbuflen > 1024) {
				Rxbuflen = 1024;
				vfile("Rxbuflen=%ld", (long)Rxbuflen);
			}
#endif
#endif

			/* Override to force shorter frame length */
			if (Rxbuflen && (Rxbuflen>Tframlen) && (Tframlen>=32))
				Rxbuflen = Tframlen;
			if ( !Rxbuflen && (Tframlen>=32) && (Tframlen<=1024))
				Rxbuflen = Tframlen;
			vfile("Rxbuflen=%ld", (long)Rxbuflen);

			/* If using a pipe for testing set lower buf len */
			fstat(STDIN_FILENO, &f);
			if ((f.st_mode & S_IFMT) != S_IFCHR) {
				Rxbuflen = 1024;
			}


			/*
			 * If input is not a regular file, force ACK's to
			 *  prevent running beyond the buffer limits
			 */
			if ( !Command) {
				fstat(fileno(in), &f);
				if ((f.st_mode & S_IFMT) != S_IFREG) {
					Canseek = -1;
#ifdef TXBSIZE
					Txwindow = TXBSIZE - 1024;
					Txwspac = TXBSIZE/4;
#else
					return ERROR;
#endif
				}
			}

			/* Set initial subpacket length */
			if (blklen < 1024) {	/* Command line override? */
				if (Effbaud > 300)
					blklen = 256;
				if (Effbaud > 1200)
					blklen = 512;
				if (Effbaud > 2400)
					blklen = 1024;
			}
			if (Rxbuflen && blklen>Rxbuflen)
				blklen = Rxbuflen;
			if (blkopt && blklen > blkopt)
				blklen = blkopt;
			vfile("Rxbuflen=%ld blklen=%ld", (long)Rxbuflen, (long)blklen);
			vfile("Txwindow = %lu Txwspac = %ld", (long)Txwindow,(long) Txwspac);


			if (Lztrans == ZTRLE && (Rxflags & CANRLE))
				Txfcs32 = 2;
			else
				Lztrans = 0;

			return (sendzsinit());
		case ZCAN:
		case TIMEOUT:
			return ERROR;
		case ZRQINIT:
			if (Rxhdr[ZF0] == ZCOMMAND)
				continue;
		default:
			zshhdr(4, ZNAK, Txhdr);
			continue;
		}
	}
	return ERROR;
}

/* Send send-init information */
int sendzsinit(void)
{
	register c;

	if (Myattn[0] == '\0' && (!Zctlesc || (Rxflags & TESCCTL)))
		return OK;
	errors = 0;
	for (;;) {
		stohdr(0L);
#ifdef ALTCANOFF
		Txhdr[ALTCOFF] = ALTCANOFF;
#endif
		if (Zctlesc) {
			Txhdr[ZF0] |= TESCCTL; zshhdr(4, ZSINIT, Txhdr);
		}
		else
			zsbhdr(4, ZSINIT, Txhdr);
		zsdata(Myattn, ZATTNLEN, ZCRCW);
		c = zgethdr(Rxhdr, 1);
                switch (c) {
		case ZCAN:
			return ERROR;
		case ZACK:
			return OK;
		default:
			if (++errors > 19)
				return ERROR;
			continue;
		}
	}
}

/* Send file name and related info */
int zsendfile(char *buf, int blen)
{
	int c;
        int foo;
        word stack,stack2;
	unsigned long crc;
	long lastcrcrq = -1;
	char *p;

        asm {
	        tsc
                sta stack
        }
	for (errors=0; ++errors<11;) {
		Txhdr[ZF0] = Lzconv;	/* file conversion request */
		Txhdr[ZF1] = Lzmanag;	/* file management request */
		if (Lskipnocor)
			Txhdr[ZF1] |= ZMSKNOLOC;
		Txhdr[ZF2] = Lztrans;	/* file transport request */
		Txhdr[ZF3] = 0;
		zsbhdr(4, ZFILE, Txhdr);
		zsdata(buf, blen, ZCRCW);
again:
		c = zgethdr(Rxhdr, 1);
		switch (c) {
		case ZRINIT:
			while ((c = readline(50)) > 0)
				if (c == ZPAD) {
					goto again;
				}
			continue;
		case ZCAN:
		case TIMEOUT:
		case ZABORT:
		case ZFIN:
			sprintf(endmsg, "Got %s on pathname", frametypes[c+FTOFFSET]);
			return ERROR;
		case ERROR:
		case ZNAK:
			continue;
		case ZCRC:
			if (Rxpos != lastcrcrq) {
				lastcrcrq = Rxpos;
				crc = 0xFFFFFFFFL;
				if (Canseek >= 0) {
					fseek(in, 0L, 0);
					while (((c = getc(in)) != EOF) && --lastcrcrq)
						crc = UPDC32(c, crc);
					crc = ~crc;
					clearerr(in);	/* Clear possible EOF */
					lastcrcrq = Rxpos;
				}
			}
			stohdr(crc);
			zsbhdr(4, ZCRC, Txhdr);
			goto again;
		case ZFERR:
		case ZSKIP:
			sprintf(endmsg, "File skipped by receiver request");
			fclose(in); return c;
		case ZRPOS:
			/*
			 * Suppress zcrcw request otherwise triggered by
			 * lastsync==bytcnt
			 */
                        if (fseek(in, Rxpos, 0))
			       return ERROR;
                        Lrxpos = Rxpos;
                        Txpos = Rxpos;
                        bytcnt = Rxpos;
                        Lastsync = Rxpos - 1l;
      /* $$$ ORCA/C BUG! Lastsync = (bytcnt = Txpos = Lrxpos = Rxpos) -1; */
                        return zsendfdata();
		default:
			sprintf(endmsg, "Got %d frame type on pathname", c);
			continue;
		}
	}
	fclose(in); return ERROR;
}

/* Send the data in the file */
int zsendfdata(void)
{
	register c, e, n;
	register newcnt;
	register long tcount = 0;
	int junkcount;		/* Counts garbage chars received by TX */
	static int tleft = 6;	/* Counter for test mode */

        zperr("Sending file data!!\n");

	junkcount = 0;
	Beenhereb4 = FALSE;
somemore:
	if (setjmp(intrjmp)) {
waitack:
		junkcount = 0;
		c = getinsync(0);
gotack:
		switch (c) {
		default:
		case ZCAN:
			fclose(in);
			return ERROR;
		case ZSKIP:
			fclose(in);
			return c;
		case ZACK:
		case ZRPOS:
			break;
		case ZRINIT:
			fclose(in);
			return OK;
		}
#ifdef READCHECK
		/*
		 * If the reverse channel can be tested for data,
		 *  this logic may be used to detect error packets
		 *  sent by the receiver, in place of setjmp/longjmp
		 *  rdchk(Tty) returns non 0 if a character is available
		 */
		while (rdchk(Tty)) {
#ifdef EATSIT
			switch (checked)
#else
			switch (readline(1))
#endif
			{
			case CAN:
			case ZPAD:
                                c = getinsync(1);
				goto gotack;
			case XOFF:		/* Wait a while for an XON */
			case XOFF|0200:
				readline(100);
			}
		}
#endif
	}

	signal(SIGINT, onintr);
	newcnt = Rxbuflen;
	Txwcnt = 0;
	stohdr(Txpos);
	zsbhdr(4, ZDATA, Txhdr);

	/*
	 * Special testing mode.  This should force receiver to Attn,ZRPOS
	 *  many times.  Each time the signal should be caught, causing the
	 *  file to be started over from the beginning.
	 */
	if (Test) {
		if ( --tleft)
			while (tcount < 20000) {
				printf(qbf); fflush(stdout);
				tcount += strlen(qbf);
#ifdef READCHECK
				while (rdchk(Tty)) {
#ifdef EATSIT
					switch (checked)
#else
					switch (readline(1))
#endif
					{
					case CAN:
					case ZPAD:
#ifdef TCFLSH
						ioctl(0, TCFLSH, 1);
#endif
						goto waitack;
					case XOFF:	/* Wait for XON */
					case XOFF|0200:
						readline(100);
					}
				}
#endif
			}
		signal(SIGINT, SIG_IGN); canit();
		sleep(3); purgeline(); mode(0);
		printf("\nsz: Tcount = %ld\n", tcount);
		if (tleft) {
			printf("ERROR: Interrupts Not Caught\n");
			exit(1);
		}
		exit(0);
	}

	do {
		n = zfilbuf();
                if (Eofseen)
			e = ZCRCE;
		else if (junkcount > 3)
			e = ZCRCW;
		else if (bytcnt == Lastsync)
			e = ZCRCW;
                /* $$$ This statement is causing us to freak out */
                else if (Rxbuflen && (newcnt -= n) <= 0)
			e = ZCRCW;
		else if (Txwindow && (Txwcnt += n) >= Txwspac) {
			Txwcnt = 0;  e = ZCRCQ;
		} else
			e = ZCRCG;
		if (Verbose>1)
			fprintf(stderr, "\r%7ld ZMODEM%s    ",
			  Txpos, Crc32t ? " CRC-32" : "");
		zsdata(txbuf, n, e);
		bytcnt = Txpos += n;
		if (e == ZCRCW) {
		        zperr("We're going to waitack, for some reason\n");
			goto waitack;
		}
#ifdef READCHECK
		/*
		 * If the reverse channel can be tested for data,
		 *  this logic may be used to detect error packets
		 *  sent by the receiver, in place of setjmp/longjmp
		 *  rdchk(Tty) returns non 0 if a character is available
		 */
		fflush(stdout);
		while (rdchk(Tty)) {
#ifdef EATSIT
			switch (checked)
#else
			switch (readline(1))
#endif
			{
			case CAN:
			case ZPAD:
                                c = getinsync(1);
				if (c == ZACK)
					break;
#ifdef TCFLSH
				ioctl(0, TCFLSH, 1);
#endif
				/* zcrce - dinna wanna starta ping-pong game */
				zsdata(txbuf, 0, ZCRCE);
				goto gotack;
			case XOFF:		/* Wait a while for an XON */
			case XOFF|0200:
				readline(100);
			default:
				++junkcount;
			}
		}
#endif	/* READCHECK */
		if (Txwindow) {
			while ((tcount = (Txpos - Lrxpos)) >= Txwindow) {
				vfile("%ld window >= %lu", (long)tcount, (long)Txwindow);
				if (e != ZCRCQ)
					zsdata(txbuf, 0, e = ZCRCQ);
				c = getinsync(1);
				if (c != ZACK) {
#ifdef TCFLSH
					ioctl(STDIN_FILENO, TCFLSH, 1);
#endif
					zsdata(txbuf, 0, ZCRCE);
					goto gotack;
				}
			}
			vfile("window = %ld", tcount);
		}
	} while (!Eofseen); /* Keep going until we get to the end of the file */
	signal(SIGINT, SIG_IGN);

        /* The file has been transmitted, so wait for the other end to
           recognize that fact */
        for (;;) {
		stohdr(Txpos);
		zsbhdr(4, ZEOF, Txhdr);
		switch (getinsync(0)) {
		case ZACK:
			continue;
		case ZRPOS:
			goto somemore;
		case ZRINIT:
                        /* That's all, folks! */
                        fclose(in);
			return OK;                 
		case ZSKIP:
			fclose(in);
			sprintf(endmsg, "File skipped by receiver request");
			return c;
		default:
			sprintf(endmsg, "Got %d trying to send end of file", c);
			fclose(in);
			return ERROR;
		}
	}
}

/*
 * Respond to receiver's complaint, get back in sync with receiver
 */
int getinsync(int flag)
{
	register c;

	for (;;) {
		if (Test) {
			printf("\r\n\n\n***** Signal Caught *****\r\n");
			Rxpos = 0; c = ZRPOS;
		} else
		      c = zgethdr(Rxhdr, 0);

		switch (c) {
		case ZCAN:
		case ZABORT:
		case ZFIN:
		case TIMEOUT:
			sprintf(endmsg, "Got %s sending data", frametypes[c+FTOFFSET]);
			return ERROR;
		case ZRPOS:
			/* ************************************* */
			/*  If sending to a buffered modem, you  */
			/*   might send a break at this point to */
			/*   dump the modem's buffer.		 */
			clearerr(in);	/* In case file EOF seen */
			if (fseek(in, Rxpos, 0))
				return ERROR;
			Eofseen = 0;
			bytcnt = Lrxpos = Txpos = Rxpos;
			if (Lastsync == Rxpos) {
				if (++Beenhereb4 > 12) {
					sprintf(endmsg, "Can't send block");
					return ERROR;
				}
				if (Beenhereb4 > 4)
					if (blklen > 32)
						blklen /= 2;
			}
			Lastsync = Rxpos;
			return c;
		case ZACK:
			Lrxpos = Rxpos;
			if (flag || Txpos == Rxpos)
				return ZACK;
			continue;
		case ZRINIT:
                        return c;
		case ZSKIP:
			sprintf(endmsg, "File skipped by receiver request");
			return c;
		case ERROR:
		default:
			zsbhdr(4, ZNAK, Txhdr);
			continue;
		}
	}
}


/* Say "bibi" to the receiver, try to do it cleanly */
void saybibi(void)
{
int foo;

	for (;;) {
		stohdr(0L);		/* CAF Was zsbhdr - minor change */
		zshhdr(4, ZFIN, Txhdr);	/*  to make debugging easier */
                switch (zgethdr(Rxhdr, 0)) {
		case ZFIN:
			sendline('O'); sendline('O'); flushmo();
		case ZCAN:
		case TIMEOUT:
			return;
		}
	}
}

/* Send command and related info */
int zsendcmd(char *buf, int blen)
{
	register int c;
	long cmdnum;

	cmdnum = getpid();
	errors = 0;
	for (;;) {
		stohdr(cmdnum);
		Txhdr[ZF0] = Cmdack1;
		zsbhdr(4, ZCOMMAND, Txhdr);
		zsdata(buf, blen, ZCRCW);
listen:
		Rxtimeout = 100;		/* Ten second wait for resp. */
		Usevhdrs = 0;		/* Allow rx to send fixed len headers */
		c = zgethdr(Rxhdr, 1);

		switch (c) {
		case ZRINIT:
			fprintf(stderr,"got a ZRINIT\n");
                        goto listen;	/* CAF 8-21-87 */
		case ERROR:
		case GCOUNT:
		case TIMEOUT:
			if (++errors > Cmdtries)
                                return ERROR;
			continue;
		case ZCAN:
		case ZABORT:
		case ZFIN:
		case ZSKIP:
		case ZRPOS:
                        return ERROR;
		case ZCOMPL:
			Exitcode = Rxpos;
			saybibi();
                        return OK;
		case ZRQINIT:
			vfile("******** RZ *******");
			system("rz");
			vfile("******** SZ *******");
			goto listen;
		default:
			if (++errors > 20)
				return ERROR;
			continue;
		}
	}
}

/*
 * If called as sb use YMODEM protocol
 */
void chkinvok(char *s)
{
	register char *p;

	p = s;
	while (*p == '-')
		s = ++p;
	while (*p)
		if (*p++ == '/')
			s = p;
	if (*s == 'v') {
		Verbose=1; ++s;
	}
	Progname = s;
	if (s[0]=='z' && s[1] == 'c') {
		Command = TRUE;
		if (s[8] == 'i')
			Cmdack1 = ZCACK1;
	}
	if (s[0]=='s' && s[1]=='b') {
		IsYMODEM = TRUE; blklen=1024;
	}
	if (s[0]=='s' && s[1]=='x') {
		IsXMODEM = TRUE;
	}
}

void countem(int argc, char **argv)
{
	register c;
	struct stat f;

	for (Totalleft = 0, Filesleft = 0; --argc >=0; ++argv) {
		f.st_size = -1;
		if (Verbose>2) {
			fprintf(stderr, "\nCountem: %03d %s ", argc, *argv);
			fflush(stderr);
		}
		if (/*access(*argv, 04) >= 0 && */stat(*argv, &f) >= 0) {
			c = f.st_mode & S_IFMT;
			if (c != S_IFDIR && c != S_IFBLK) {
				++Filesleft;  Totalleft += f.st_size;
			}
		}
		if (Verbose>2)
			fprintf(stderr, " %ld", f.st_size);
	}
	if (Verbose>2)
		fprintf(stderr, "\ncountem: Total %d %ld\n",
		  Filesleft, Totalleft);
}

void chartest(int m)
{
	register n;

	mode(m);
	printf("\r\n\nCharacter Transparency Test Mode %d\r\n", m);
	printf("If Pro-YAM/ZCOMM is not displaying ^M hit ALT-V NOW.\r\n");
	printf("Hit Enter.\021");  fflush(stdout);
	readline(500);

	for (n = 0; n < 256; ++n) {
		if (!(n%8))
			printf("\r\n");
		printf("%02x ", n);  fflush(stdout);
		sendline(n);	flushmo();
		printf("  ");  fflush(stdout);
		if (n == 127) {
			printf("Hit Enter.\021");  fflush(stdout);
			readline(500);
			printf("\r\n");  fflush(stdout);
		}
	}
	printf("\021\r\nEnter Characters, echo is in hex.\r\n");
	printf("Hit SPACE or pause 40 seconds for exit.\r\n");

	while (n != TIMEOUT && n != ' ') {
		n = readline(400);
		printf("%02x\r\n", n);
		fflush(stdout);
	}
	printf("\r\nMode %d character transparency test ends.\r\n", m);
	fflush(stdout);
}

/* End of sz.c */
