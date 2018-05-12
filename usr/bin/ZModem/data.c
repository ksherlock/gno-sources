/*
 *	data.c
 *
 *	Global variables used throughout the program
 */

#include <stdio.h>
#include <setjmp.h>
#include <sys/ioctl.h>
#include "config.h"

#ifdef __ORCAC__
#pragma noroot
#endif

#ifdef V7
#ifdef LLITOUT
long Locmode;		/* Saved "local mode" for 4.x BSD "new driver" */
long Locbit = LLITOUT;	/* Bit SUPPOSED to disable output translations */
#endif
#endif

int howmany;
FILE *Ttystream;
int Tty;
char linbuf[256];
char xXbuf[BUFSIZ];
int Lleft=0;		/* number of characters in linbuf */
jmp_buf tohere;		/* For the interrupt on RX timeout */
#ifdef ONEREAD
/* Sorry, Regulus and some others don't work right in raw mode! */
int Readnum = 1;	/* Number of bytes to ask for in read() from modem */
#else
int Readnum = 2;	/* Number of bytes to ask for in read() from modem */
#endif
int Verbose=0;

#ifdef ICANON
struct termio oldtty, tty;
#else
struct sgttyb oldtty, tty;
struct tchars oldtch, tch;
#endif

struct speedsStruct {
	unsigned baudr;
	int speedcode;
};

struct speedsStruct speeds[] = {
	{110,	B110   },
	{300,	B300   },
	{600,	B600   },
	{1200,	B1200  },
	{2400,	B2400  },
	{4800,	B4800  },
	{9600,	B9600  },
	{19200,	EXTA   },
	{38400,	EXTB   },
	{0,	0      }
};

int Twostop;		/* Use two stop bits */

#define ZMAXHLEN 16	/* Max header information length  NEVER CHANGE */
#define ZATTNLEN 32	/* Max length of attention string */

/* Globals used by ZMODEM functions */
int Rxframeind;		/* ZBIN ZBIN32, or ZHEX type of frame */
int Rxtype;		/* Type of header received */
int Rxhlen;		/* Length of header received */
int Rxcount;		/* Count of data bytes received */
char Rxhdr[ZMAXHLEN];	/* Received header */
char Txhdr[ZMAXHLEN];	/* Transmitted header */
long Rxpos;		/* Received file position */
long Txpos;		/* Transmitted file position */
int Txfcs32;		/* TURE means send binary frames with 32 bit FCS */
int Crc32t;		/* Controls 32 bit CRC being sent */
			/* 1 == CRC32,  2 == CRC32 + RLE */
int Crc32r;		/* Indicates/controls 32 bit CRC being received */
			/* 0 == CRC16,  1 == CRC32,  2 == CRC32 + RLE */
int Crc32;
unsigned long crc;
int Usevhdrs;		/* Use variable length headers */
int Znulls;		/* Number of nulls to send at beginning of ZDATA hdr */
char Attn[ZATTNLEN+1];	/* Attention string rx sends to tx on err */
char *Altcan;		/* Alternate canit string */

int lastsent;	/* Last char we sent */
int Not8bit;		/* Seven bits seen on header */

char *frametypes[] = {
	"No Response to Error Correction Request",	/* -4 */
	"No Carrier Detect",		/* -3 */
	"TIMEOUT",		/* -2 */
	"ERROR",		/* -1 */
	"ZRQINIT",
	"ZRINIT",
	"ZSINIT",
	"ZACK",
	"ZFILE",
	"ZSKIP",
	"ZNAK",
	"ZABORT",
	"ZFIN",
	"ZRPOS",
	"ZDATA",
	"ZEOF",
	"ZFERR",
	"ZCRC",
	"ZCHALLENGE",
	"ZCOMPL",
	"ZCAN",
	"ZFREECNT",
	"ZCOMMAND",
	"ZSTDERR",
	"xxxxx"
};

char badcrc[] = "Bad CRC";

int Zctlesc;	/* Encode control characters */
int IsYMODEM = 0;	/* If invoked as "sb" */
char *Progname = "sz";
int Zrwindow = 1400;	/* RX window size (controls garbage count) */

/* ZM.C */
char *Zendnames[] = { "ZCRCE", "ZCRCG", "ZCRCQ", "ZCRCW"};
int Rxtimeout = 100;		/* Tenths of seconds to wait for something */

#ifdef FASTOUT
char outbuf[512];
unsigned outbufInd = 0;
#endif
