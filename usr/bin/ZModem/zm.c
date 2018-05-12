#pragma optimize 9

/*
 *   Z M . C
 *    Copyright 1991 Omen Technology Inc All Rights Reserved
 *    ZMODEM protocol primitives
 *    05-24-89  Chuck Forsberg Omen Technology Inc
 *
 * Entry point Functions:
 *	zsbhdr(type, hdr) send binary header
 *	zshhdr(type, hdr) send hex header
 *	zgethdr(hdr, eflag) receive header - binary or hex
 *	zsdata(buf, len, frameend) send data
 *	zrdata(buf, len) receive data
 *	stohdr(pos) store position data in Txhdr
 *	long rclhdr(hdr) recover position offset from header
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
 */

#include "zmodem.h"

#ifndef CANFDX
#endif

#ifndef UNSL
#define UNSL
#endif

#ifdef __ORCAC__
#pragma lint -1
#endif

/* Send ZMODEM binary header hdr of type type */
int zsbhdr(int len, int type, char *hdr)
{
	register int n;
	register unsigned short crc;

#ifndef DSZ
	vfile("zsbhdr: %s %ld %s %lx", Usevhdrs?"v":"f", (long)len,
	  frametypes[type+FTOFFSET], rclhdr(hdr));
#endif
	if (type == ZDATA)
		for (n = Znulls; --n >=0; )
			xsendline(0);

	xsendline(ZPAD); xsendline(ZDLE);

	switch (Crc32t=Txfcs32) {
	case 2:
		zsbh32(len, hdr, type, Usevhdrs?ZVBINR32:ZBINR32);
		flushmo();  break;
	case 1:
		zsbh32(len, hdr, type, Usevhdrs?ZVBIN32:ZBIN32);  break;
	default:
		if (Usevhdrs) {
			xsendline(ZVBIN);
			zsendline(len);
		}
		else
			xsendline(ZBIN);
		zsendline(type);
		crc = updcrc(type, 0);

		for (n=len; --n >= 0; ++hdr) {
			zsendline(*hdr);
			crc = updcrc((0377& *hdr), crc);
		}
		crc = updcrc(0,updcrc(0,crc));
		zsendline(crc>>8);
		zsendline(crc);
	}
	if (type != ZDATA)
		flushmo();
}


/* Send ZMODEM binary header hdr of type type */
int zsbh32(int len, char *hdr, int type, int flavour)
{
	register int n;
	register UNSL long crc;

	xsendline(flavour); 
	if (Usevhdrs) 
		zsendline(len);
	zsendline(type);
	crc = 0xFFFFFFFFL; crc = UPDC32(type, crc);

	for (n=len; --n >= 0; ++hdr) {
		crc = UPDC32((0377 & *hdr), crc);
		zsendline(*hdr);
	}
	crc = ~crc;
	for (n=4; --n >= 0;) {
		zsendline((int)crc);
		crc >>= 8;
	}
}

/* Send ZMODEM HEX header hdr of type type */
int zshhdr(int len, int type, char *hdr)
{
	register int n;
	register unsigned short crc;

#ifndef DSZ
	vfile("zshhdr: %s %ld %s %lx", Usevhdrs?"v":"f", (long)len,
	  frametypes[type+FTOFFSET], rclhdr(hdr));
#endif
	sendline(ZPAD); sendline(ZPAD); sendline(ZDLE);
	if (Usevhdrs) {
		sendline(ZVHEX);
		zputhex(len);
	}
	else
		sendline(ZHEX);
	zputhex(type);
	Crc32t = 0;

	crc = updcrc(type, 0);
	for (n=len; --n >= 0; ++hdr) {
		zputhex(*hdr); crc = updcrc((0377 & *hdr), crc);
	}
	crc = updcrc(0,updcrc(0,crc));
	zputhex(crc>>8); zputhex(crc);

	/* Make it printable on remote machine */
	sendline(015); sendline(0212);
	/*
	 * Uncork the remote in case a fake XOFF has stopped data flow
	 */
	if (type != ZFIN && type != ZACK)
		sendline(021);
	flushmo();
}

/*
 * Send binary array buf of length length, with ending ZDLE sequence frameend
 */
int zsdata(char *buf, int length, int frameend)
{
	register unsigned short crc;

#ifndef DSZ
	vfile("zsdata: %ld %s", (long)length, Zendnames[frameend-ZCRCE&3]);
#endif
	switch (Crc32t) {
	case 1:
		zsda32(buf, length, frameend);  break;
	case 2:
		zsdar32(buf, length, frameend);  break;
	default:
		crc = 0;
		for (;--length >= 0; ++buf) {
			zsendline(*buf); crc = updcrc((0377 & *buf), crc);
		}
		xsendline(ZDLE); xsendline(frameend);
		crc = updcrc(frameend, crc);

		crc = updcrc(0,updcrc(0,crc));
		zsendline(crc>>8); zsendline(crc);
	}
	if (frameend == ZCRCW)
		xsendline(XON);
	if (frameend != ZCRCG)
		flushmo();
}

/*#pragma optimize -1*/
int zsda32(char *buf, int length, int frameend)
{
	register int c;
	register unsigned long crc;

	crc = 0xFFFFFFFFL;
	for (;--length >= 0; ++buf) {
		c = *buf & 0377;
		if (c & 0140)
			xsendline(lastsent = c);
		else
			zsendline(c);
		crc = UPDC32(c, crc);
	}
	xsendline(ZDLE); xsendline(frameend);
	crc = UPDC32(frameend, crc);

	crc = ~crc;
	for (c=4; --c >= 0;) {
		zsendline((int)crc);  crc >>= 8;
	}
}

/*#pragma optimize 8*/

/*
 * Receive array buf of max length with ending ZDLE sequence
 *  and CRC.  Returns the ending character or error code.
 *  NB: On errors may store length+1 bytes!
 */
int zrdata(char *buf, int length)
{
	register int c;
	register unsigned short crc;
	register char *end;
	register int d;

	switch (Crc32r) {
	case 1:
		return zrdat32(buf, length);
	case 2:
		return zrdatr32(buf, length);
	}

	crc = Rxcount = 0;  end = buf + length;
	while (buf <= end) {
		if ((c = zdlread()) & ~0377) {
crcfoo:
			switch (c) {
			case GOTCRCE:
			case GOTCRCG:
			case GOTCRCQ:
			case GOTCRCW:
				crc = updcrc((d=c)&0377, crc);
				if ((c = zdlread()) & ~0377)
					goto crcfoo;
				crc = updcrc(c, crc);
				if ((c = zdlread()) & ~0377)
					goto crcfoo;
				crc = updcrc(c, crc);
				if (crc & 0xFFFF) {
					zperr(badcrc);
					return ERROR;
				}
				Rxcount = length - (end - buf);
#ifndef DSZ
				vfile("zrdata: %ld  %s", (long)Rxcount,
				 Zendnames[d-GOTCRCE&3]);
#endif
				return d;
			case GOTCAN:
				zperr("Sender Canceled");
				return ZCAN;
			case TIMEOUT:
				zperr("TIMEOUT");
				return c;
			default:
				garbitch(); return c;
			}
		}
		*buf++ = c;
		crc = updcrc(c, crc);
	}
#ifdef DSZ
	garbitch(); 
#else
	zperr("Data subpacket too long");
#endif
	return ERROR;
}

int zrdat32(char *buf, int length)
{
	register int c;
	register UNSL long crc;
	register char *end;
	register int d;

	crc = 0xFFFFFFFFL;  Rxcount = 0;  end = buf + length;
	while (buf <= end) {
		if ((c = zdlread()) & ~0377) {
crcfoo:
			switch (c) {
			case GOTCRCE:
			case GOTCRCG:
			case GOTCRCQ:
			case GOTCRCW:
				d = c;  c &= 0377;
				crc = UPDC32(c, crc);
				if ((c = zdlread()) & ~0377)
					goto crcfoo;
				crc = UPDC32(c, crc);
				if ((c = zdlread()) & ~0377)
					goto crcfoo;
				crc = UPDC32(c, crc);
				if ((c = zdlread()) & ~0377)
					goto crcfoo;
				crc = UPDC32(c, crc);
				if ((c = zdlread()) & ~0377)
					goto crcfoo;
				crc = UPDC32(c, crc);
				if (crc != 0xDEBB20E3) {
					zperr(badcrc);
					return ERROR;
				}
				Rxcount = length - (end - buf);
#ifndef DSZ
				vfile("zrdat32: %ld %s", (long)Rxcount,
				 Zendnames[d-GOTCRCE&3]);
#endif
				return d;
			case GOTCAN:
				zperr("Sender Canceled");
				return ZCAN;
			case TIMEOUT:
				zperr("TIMEOUT");
				return c;
			default:
				garbitch(); return c;
			}
		}
		*buf++ = c;
		crc = UPDC32(c, crc);
	}
	zperr("Data subpacket too long");
	return ERROR;
}

int garbitch(void)
{
	zperr("Garbled data subpacket");
}

char *onech = "a";
/*
 * Read a ZMODEM header to hdr, either binary or hex.
 *  eflag controls local display of non zmodem characters:
 *	0:  no display
 *	1:  display printing characters only
 *	2:  display all non ZMODEM characters
 *
 *   Set Rxhlen to size of header (default 4) (valid iff good hdr)
 *  On success, set Zmodem to 1, set Rxpos and return type of header.
 *   Otherwise return negative on error.
 *   Return ERROR instantly if ZCRCW sequence, for fast error recovery.
 */
int zgethdr(char *hdr, int eflag)
{
	register int c, n, cancount;

	n = Zrwindow + Effbaud;		/* Max bytes before start of frame */
	Rxframeind = Rxtype = 0;

startover:
	cancount = 5;
again:
	/* Return immediate ERROR if ZCRCW sequence seen */
	switch (c = readline(Rxtimeout)) {
	case 021: case 0221:
		goto again;
	case RCDO:
	case TIMEOUT:
		goto fifi;
	case CAN:
gotcan:
		if (--cancount <= 0) {
			c = ZCAN; goto fifi;
		}
		switch (c = readline(1)) {
		case TIMEOUT:
			goto again;
		case ZCRCW:
			switch (readline(1)) {
			case TIMEOUT:
				c = ERROR; goto fifi;
			case RCDO:
				goto fifi;
			default:
				goto agn2;
			}
		case RCDO:
			goto fifi;
		default:
			break;
		case CAN:
			if (--cancount <= 0) {
				c = ZCAN; goto fifi;
			}
			goto again;
		}
	/* **** FALL THRU TO **** */
	default:
agn2:
		if ( --n == 0) {
			c = GCOUNT;  goto fifi;
		}
		goto startover;
	case ZPAD|0200:		/* This is what we want. */
		Not8bit = c;
	case ZPAD:		/* This is what we want. */
		break;
	}
	cancount = 5;
splat:
	switch (c = noxrd7()) {
	case ZPAD:
		goto splat;
	case RCDO:
	case TIMEOUT:
		goto fifi;
	default:
		goto agn2;
	case ZDLE:		/* This is what we want. */
		break;
	}


	Rxhlen = 4;		/* Set default length */
	Rxframeind = c = noxrd7();
	switch (c) {
	case ZVBIN32:
		if ((Rxhlen = c = zdlread()) < 0)
			goto fifi;
		if (c > ZMAXHLEN)
			goto agn2;
		Crc32r = 1;  c = zrbhd32(hdr); break;
	case ZBIN32:
		if (Usevhdrs)
			goto agn2;
		Crc32r = 1;  c = zrbhd32(hdr); break;
	case ZVBINR32:
		if ((Rxhlen = c = zdlread()) < 0)
			goto fifi;
		if (c > ZMAXHLEN)
			goto agn2;
		Crc32r = 2;  c = zrbhd32(hdr); break;
	case ZBINR32:
		if (Usevhdrs)
			goto agn2;
		Crc32r = 2;  c = zrbhd32(hdr); break;
	case RCDO:
	case TIMEOUT:
		goto fifi;
	case ZVBIN:
		if ((Rxhlen = c = zdlread()) < 0)
			goto fifi;
		if (c > ZMAXHLEN)
			goto agn2;
		Crc32r = 0;  c = zrbhdr(hdr); break;
	case ZBIN:
		if (Usevhdrs)
			goto agn2;
		Crc32r = 0;  c = zrbhdr(hdr); break;
	case ZVHEX:
		if ((Rxhlen = c = zgethex()) < 0)
			goto fifi;
		if (c > ZMAXHLEN)
			goto agn2;
		Crc32r = 0;  c = zrhhdr(hdr); break;
	case ZHEX:
		if (Usevhdrs)
			goto agn2;
		Crc32r = 0;  c = zrhhdr(hdr); break;
	case CAN:
		goto gotcan;
	default:
		goto agn2;
	}
	for (n = Rxhlen; ++n < ZMAXHLEN; )	/* Clear unused hdr bytes */
		hdr[n] = 0;
	Rxpos = hdr[ZP3] & 0377;
	Rxpos = (Rxpos<<8) + (hdr[ZP2] & 0377);
	Rxpos = (Rxpos<<8) + (hdr[ZP1] & 0377);
	Rxpos = (Rxpos<<8) + (hdr[ZP0] & 0377);
fifi:
	switch (c) {
	case GOTCAN:
		c = ZCAN;
	/* **** FALL THRU TO **** */
	case ZNAK:
	case ZCAN:
	case ERROR:
	case TIMEOUT:
	case RCDO:
	case GCOUNT:
		zperr("Got %s", frametypes[c+FTOFFSET]);
	/* **** FALL THRU TO **** */
#ifndef DSZ
	default:
		if (c >= -4 && c <= FRTYPES) {
			onech[0] = Rxframeind;
            vfile("zgethdr: %s %ld %s %lx", onech, (long)Rxhlen,
			  frametypes[c+FTOFFSET], Rxpos);
        }
        else {
			onech[0] = Rxframeind;
            vfile("zgethdr: %s %ld %lx", onech, (long)c, Rxpos);
        }
#endif
	}
	/* Use variable length headers if we got one */
	if (c >= 0 && c <= FRTYPES && Rxframeind & 040)
		Usevhdrs = 1;
        return c;
}

/* Receive a binary style header (type and position) */
int zrbhdr(char *hdr)
{
	register int c, n;
	register unsigned short crc;

	if ((c = zdlread()) & ~0377)
		return c;
	Rxtype = c;
	crc = updcrc(c, 0);

	for (n=Rxhlen; --n >= 0; ++hdr) {
		if ((c = zdlread()) & ~0377)
			return c;
		crc = updcrc(c, crc);
		*hdr = c;
	}
	if ((c = zdlread()) & ~0377)
		return c;
	crc = updcrc(c, crc);
	if ((c = zdlread()) & ~0377)
		return c;
	crc = updcrc(c, crc);
	if (crc & 0xFFFF) {
		zperr(badcrc);
		return ERROR;
	}
#ifdef ZMODEM
	Protocol = ZMODEM;
#endif
	Zmodem = 1;
	return Rxtype;
}

/* Receive a binary style header (type and position) with 32 bit FCS */
int zrbhd32(char *hdr)
{
	register int c, n;
	register UNSL long crc;

	if ((c = zdlread()) & ~0377)
		return c;
	Rxtype = c;
	crc = 0xFFFFFFFFL; crc = UPDC32(c, crc);
#ifdef DEBUGZ
	vfile("zrbhd32 c=%lX  crc=%lX", (long)c, crc);
#endif

	for (n=Rxhlen; --n >= 0; ++hdr) {
		if ((c = zdlread()) & ~0377)
			return c;
		crc = UPDC32(c, crc);
		*hdr = c;
#ifdef DEBUGZ
		vfile("zrbhd32 c=%lX  crc=%lX", (long)c, crc);
#endif
	}
	for (n=4; --n >= 0;) {
		if ((c = zdlread()) & ~0377)
			return c;
		crc = UPDC32(c, crc);
#ifdef DEBUGZ
		vfile("zrbhd32 c=%lX  crc=%lX", (long)c, crc);
#endif
	}
	if (crc != 0xDEBB20E3) {
		zperr(badcrc);
		return ERROR;
	}
#ifdef ZMODEM
	Protocol = ZMODEM;
#endif
	Zmodem = 1;
	return Rxtype;
}


/* Receive a hex style header (type and position) */
int zrhhdr(char *hdr)
{
	register int c;
	register unsigned short crc;
	register int n;

	if ((c = zgethex()) < 0)
		return c;
	Rxtype = c;
	crc = updcrc(c, 0);

	for (n=Rxhlen; --n >= 0; ++hdr) {
		if ((c = zgethex()) < 0)
			return c;
		crc = updcrc(c, crc);
		*hdr = c;
	}
	if ((c = zgethex()) < 0)
		return c;
	crc = updcrc(c, crc);
	if ((c = zgethex()) < 0)
		return c;
	crc = updcrc(c, crc);
	if (crc & 0xFFFF) {
		zperr(badcrc); return ERROR;
	}
	switch ( c = readline(2)) {
	case 0215:
		Not8bit = c;
		/* **** FALL THRU TO **** */
	case 015:
	 	/* Throw away possible cr/lf */
		switch (c = readline(2)) {
		case 012:
			Not8bit |= c;
		}
	}
#ifdef ZMODEM
	Protocol = ZMODEM;
#endif
	Zmodem = 1;
	if (c < 0)
		return c;
	return Rxtype;
}

/* Send a byte as two hex digits */
void zputhex(int c)
{
	static char	digits[]	= "0123456789abcdef";

#ifdef DEBUGZ
	if (Verbose>8)
		vfile("zputhex: %02lX", (long)c);
#endif
	sendline(digits[(c&0xF0)>>4]);
	sendline(digits[(c)&0xF]);
}

/*
 * Send character c with ZMODEM escape sequence encoding.
 *  Escape XON, XOFF. Escape CR following @ (Telenet net escape)
 */
/*#pragma optimize -1*/
#ifndef FASTOUT
void zsendline(int c)
{

	/* Quick check for non control characters */
	if (c & 0140)
		xsendline(lastsent = c);
	else {
		switch (c &= 0377) {
		case ZDLE:
			xsendline(ZDLE);
			xsendline (lastsent = (c ^= 0100));
			break;
		case 015:
		case 0215:
			if (!Zctlesc && (lastsent & 0177) != '@')
				goto sendit;
		/* **** FALL THRU TO **** */
		case 020:
		case 021:
		case 023:
		case 0220:
		case 0221:
		case 0223:
			xsendline(ZDLE);
			c ^= 0100;
	sendit:
			xsendline(lastsent = c);
			break;
		default:
			if (Zctlesc && ! (c & 0140)) {
				xsendline(ZDLE);
				c ^= 0100;
			}
			xsendline(lastsent = c);
		}
	}
}
#endif
/*#pragma optimize 8*/

/* Decode two lower case hex digits into an 8 bit byte value */
int zgethex(void)
{
	register int c;

	c = zgeth1();
#ifdef DEBUGZ
	if (Verbose>8)
		vfile("zgethex: %02X", c);
#endif
	return c;
}
int zgeth1(void)
{
	register int c, n;

	if ((c = noxrd7()) < 0)
		return c;
	n = c - '0';
	if (n > 9)
		n -= ('a' - ':');
	if (n & ~0xF)
		return ERROR;
	if ((c = noxrd7()) < 0)
		return c;
	c -= '0';
	if (c > 9)
		c -= ('a' - ':');
	if (c & ~0xF)
		return ERROR;
	c += (n<<4);
	return c;
}

/*
 * Read a byte, checking for ZMODEM escape encoding
 *  including CAN*5 which represents a quick abort
 */
int zdlread(void)
{
	register int c;

again:
	/* Quick check for non control characters */
	if ((c = readline(Rxtimeout)) & 0140)
		return c;
	switch (c) {
	case ZDLE:
		break;
	case 023:
	case 0223:
	case 021:
	case 0221:
		goto again;
	default:
		if (Zctlesc && !(c & 0140)) {
			goto again;
		}
		return c;
	}
again2:
	if ((c = readline(Rxtimeout)) < 0)
		return c;
	if (c == CAN && (c = readline(Rxtimeout)) < 0)
		return c;
	if (c == CAN && (c = readline(Rxtimeout)) < 0)
		return c;
	if (c == CAN && (c = readline(Rxtimeout)) < 0)
		return c;
	switch (c) {
	case CAN:
		return GOTCAN;
	case ZCRCE:
	case ZCRCG:
	case ZCRCQ:
	case ZCRCW:
		return (c | GOTOR);
	case ZRUB0:
		return 0177;
	case ZRUB1:
		return 0377;
	case 023:
	case 0223:
	case 021:
	case 0221:
		goto again2;
	default:
		if (Zctlesc && ! (c & 0140)) {
			goto again2;
		}
		if ((c & 0140) ==  0100)
			return (c ^ 0100);
		break;
	}
	if (Verbose>1)
		zperr("Bad escape sequence %x", c);
	return ERROR;
}

/*
 * Read a character from the modem line with timeout.
 *  Eat parity, XON and XOFF characters.
 */
int noxrd7(void)
{
	register int c;

	for (;;) {
		if ((c = readline(Rxtimeout)) < 0)
			return c;
		switch (c &= 0177) {
		case XON:
		case XOFF:
			continue;
		default:
			if (Zctlesc && !(c & 0140))
				continue;
		case '\r':
		case '\n':
		case ZDLE:
			return c;
		}
	}
}

/* Store long integer pos in Txhdr */
void stohdr(long int pos)
{
	Txhdr[ZP0] = pos;
	Txhdr[ZP1] = pos>>8;
	Txhdr[ZP2] = pos>>16;
	Txhdr[ZP3] = pos>>24;
}

/* Recover a long integer from a header */
long rclhdr(char *hdr)
{
	register long l;

	l = (hdr[ZP3] & 0377);
	l = (l << 8) | (hdr[ZP2] & 0377);
	l = (l << 8) | (hdr[ZP1] & 0377);
	l = (l << 8) | (hdr[ZP0] & 0377);
	return l;
}

/* End of zm.c */
