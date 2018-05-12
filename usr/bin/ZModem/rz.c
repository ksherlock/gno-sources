#pragma stacksize 2048
#pragma optimize 9

/*
 *
 * rz.c By Chuck Forsberg
 *    Copyright 1991 Omen Technology Inc All Rights Reserved
 *
 * A program for Unix to receive files and commands from computers running
 *  Professional-YAM, PowerCom, YAM, IMP, or programs supporting XMODEM.
 *  rz uses Unix buffered input to reduce wasted CPU time.
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
 *
 *
 *
 * Iff the program is invoked by rzCOMMAND, output is piped to 
 * "COMMAND filename"  (Unix only)
 *
 *  Alarm signal handling changed to work with 4.2 BSD 7-15-84 CAF 
 *
 *  SEGMENTS=n added 2-21-88 as a model for CP/M programs
 *    for CP/M-80 systems that cannot overlap modem and disk I/O.
 *
 *  -DMD may be added to compiler command line to compile in
 *    Directory-creating routines from Public Domain TAR by John Gilmore
 *
 *  HOWMANY may be tuned for best performance
 *
 *  USG UNIX (3.0) ioctl conventions courtesy  Jeff Martin
 */

#define LOGFILE ".ttyco"
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <gno/gno.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#define __RZ__
#include "zmodem.h"

#define SEGMENTS 16

FILE *popen();

#define OK 0
#define FALSE 0
#define TRUE 1
#define ERROR (-1)

/* Ward Christensen / CP/M parameters - Don't change these! */
#define ENQ 005
#define CAN ('X'&037)
#define XOFF ('s'&037)
#define XON ('q'&037)
#define SOH 1
#define STX 2
#define EOT 4
#define ACK 6
#define NAK 025
#define CPMEOF 032
#define WANTCRC 0103	/* send C not NAK to get crc not checksum */
#define TIMEOUT (-2)
#define RCDO (-3)
#define GCOUNT (-4)
#define ERRORMAX 5
/*#define RETRYMAX 5*/
#define WCEOT (-10)
#define PATHLEN 257	/* ready for 4.2 bsd ? */
#define UNIXFILE 0xF000	/* The S_IFMT file mask bit for stat */

int Zmodem=0;		/* ZMODEM protocol requested */
unsigned Baudrate = 4800;
unsigned Effbaud = 4800;

char endmsg[90] = {0};	/* Possible message to display on exit */

FILE *fout;

/*
 * Routine to calculate the free bytes on the current file system
 *  ~0 means many free bytes (unknown)
 */
long getfree(void)
{
	return(~0L);	/* many free bytes ... */
}

int Lastrx;
long rxbytes;
int Crcflg;
int Firstsec;
int Eofseen;		/* indicates cpm eof (^Z) has been received */
int errors;
int Restricted=0;	/* restricted; no /.. or ../ in filenames */

#define DEFBYTL 2000000000L	/* default rx file size */
long Bytesleft;		/* number of bytes of incoming file left */
long Modtime;		/* Unix style mod time for incoming file */
int Filemode;		/* Unix style mode for incoming file */
long Totalleft;
long Filesleft;
char Pathname[PATHLEN];
/*char *Progname;		/* the name by which we were called */

int Batch=0;
int Topipe=0;
int Thisbinary;		/* current file is to be received in bin mode */
int Rxbinary=FALSE;	/* receive all files in bin mode */
int Rxascii=FALSE;	/* receive files in ascii (translate) mode */
int Blklen;		/* record length of received packets */

#ifdef SEGMENTS
int chinseg = 0;	/* Number of characters received in this data seg */
char secbuf[1+(SEGMENTS+1)*1024];
#else
char secbuf[1025];
#endif

time_t timep[2];
char Lzmanag;		/* Local file management request */
char Lzconv;		/* Local ZMODEM file conversion request */
char zconv;		/* ZMODEM file conversion request */
char zmanag;		/* ZMODEM file management request */
char ztrans;		/* ZMODEM file transport request */
/*int Zctlesc;		/* Encode control characters */
/*int Zrwindow = 1400;	/* RX window size (controls garbage count) */

int tryzhdrtype=ZRINIT;	/* Header type to send corresponding to Last rx close */

/* called by signal interrupt or terminate to clean things up */
void bibi(int n, int code)
{
	if (Zmodem)
		zmputs(Attn);
	canit(); mode(0);
	fprintf(stderr, "rz: caught signal %d; exiting", n);
	exit(3);
}

segment "main      ";
int main(int argc, char **argv)
{
	register char *cp;
	int npats;
	char *virgin, **patts;
	int exitcode = 0;
        extern int howmany;

        Readnum = howmany = 255;

        Rxtimeout = 100;
	setbuf(stderr, NULL);
	if ((cp=getenv("SHELL")) && (substr(cp, "rsh") || substr(cp, "rksh")))
		Restricted=TRUE;

	chkinvok(virgin=argv[0]);	/* if called as [-]rzCOMMAND set flag */
	inittty();
	npats = 0;
	while (--argc) {
		cp = *++argv;
		if (*cp == '-') {
			while( *++cp) {
				switch(*cp) {
				case '\\':
					 cp[1] = toupper(cp[1]);  continue;
				case 'a':
					if (!Batch || IsYMODEM)
						Rxascii=TRUE;
					else
						usage();
					break;
				case 't':
					if (--argc < 1) {
						usage();
					}
					Rxtimeout = atoi(*++argv);
					if (Rxtimeout<10 || Rxtimeout>1000)
						usage();
					break;
				case 'w':
					if (--argc < 1) {
						usage();
					}
					Zrwindow = atoi(*++argv);
					break;
				case 'v':
					++Verbose; break;
				default:
					usage();
				}
			}
		}
		else if ( !npats && argc>0) {
			if (argv[0][0]) {
				npats=argc;
				patts=argv;
			}
		}
	}
	if (npats > 1)
		usage();
	if (Batch && npats)
		usage();
	if (Verbose) {
        /*FIXME if needed */
	/* if (freopen(LOGFILE, "a", stderr)==NULL) {
			fprintf(stderr, "Can't open log file %s\n",LOGFILE);
			exit(2);
		}  */
		setbuf(stderr, NULL);
		fprintf(stderr, "argv[0]=%s Progname=%s\n", virgin, Progname);
	}
	vfile("%s %s for %s\n", Progname, VERSION, OS);
	mode(1);
	if (signal(SIGINT, bibi) == (void *)SIG_IGN) {
		signal(SIGINT, SIG_IGN); signal(SIGKILL, SIG_IGN);
	}
	else {
		signal(SIGINT, bibi); signal(SIGKILL, bibi);
	}
	signal(SIGTERM, bibi);
	if (wcreceive(npats, patts)==ERROR) {
		exitcode=1;
		canit();
	}
	mode(0);
	if (exitcode && !Zmodem)	/* bellow again with all thy might. */
		canit();
	if (endmsg[0])
		printf( "  %s: %s\r\n", Progname, endmsg);
	printf( "%s %s finished.\r\n", Progname, VERSION);
	fflush(stdout);
	exit(exitcode != 0);
}


void usage(void)
{
	fprintf(stderr,
	"Receive Files and Commands with ZMODEM/YMODEM/XMODEM Protocol\n\n");
	fprintf(stderr,"%s %s for %s by Chuck Forsberg, Omen Technology INC\n",
	  Progname, VERSION, OS);
	fprintf(stderr, "\t\t\042The High Reliability Software\042\n\n");
	fprintf(stderr,"Usage:	rz [-v]		(ZMODEM)\n");
	fprintf(stderr,"or	rb [-av]	(YMODEM)\n");
	fprintf(stderr,"or	rc [-av] file	(XMODEM-CRC)\n");
	fprintf(stderr,"or	rx [-av] file	(XMODEM)\n\n");
	fprintf(stderr,
	"\nSee rz.doc for option descriptions and licensing information.\n\n");
	fprintf(stderr,
"Supports incoming ZMODEM binary (-b), ASCII CR/LF>NL (-a), newer(-n),\n\
	newer+longer(-N), protect (-p), Crash Recovery (-r),\n\
clobber (-y), match+clobber (-Y), compression (-Z), and append (-+).\n\n");
	fprintf(stderr,"\tCopyright 1991 Omen Technology INC All Rights Reserved\n");
	exit(2);
}

/*
 * Let's receive something already.
 */

char *rbmsg = "%s ready. Type \"%s file ...\" to your modem program\n\r";

int wcreceive(int argc, char **argp)
{
	int c;

	if (Batch || argc==0) {
		Crcflg=1;
		fprintf(stderr, rbmsg, Progname, IsYMODEM?"sb":"sz");
		if (c=tryz()) {
			if (c == ZCOMPL)
				return OK;
			if (c == ERROR)
				goto fubar;
			c = rzfiles();
			if (c)
				goto fubar;
		} else {
			for (;;) {
				if (wcrxpn(secbuf)== ERROR)
					goto fubar;
				if (secbuf[0]==0)
					return OK;
				if (procheader(secbuf) == ERROR)
					goto fubar;
				if (wcrx()==ERROR)
					goto fubar;
			}
		}
	} else {
		Bytesleft = DEFBYTL; Filemode = 0; Modtime = 0L;

		/*procheader("");*/ strcpy(Pathname, *argp); checkpath(Pathname);
		fprintf(stderr, "\nrz: ready to receive %s\r\n", Pathname);
		if ((fout=fopen(Pathname, "wb")) == NULL)
			return ERROR;
		if (wcrx()==ERROR)
			goto fubar;
		setvbuf(fout,NULL,_IOFBF,0x7000l);
	}
	return OK;
fubar:
	canit();
	if (Topipe && fout) {
		pclose(fout);  return ERROR;
	}
	Modtime = 1;
	if (fout)
		fclose(fout);
	if (Restricted) {
		unlink(Pathname);
		fprintf(stderr, "\r\nrz: %s removed.\r\n", Pathname);
	}
	return ERROR;
}


/*
 * Fetch a pathname from the other end as a C ctyle ASCIZ string.
 * Length is indeterminate as long as less than Blklen
 * A null string represents no more files (YMODEM)
 */
int wcrxpn(char *rpn)
{
	int c;

	purgeline();

et_tu:
	Firstsec=TRUE;  Eofseen=FALSE;
	sendline(Crcflg?WANTCRC:NAK);
	Lleft=0;	/* Do read next time ... */
	while ((c = wcgetsec(rpn, 100)) != 0) {
		if (c == WCEOT) {
			zperr("Pathname fetch returned %d", c);
			sendline(ACK);
			Lleft=0;	/* Do read next time ... */
			readline(1);
			goto et_tu;
		}
		return ERROR;
	}
	sendline(ACK);
	return OK;
}

/*
 * Adapted from CMODEM13.C, written by
 * Jack M. Wierda and Roderick W. Hart
 */

int wcrx(void)
{
	register int sectnum, sectcurr;
	register char sendchar;
	register char *p;
	int cblklen;			/* bytes to dump this block */

	Firstsec=TRUE;sectnum=0; Eofseen=FALSE;
	sendchar=Crcflg?WANTCRC:NAK;

	for (;;) {
		sendline(sendchar);	/* send it now, we're ready! */
		Lleft=0;	/* Do read next time ... */
		sectcurr=wcgetsec(secbuf, (sectnum&0177)?50:130);
		report(sectcurr);
		if (sectcurr==(sectnum+1 &0377)) {
			sectnum++;
			cblklen = Bytesleft>Blklen ? Blklen:Bytesleft;
			if (putsec(secbuf, cblklen)==ERROR)
				return ERROR;
			if ((Bytesleft-=cblklen) < 0)
				Bytesleft = 0;
			sendchar=ACK;
		}
		else if (sectcurr==(sectnum&0377)) {
			zperr("Received dup Sector");
			sendchar=ACK;
		}
		else if (sectcurr==WCEOT) {
			if (closeit())
				return ERROR;
			sendline(ACK);
			Lleft=0;	/* Do read next time ... */
			return OK;
		}
		else if (sectcurr==ERROR)
			return ERROR;
		else {
			zperr("Sync Error");
			return ERROR;
		}
	}
}

/*
 * Wcgetsec fetches a Ward Christensen type sector.
 * Returns sector number encountered or ERROR if valid sector not received,
 * or CAN CAN received
 * or WCEOT if eot sector
 * time is timeout for first char, set to 4 seconds thereafter
 ***************** NO ACK IS SENT IF SECTOR IS RECEIVED OK **************
 *    (Caller must do that when he is good and ready to get next sector)
 */

int wcgetsec(char *rxbuf, int maxtime)
{
	register int checksum, wcj, firstch;
	register unsigned short oldcrc;
	register char *p;
	int sectcurr;

	for (Lastrx=errors=0; errors<RETRYMAX; errors++) {

		if ((firstch=readline(maxtime))==STX) {
			Blklen=1024; goto get2;
		}
		if (firstch==SOH) {
			Blklen=128;
get2:
			sectcurr=readline(1);
			if ((sectcurr+(oldcrc=readline(1)))==0377) {
				oldcrc=checksum=0;
				for (p=rxbuf,wcj=Blklen; --wcj>=0; ) {
					if ((firstch=readline(1)) < 0)
						goto bilge;
					oldcrc=updcrc(firstch, oldcrc);
					checksum += (*p++ = firstch);
				}
				if ((firstch=readline(1)) < 0)
					goto bilge;
				if (Crcflg) {
					oldcrc=updcrc(firstch, oldcrc);
					if ((firstch=readline(1)) < 0)
						goto bilge;
					oldcrc=updcrc(firstch, oldcrc);
					if (oldcrc & 0xFFFF)
						zperr("CRC");
					else {
						Firstsec=FALSE;
						return sectcurr;
					}
				}
				else if (((checksum-firstch)&0377)==0) {
					Firstsec=FALSE;
					return sectcurr;
				}
				else
					zperr("Checksum");
			}
			else
				zperr("Sector number garbled");
		}
		/* make sure eot really is eot and not just mixmash */
		else if (firstch==EOT && Lleft==0)
			return WCEOT;
		else if (firstch==CAN) {
			if (Lastrx==CAN) {
				zperr("Sender CANcelled");
				return ERROR;
			} else {
				Lastrx=CAN;
				continue;
			}
		}
		else if (firstch==TIMEOUT) {
			if (Firstsec)
				goto humbug;
bilge:
			zperr("TIMEOUT");
		}
		else
			zperr("Got 0%o sector header", firstch);

humbug:
		Lastrx=0;
		while(readline(1)!=TIMEOUT)
			;
		if (Firstsec) {
			sendline(Crcflg?WANTCRC:NAK);
			Lleft=0;	/* Do read next time ... */
		} else {
			maxtime=40; sendline(NAK);
			Lleft=0;	/* Do read next time ... */
		}
	}
	/* try to stop the bubble machine. */
	canit();
	return ERROR;
}


/*
 * Process incoming file information header
 */
int procheader(char *name)
{
	register char *openmode, *p, **pp;
	static int dummy;
	struct stat f;

        fprintf(stderr,"entering procheader\n");
	/* set default parameters and overrides */
	openmode = "wb";
	Thisbinary = (!Rxascii) || Rxbinary;
	if (zconv == ZCBIN && Lzconv != ZCRESUM)
		Lzconv = zconv;			/* Remote Binary override */
	if (Lzconv)
		zconv = Lzconv;
	if (Lzmanag)
		zmanag = Lzmanag;

	/*
	 *  Process ZMODEM remote file management requests
	 */
	if (!Rxbinary && zconv == ZCNL)	/* Remote ASCII override */
		Thisbinary = 0;
	if (zconv == ZCBIN)	/* Remote Binary override */
		Thisbinary = TRUE;
	else if (zmanag == ZMAPND)
		openmode = "ab";

	Bytesleft = DEFBYTL; Filemode = 0; Modtime = 0L;

	p = name + 1 + strlen(name);
	if (*p) {	/* file coming from Unix or DOS system */
		sscanf(p, "%ld%lo%o%lo%d%ld%d%d",
		  &Bytesleft, &Modtime, &Filemode,
		  &dummy, &Filesleft, &Totalleft, &dummy, &dummy);
		if (Filemode & UNIXFILE)
			++Thisbinary;
		if (Verbose) {
			fprintf(stderr,  "Incoming: %s %ld %lo %o\n",
			  name, Bytesleft, Modtime, Filemode);
			fprintf(stderr,  "YMODEM header: %s\n", p);
		}
	}


	else {		/* File coming from CP/M system */
		for (p=name; *p; ++p)		/* change / to _ */
			if ( *p == '/')
				*p = '_';

		if ( *--p == '.')		/* zap trailing period */
			*p = 0;
	}

	strcpy(Pathname, name);
	fprintf(stderr,"did strcpy\n");
        checkpath(name);
        fprintf(stderr,"did checkpath()\n");
	if (*name && stat(name, &f)!= -1) {
		zmanag &= ZMMASK;
		vfile("Current %s is %ld %lo", name, f.st_size, f.st_mtime);
		if (Thisbinary && zconv==ZCRESUM) {
			rxbytes = f.st_size & ~511;
			if (Bytesleft < rxbytes) {
				rxbytes = 0;  goto doopen;
			} else
				openit(name, "r+");
			if ( !fout)
				return ZFERR;
			if (fseek(fout, rxbytes, 0)) {
				closeit();
				return ZFERR;
			}
			vfile("Crash recovery at %ld", rxbytes);
			return 0;
		}
		else if ((zmanag==ZMNEW) ||
		  ((zmanag==ZMNEWL) && Bytesleft <= f.st_size) ) {
			if ((f.st_mtime+1) >= Modtime)
				goto skipfile;
			goto doopen;
		}
		switch (zmanag & ZMMASK) {
		case ZMCLOB:
		case ZMAPND:
			goto doopen;
		default:
			goto skipfile;
		}
	} else if (zmanag & ZMSKNOLOC) {
skipfile:
		vfile("Skipping %s", name);
		return ZSKIP;
	}
doopen:
	openit(name, openmode);
#ifdef MD
	if ( !fout)
		if (make_dirs(name))
			openit(name, openmode);
#endif
	if ( !fout)
		return ZFERR;
	return 0;
}

void openit(char *name, char *openmode)
{
	fout = fopen(name, openmode);
}

#ifdef MD
/*
 *  Directory-creating routines from Public Domain TAR by John Gilmore
 */

/*
 * After a file/link/symlink/dir creation has failed, see if
 * it's because some required directory was not present, and if
 * so, create all required dirs.
 */
int make_dirs(char *pathname)
{
	register char *p;		/* Points into path */
	int madeone = 0;		/* Did we do anything yet? */
	int save_errno = errno;		/* Remember caller's errno */

	if (errno != ENOENT)
		return 0;		/* Not our problem */

	for (p = strchr(pathname, '/'); p != NULL; p = strchr(p+1, '/')) {
		/* Avoid mkdir of empty string, if leading or double '/' */
		if (p == pathname || p[-1] == '/')
			continue;
		/* Avoid mkdir where last part of path is '.' */
		if (p[-1] == '.' && (p == pathname+1 || p[-2] == '/'))
			continue;
		*p = 0;				/* Truncate the path there */
		if ( !mkdir(pathname, 0777)) {	/* Try to create it as a dir */
			vfile("Made directory %s\n", pathname);
			madeone++;		/* Remember if we made one */
			*p = '/';
			continue;
		}
		*p = '/';
		if (errno == EEXIST)		/* Directory already exists */
			continue;
		/*
		 * Some other error in the mkdir.  We return to the caller.
		 */
		break;
	}
	errno = save_errno;		/* Restore caller's errno */
	return madeone;			/* Tell them to retry if we made one */
}

#if (MD != 2)
#define	TERM_SIGNAL(status)	((status) & 0x7F)
#define TERM_COREDUMP(status)	(((status) & 0x80) != 0)
#define TERM_VALUE(status)	((status) >> 8)
/*
 * Make a directory.  Compatible with the mkdir() system call on 4.2BSD.
 */
int mkdir(char *dpath, int dmode)
{
	int cpid, status;
	struct stat statbuf;

	if (stat(dpath,&statbuf) == 0) {
		errno = EEXIST;		/* Stat worked, so it already exists */
		return -1;
	}

	/* If stat fails for a reason other than non-existence, return error */
	if (errno != ENOENT) return -1; 

	switch (cpid = fork()) {

	case -1:			/* Error in fork() */
		return(-1);		/* Errno is set already */

	case 0:				/* Child process */
		/*
		 * Cheap hack to set mode of new directory.  Since this
		 * child process is going away anyway, we zap its umask.
		 * FIXME, this won't suffice to set SUID, SGID, etc. on this
		 * directory.  Does anybody care?
		 */
		status = umask(0);	/* Get current umask */
		status = umask(status | (0777 & ~dmode)); /* Set for mkdir */
		execl("/bin/mkdir", "mkdir", dpath, (char *)0);
		_exit(2);		/* Can't exec /bin/mkdir */
	
	default:			/* Parent process */
		while (cpid != wait(&status)) ;	/* Wait for kid to finish */
	}

	if (TERM_SIGNAL(status) != 0 || TERM_VALUE(status) != 0) {
		errno = EIO;		/* We don't know why, but */
		return -1;		/* /bin/mkdir failed */
	}

	return 0;
}
#endif /* MD != 2 */
#endif /* MD */

/*
 * Putsec writes the n characters of buf to receive file fout.
 *  If not in binary mode, carriage returns, and all characters
 *  starting with CPMEOF are discarded.
 */
int putsec(char *buf, int n)
{
	register char *p;

	if (n == 0)
		return OK;
	if (Thisbinary) {
		/* $$$ fwrite would be cool here */
                for (p=buf; --n>=0; )
			putc( *p++, fout);
	}
	else {
		if (Eofseen)
			return OK;
		/* $$$ We can use fwrite() here instead of this crap */
                for (p=buf; --n>=0; ++p ) {
			if ( *p == '\r')
				continue;
			if (*p == CPMEOF) {
				Eofseen=TRUE; return OK;
			}
			putc(*p ,fout);
		}
	}
	return OK;
}

/*
 * substr(string, token) searches for token in string s
 * returns pointer to token within string if found, NULL otherwise
 */
char *substr(char *s, char *t)
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

/*
 * Log an error
 */
/*VARARGS1*/


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

void report(int sct)
{
	if (Verbose>1)
		fprintf(stderr,"%03d%c",sct,sct%10? ' ' : '\r');
}

/*
 * If called as [-][dir/../]vrzCOMMAND set Verbose to 1
 * If called as [-][dir/../]rzCOMMAND set the pipe flag
 * If called as rb use YMODEM protocol
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
	if (s[0]=='r' && s[1]=='z')
		Batch = TRUE;
	if (s[0]=='r' && s[1]=='c')
		Crcflg = TRUE;
	if (s[0]=='r' && s[1]=='b')
		Batch = IsYMODEM = TRUE;
	if (s[2] && s[0]=='r' && s[1]=='b')
		Topipe = 1;
	if (s[2] && s[0]=='r' && s[1]=='z')
		Topipe = 1;
}

/*
 * Totalitarian Communist pathname processing
 */
void checkpath(char *name)
{
	if (Restricted) {
		if (fopen(name, "rb") != NULL) {
			canit();
			fprintf(stderr, "\r\nrz: %s exists\n", name);
			bibi(-1,-1);
		}
		/* restrict pathnames to current tree or uucppublic */
		if ( substr(name, "../")
		 || (name[0]== '/' && strncmp(name, PUBDIR, strlen(PUBDIR))) ) {
			canit();
			fprintf(stderr,"\r\nrz:\tSecurity Violation\r\n");
			bibi(-1,-1);
		}
	}
}

/*
 * Initialize for Zmodem receive attempt, try to activate Zmodem sender
 *  Handles ZSINIT frame
 *  Return ZFILE if Zmodem filename received, -1 on error,
 *   ZCOMPL if transaction finished,  else 0
 */
int tryz(void)
{
	int c, n;
	int cmdzack1flg;

	if (IsYMODEM)		/* Check for "rb" program name */
		return 0;


	for (n=Zmodem?15:5; --n>=0; ) {
		/* Set buffer length (0) and capability flags */
#ifdef SEGMENTS
		stohdr(SEGMENTS*1024L);
#else
		stohdr(0L);
#endif
#ifdef CANBREAK
		Txhdr[ZF0] = CANFC32|CANFDX|CANOVIO|CANBRK;
#else
		Txhdr[ZF0] = CANFC32|CANFDX|CANOVIO;
#endif
		if (Zctlesc)
			Txhdr[ZF0] |= TESCCTL;
		Txhdr[ZF0] |= CANRLE;
		Txhdr[ZF1] = CANVHDR;
		/* tryzhdrtype may == ZRINIT */
		zshhdr(4,tryzhdrtype, Txhdr);
		if (tryzhdrtype == ZSKIP)	/* Don't skip too far */
			tryzhdrtype = ZRINIT;	/* CAF 8-21-87 */
again:
		switch (zgethdr(Rxhdr, 0)) {
		case ZRQINIT:
			if (Rxhdr[ZF3] & 0x80)
				Usevhdrs = 1;	/* we can var header */
			continue;
		case ZEOF:
			continue;
		case TIMEOUT:
			continue;
		case ZFILE:
			zconv = Rxhdr[ZF0];
			zmanag = Rxhdr[ZF1];
			ztrans = Rxhdr[ZF2];
			if (Rxhdr[ZF3] & ZCANVHDR)
				Usevhdrs = TRUE;
			tryzhdrtype = ZRINIT;
			c = zrdata(secbuf, 1024);
			mode(3);
			if (c == GOTCRCW)
				return ZFILE;
			zshhdr(4,ZNAK, Txhdr);
			goto again;
		case ZSINIT:
			Zctlesc = TESCCTL & Rxhdr[ZF0];
			if (zrdata(Attn, ZATTNLEN) == GOTCRCW) {
				stohdr(1L);
				zshhdr(4,ZACK, Txhdr);
				goto again;
			}
			zshhdr(4,ZNAK, Txhdr);
			goto again;
		case ZFREECNT:
			stohdr(getfree());
			zshhdr(4,ZACK, Txhdr);
			goto again;
		case ZCOMMAND:
			cmdzack1flg = Rxhdr[ZF0];
			if (zrdata(secbuf, 1024) == GOTCRCW) {
				if (cmdzack1flg & ZCACK1)
					stohdr(0L);
				else
					stohdr((long)sys2(secbuf));
				purgeline();	/* dump impatient questions */
				do {
					zshhdr(4,ZCOMPL, Txhdr);
				}
				while (++errors<20 && zgethdr(Rxhdr,1) != ZFIN);
				ackbibi();
				if (cmdzack1flg & ZCACK1)
					exec2(secbuf);
				return ZCOMPL;
			}
			zshhdr(4,ZNAK, Txhdr); goto again;
		case ZCOMPL:
			goto again;
		default:
			continue;
		case ZFIN:
			ackbibi(); return ZCOMPL;
		case ZCAN:
			return ERROR;
		}
	}
	return 0;
}

/*
 * Receive 1 or more files with ZMODEM protocol
 */
int rzfiles(void)
{
	int c;

	for (;;) {
		switch (c = rzfile()) {
		case ZEOF:
		case ZSKIP:
			switch (tryz()) {
			case ZCOMPL:
				return OK;
			default:
				return ERROR;
			case ZFILE:
				break;
			}
			continue;
		default:
			return c;
		case ERROR:
			return ERROR;
		}
	}
}

/*
 * Receive a file with ZMODEM protocol
 *  Assumes file name frame is in secbuf
 */
int rzfile(void)
{
	int c, n;

	Eofseen=FALSE;
	n = 20; rxbytes = 0l;

	if (c = procheader(secbuf)) {
		return (tryzhdrtype = c);
	}

	for (;;) {
#ifdef SEGMENTS
		chinseg = 0;
#endif
		stohdr(rxbytes);
		zshhdr(4,ZRPOS, Txhdr);
nxthdr:
		switch (c = zgethdr(Rxhdr, 0)) {
		default:
			vfile("rzfile: Wrong header %d", c);
			if ( --n < 0) {
				sprintf(endmsg, "rzfile: Wrong header %d", c);
				return ERROR;
			}
			continue;
		case ZCAN:
			sprintf(endmsg, "Sender CANcelled");
			return ERROR;
		case ZNAK:
#ifdef SEGMENTS
			putsec(secbuf, chinseg);
			chinseg = 0;
#endif
			if ( --n < 0) {
				sprintf(endmsg, "rzfile: got ZNAK %d", c);
				return ERROR;
			}
			continue;
		case TIMEOUT:
#ifdef SEGMENTS
			putsec(secbuf, chinseg);
			chinseg = 0;
#endif
			if ( --n < 0) {
				sprintf(endmsg, "rzfile: TIMEOUT %d", c);
				return ERROR;
			}
			continue;
		case ZFILE:
			zrdata(secbuf, 1024);
			continue;
		case ZEOF:
#ifdef SEGMENTS
			putsec(secbuf, chinseg);
			chinseg = 0;
#endif
			if (rclhdr(Rxhdr) != rxbytes) {
				/*
				 * Ignore eof if it's at wrong place - force
				 *  a timeout because the eof might have gone
				 *  out before we sent our zrpos.
				 */
				errors = 0;  goto nxthdr;
			}
			if (closeit()) {
				tryzhdrtype = ZFERR;
				vfile("rzfile: closeit returned <> 0");
				sprintf(endmsg,"Error closing file");
				return ERROR;
			}
			vfile("rzfile: normal EOF");
			return c;
		case ERROR:	/* Too much garbage in header search error */
#ifdef SEGMENTS
			putsec(secbuf, chinseg);
			chinseg = 0;
#endif
			if ( --n < 0) {
				sprintf(endmsg, "Persistent CRC or other ERROR");
				return ERROR;
			}
			zmputs(Attn);
			continue;
		case ZSKIP:
#ifdef SEGMENTS
			putsec(secbuf, chinseg);
			chinseg = 0;
#endif
			Modtime = 1;
			closeit();
			sprintf(endmsg, "Sender SKIPPED file");
			return c;
		case ZDATA:
			if (rclhdr(Rxhdr) != rxbytes) {
				if ( --n < 0) {
					sprintf(endmsg,"Data has bad addr");
					return ERROR;
				}
#ifdef SEGMENTS
				putsec(secbuf, chinseg);
				chinseg = 0;
#endif
				zmputs(Attn);  continue;
			}
moredata:
			if (Verbose>1)
				fprintf(stderr, "\r%7ld ZMODEM%s    ",
				  rxbytes, Crc32r?" CRC-32":"");
#ifdef SEGMENTS
			if (chinseg >= (1024 * SEGMENTS)) {
				putsec(secbuf, chinseg);
				chinseg = 0;
			}
			switch (c = zrdata(secbuf+chinseg, 1024))
#else
			switch (c = zrdata(secbuf, 1024))
#endif
			{
			case ZCAN:
#ifdef SEGMENTS
				putsec(secbuf, chinseg);
				chinseg = 0;
#endif
				sprintf(endmsg, "Sender CANcelled");
				return ERROR;
			case ERROR:	/* CRC error */
#ifdef SEGMENTS
				putsec(secbuf, chinseg);
				chinseg = 0;
#endif
				if ( --n < 0) {
					sprintf(endmsg, "Persistent CRC or other ERROR");
					return ERROR;
				}
				zmputs(Attn);
				continue;
			case TIMEOUT:
#ifdef SEGMENTS
				putsec(secbuf, chinseg);
				chinseg = 0;
#endif
				if ( --n < 0) {
					sprintf(endmsg, "TIMEOUT");
					return ERROR;
				}
				continue;
			case GOTCRCW:
				n = 20;
#ifdef SEGMENTS
				chinseg += Rxcount;
				putsec(secbuf, chinseg);
				chinseg = 0;
#else
				putsec(secbuf, Rxcount);
#endif
				rxbytes += Rxcount;
				stohdr(rxbytes);
				zshhdr(4,ZACK, Txhdr);
				sendline(XON);
				goto nxthdr;
			case GOTCRCQ:
				n = 20;
#ifdef SEGMENTS
				chinseg += Rxcount;
#else
				putsec(secbuf, Rxcount);
#endif
				rxbytes += Rxcount;
				stohdr(rxbytes);
				zshhdr(4,ZACK, Txhdr);
				goto moredata;
			case GOTCRCG:
				n = 20;
#ifdef SEGMENTS
				chinseg += Rxcount;
#else
                                putsec(secbuf, Rxcount);
#endif
				rxbytes += Rxcount;
				goto moredata;
			case GOTCRCE:
				n = 20;
#ifdef SEGMENTS
				chinseg += Rxcount;
#else
				putsec(secbuf, Rxcount);
#endif
				rxbytes += Rxcount;
				goto nxthdr;
			}
		}
	}
}


/*
 * Close the receive dataset, return OK or ERROR
 */
int closeit(void)
{
	if (Topipe) {
		if (pclose(fout)) {
			return ERROR;
		}
		return OK;
	}
	if (fclose(fout)==ERROR) {
		fprintf(stderr, "file close ERROR\n");
		return ERROR;
	}
	if (Modtime) {
		timep[0] = time(NULL);
		timep[1] = Modtime;
		/*utime(Pathname, timep);*/
	}
	if ((Filemode&S_IFMT) == S_IFREG) {
	        if (Filemode & 0400)	        /* If writable, set */
	                Filemode |= 030000;     /* delete and rename */
	      /*chmod(Pathname, ((077777 & Filemode) | 040000));*/ /* and set backup too */
        }
	return OK;
}

/*
 * Ack a ZFIN packet, let byegones be byegones
 */
void ackbibi(void)
{
	int n;

	vfile("ackbibi:");
	Readnum = 1;
	stohdr(0L);
	for (n=3; --n>=0; ) {
		purgeline();
		zshhdr(4,ZFIN, Txhdr);
		switch (readline(100)) {
		case 'O':
			readline(1);	/* Discard 2nd 'O' */
			vfile("ackbibi complete");
			return;
		case RCDO:
			return;
		case TIMEOUT:
		default:
			break;
		}
	}
}


/*
 * Strip leading ! if present, do shell escape. 
 */
int sys2(char *s)
{
	if (*s == '!')
		++s;
	return system(s);
}
/*
 * Strip leading ! if present, do exec.
 */
void exec2(char *s)
{
char buf[80];

    if (*s == '!')
		++s;
	mode(0);
	strcpy(buf,"sh -c ");
    strcat(buf,s);
    execve("/bin/sh",buf);
    /*execl("/bin/sh", "sh", "-c", s);*/
}
/* End of rz.c */
