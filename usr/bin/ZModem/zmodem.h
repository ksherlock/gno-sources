/*
 *   Z M O D E M . H     Manifest constants for ZMODEM
 *    application to application file transfer protocol
 *    Copyright 1991 Omen Technology Inc All Rights Reserved
 *    04-17-89  Chuck Forsberg Omen Technology Inc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/signal.h>
#include <setjmp.h>
#include <time.h>
#ifdef __ORCAC__
#include <orca.h>
#include <stdarg.h>
#include "proto.h"
#endif

#include "config.h"

#ifdef V7
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#define STAT
#include <sgtty.h>
#define OS "V7/BSD"
#define ROPMODE "r"
#ifdef LLITOUT
extern long Locmode;		/* Saved "local mode" for 4.x BSD "new driver" */
extern long Locbit;		/* Bit SUPPOSED to disable output translations */
#endif
#endif

#ifndef OS
#ifndef USG
#define USG
#endif
#endif

#ifdef USG
#include <sys/types.h>
#include <sys/stat.h>
#define STAT
#include <termio.h>
#define OS "SYS III/V"
#define ROPMODE "rb"
#define MODE2OK
#include <string.h>
#endif

#ifdef T6K
#include <sys/ioctl.h>		/* JPRadley: for the Tandy 6000 */
#endif

#if HOWMANY  > 255
Howmany must be 255 or less
#endif

 /*
 *  Some systems (Venix, Coherent, Regulus) may not support tty raw mode
 *  read(2) the same way as Unix. ONEREAD must be defined to force one
 *  character reads for these systems. Added 7-01-84 CAF
 */


#ifdef FASTOUT
extern void xsendline(int);
#define sendline(c) xsendline(c & 0377)
extern char outbuf[512];
extern unsigned outbufInd;
#else
#define sendline(c) putc(c & 0377, Ttystream)
#define xsendline(c) putc((c), Ttystream)
#endif

#define READCHECK	/* we can check data coming back, ya know */
int rdchk(int); 	/* returns number of bytes of data waiting on port */

#define ZPAD '*'	/* 052 Padding character begins frames */
#define ZDLE 030	/* Ctrl-X Zmodem escape - `ala BISYNC DLE */
#define ZDLEE (ZDLE^0100)	/* Escaped ZDLE as transmitted */
#define ZBIN 'A'	/* Binary frame indicator (CRC-16) */
#define ZHEX 'B'	/* HEX frame indicator */
#define ZBIN32 'C'	/* Binary frame with 32 bit FCS */
#define ZBINR32 'D'	/* RLE packed Binary frame with 32 bit FCS */
#define ZVBIN 'a'	/* Binary frame indicator (CRC-16) */
#define ZVHEX 'b'	/* HEX frame indicator */
#define ZVBIN32 'c'	/* Binary frame with 32 bit FCS */
#define ZVBINR32 'd'	/* RLE packed Binary frame with 32 bit FCS */
#define ZRESC	0176	/* RLE flag/escape character */
#define ZMAXHLEN 16	/* Max header information length  NEVER CHANGE */
#define ZMAXSPLEN 1024	/* Max subpacket length  NEVER CHANGE */

/* Frame types (see array "frametypes" in zm.c) */
#define ZRQINIT	0	/* Request receive init */
#define ZRINIT	1	/* Receive init */
#define ZSINIT 2	/* Send init sequence (optional) */
#define ZACK 3		/* ACK to above */
#define ZFILE 4		/* File name from sender */
#define ZSKIP 5		/* To sender: skip this file */
#define ZNAK 6		/* Last packet was garbled */
#define ZABORT 7	/* Abort batch transfers */
#define ZFIN 8		/* Finish session */
#define ZRPOS 9		/* Resume data trans at this position */
#define ZDATA 10	/* Data packet(s) follow */
#define ZEOF 11		/* End of file */
#define ZFERR 12	/* Fatal Read or Write error Detected */
#define ZCRC 13		/* Request for file CRC and response */
#define ZCHALLENGE 14	/* Receiver's Challenge */
#define ZCOMPL 15	/* Request is complete */
#define ZCAN 16		/* Other end canned session with CAN*5 */
#define ZFREECNT 17	/* Request for free bytes on filesystem */
#define ZCOMMAND 18	/* Command from sending program */
#define ZSTDERR 19	/* Output to standard error, data follows */

/* ZDLE sequences */
#define ZCRCE 'h'	/* CRC next, frame ends, header packet follows */
#define ZCRCG 'i'	/* CRC next, frame continues nonstop */
#define ZCRCQ 'j'	/* CRC next, frame continues, ZACK expected */
#define ZCRCW 'k'	/* CRC next, ZACK expected, end of frame */
#define ZRUB0 'l'	/* Translate to rubout 0177 */
#define ZRUB1 'm'	/* Translate to rubout 0377 */

/* zdlread return values (internal) */
/* -1 is general error, -2 is timeout */
#define GOTOR 0400
#define GOTCRCE (ZCRCE|GOTOR)	/* ZDLE-ZCRCE received */
#define GOTCRCG (ZCRCG|GOTOR)	/* ZDLE-ZCRCG received */
#define GOTCRCQ (ZCRCQ|GOTOR)	/* ZDLE-ZCRCQ received */
#define GOTCRCW (ZCRCW|GOTOR)	/* ZDLE-ZCRCW received */
#define GOTCAN	(GOTOR|030)	/* CAN*5 seen */

/* Byte positions within header array */
#define ZF0	3	/* First flags byte */
#define ZF1	2
#define ZF2	1
#define ZF3	0
#define ZP0	0	/* Low order 8 bits of position */
#define ZP1	1
#define ZP2	2
#define ZP3	3	/* High order 8 bits of file position */

/* Bit Masks for ZRINIT flags byte ZF0 */
#define CANFDX	01	/* Rx can send and receive true FDX */
#define CANOVIO	02	/* Rx can receive data during disk I/O */
#define CANBRK	04	/* Rx can send a break signal */
#define CANRLE	010	/* Receiver can decode RLE */
#define CANLZW	020	/* Receiver can uncompress */
#define CANFC32	040	/* Receiver can use 32 bit Frame Check */
#define ESCCTL 0100	/* Receiver expects ctl chars to be escaped */
#define ESC8   0200	/* Receiver expects 8th bit to be escaped */

/* Bit Masks for ZRINIT flags byte ZF1 */
#define CANVHDR	01	/* Variable headers OK */

/* Parameters for ZSINIT frame */
#define ZATTNLEN 32	/* Max length of attention string */
#define ALTCOFF ZF1	/* Offset to alternate canit string, 0 if not used */
/* Bit Masks for ZSINIT flags byte ZF0 */
#define TESCCTL 0100	/* Transmitter expects ctl chars to be escaped */
#define TESC8   0200	/* Transmitter expects 8th bit to be escaped */

/* Parameters for ZFILE frame */
/* Conversion options one of these in ZF0 */
#define ZCBIN	1	/* Binary transfer - inhibit conversion */
#define ZCNL	2	/* Convert NL to local end of line convention */
#define ZCRESUM	3	/* Resume interrupted file transfer */
/* Management include options, one of these ored in ZF1 */
#define ZMSKNOLOC	0200	/* Skip file if not present at rx */
/* Management options, one of these ored in ZF1 */
#define ZMMASK	037	/* Mask for the choices below */
#define ZMNEWL	1	/* Transfer if source newer or longer */
#define ZMCRC	2	/* Transfer if different file CRC or length */
#define ZMAPND	3	/* Append contents to existing file (if any) */
#define ZMCLOB	4	/* Replace existing file */
#define ZMNEW	5	/* Transfer if source newer */
	/* Number 5 is alive ... */
#define ZMDIFF	6	/* Transfer if dates or lengths different */
#define ZMPROT	7	/* Protect destination file */
#define ZMCHNG	8	/* Change filename if destination exists */
/* Transport options, one of these in ZF2 */
#define ZTLZW	1	/* Lempel-Ziv compression */
#define ZTRLE	3	/* Run Length encoding */
/* Extended options for ZF3, bit encoded */
#define ZXSPARS	64	/* Encoding for sparse file operations */
#define ZCANVHDR	01	/* Variable headers OK */
/* Receiver window size override */
#define ZRWOVR 4	/* byte position for receive window override/256 */

/* Parameters for ZCOMMAND frame ZF0 (otherwise 0) */
#define ZCACK1	1	/* Acknowledge, then do command */

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
#define WANTG 0107	/* Send G not NAK to get nonstop batch xmsn */
#define RCDO (-3)
#define GCOUNT (-4)
#define RETRYMAX 10

long rclhdr();

/* Globals used by ZMODEM functions */
extern Rxframeind;	/* ZBIN ZBIN32, or ZHEX type of frame */
extern Rxtype;		/* Type of header received */
extern Rxcount;		/* Count of data bytes received */
extern Rxtimeout;	/* Tenths of seconds to wait for something */
extern long Rxpos;	/* Received file position */
extern long Txpos;	/* Transmitted file position */
extern Txfcs32;		/* TURE means send binary frames with 32 bit FCS */
extern Crc32t;		/* Display flag indicating 32 bit CRC being sent */
extern Crc32;		/* Display flag indicating 32 bit CRC being received */
extern Znulls;		/* Number of nulls to send at beginning of ZDATA hdr */
extern char Attn[ZATTNLEN+1];	/* Attention string rx sends to tx on err */
extern char *Altcan;	/* Alternate canit string */

extern FILE *Ttystream;
extern int Tty;
extern char linbuf[256]; /* maximum size of howmany */
extern char xXbuf[BUFSIZ];
extern int Lleft;		/* number of characters in linbuf */
extern jmp_buf tohere;		/* For the interrupt on RX timeout */
extern int Readnum;		/* number of bytes to read in a chunk */
extern int Verbose;		/* Debug verbosity flag */
extern int Zmodem;		/* Zmodem transfer flag */
extern int Effbaud;		/* effective baud rate */
extern int Baudrate;		/* actual baud rate */

struct speedsStruct {
	unsigned baudr;
	int speedcode;
};

extern struct speedsStruct speeds[];

extern int Twostop;		/* Use two stop bits */

/*----------------------------------------------------------------*/
#define OK 0
#define TIMEOUT (-2)
#define ERROR (-1)

/*----------------------------------------------------------------*
	Globals from zm.c and zmr.c
 *----------------------------------------------------------------*/
/* Globals used by ZMODEM functions */
extern int Rxframeind;
extern int Rxtype;
extern int Rxhlen;
extern int Rxcount;
extern char Rxhdr[ZMAXHLEN];
extern char Txhdr[ZMAXHLEN];
extern long Rxpos;
extern long Txpos;
extern int Txfcs32;
extern int Crc32t;

extern int Crc32r;

extern int Usevhdrs;
extern int Znulls;
extern char Attn[ZATTNLEN+1];
extern char *Altcan;

extern int lastsent;
extern int Not8bit;

#define FTOFFSET 4
#define FRTYPES 22	/*  Number of entries in the frametypes array	*/
			/*  not including psuedo negative entries	*/
extern char *frametypes[FRTYPES];

extern char badcrc[];

#ifdef ICANON
extern struct termio oldtty, tty;
#else
extern struct sgttyb oldtty, tty;
extern struct tchars oldtch, tch;
#endif

/* CRC interface */
extern unsigned short crctab[256];
#define updcrc(cp, crc) ( crctab[((crc >> 8) & 255)] ^ (crc << 8) ^ cp)
extern UNSL long crc;

/* RZ.C */
extern int Zctlesc;	/* Encode control characters */
extern int IsYMODEM;	/* If invoked as "sb" */
extern char *Progname;
extern int Zrwindow;	/* RX window size (controls garbage count) */

/* ZM.C */
void zsendline(int c);
extern char *Zendnames[];
extern int Rxtimeout;		/* Tenths of seconds to wait for something */

#pragma optimize 9
/* End of ZMODEM.H */
