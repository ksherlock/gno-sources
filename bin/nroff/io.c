/*
 *	io.c - low level I/O processing portion of nroff word processor
 *
 *	adapted for atariST/TOS by Bill Rosenkranz 11/89
 *	net:	rosenkra@hall.cray.com
 *	CIS:	71460,17
 *	GENIE:	W.ROSENKRANZ
 *
 *	original author:
 *
 *	Stephen L. Browning
 *	5723 North Parker Avenue
 *	Indianapolis, Indiana 46220
 *
 *	history:
 *
 *	- Originally written in BDS C;
 *	- Adapted for standard C by W. N. Paul
 *	- Heavily hacked up to conform to "real" nroff by Bill Rosenkranz
 */

#ifdef __ORCAC__
segment "seg2      ";
#endif

#undef NRO_MAIN					/* extern globals */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termcap.h>
#include "nroff.h"
#pragma lint -1
#pragma optimize -1

#pragma noroot

/*------------------------------*/
/*	getlin			*/
/*------------------------------*/
int getlin (char *p, FILE *in_buf)
{

/*
 *	retrieve one line of input text
 */

	register char  *q;
	register int	i;
	int		c;
	int		nreg;

	q = p;
	for (i = 0; i < MAXLINE - 1; ++i)
	{
		c = ngetc (in_buf);
		if (c == EOF)
		{
			*q = EOS;
			c  = strlen (p);
			return (c == 0 ? EOF : c);
		}
		*q++ = c;
		if (c == '\n')
			break;
	}
	*q = EOS;

	nreg = findreg (".c");
	if (nreg > 0)
		set_ireg (".c", rg[nreg].rval + 1, 0);

	return (strlen (p));
}






/*------------------------------*/
/*	ngetc			*/
/*------------------------------*/
int ngetc (FILE *infp)
{

/*
 *	get character from input file or push back buffer
 */

	register int	c;

	if (mac.ppb >= &mac.pbb[0])
		c = *mac.ppb--;
	else
		c = getc (infp);

	return (c);
}



/*------------------------------*/
/*	pbstr			*/
/*------------------------------*/
int pbstr (char *p)
{

/*
 *	Push back string into input stream
 */

	register int	i;

	/*
	 *   if string is null, we do nothing
	 */
	if (p == NULL_CPTR)
		return;
	if (p[0] == EOS)
		return;
        for (i = strlen (p) - 1; i >= 0; --i)
	{
		putbak (p[i]);
	}
}




/*------------------------------*/
/*	putbak			*/
/*------------------------------*/
int putbak (char c)
{

/*
 *	Push character back into input stream. we use the push-back buffer
 *	stored with macros.
 */

	if (mac.ppb == NULL)
	{
		mac.ppb = &(mac.pbb[0]);
		*mac.ppb = c;
	}
	else
	{
                if (mac.ppb == mac.pbb + MAXPBB)
		{
			fprintf (err_stream,
				"***%s: push back buffer overflow\n", myname);
			err_exit (-1);
		}
		*++(mac.ppb) = c;
	}
}

FILE *fpGLOB;

int tprchar(char c)
{
    putc( c, fpGLOB );
}

/*------------------------------*/
/*	prchar			*/
/*------------------------------*/
int prchar (char c, FILE *fp)
{

/*
 *	print character with test for printer
 */

/* this really slows things down. it should be fixed. for now, ignore
   line printer...

	if (fp == stdout)
		putc (c, fp);
	else
		putc_lpr (c, fp);
*/
    switch (c) {
	case E_OFF:
        	fpGLOB = fp;
                tputs(e_bold,1,tprchar);
                tputs(e_italic,1,tprchar);
                break;
	case S_BOLD:
        	fpGLOB = fp; tputs(s_bold,1,tprchar); break;
	case E_BOLD:
        	fpGLOB = fp; tputs(e_bold,1,tprchar); break;
	case S_ITALIC:
        	fpGLOB = fp; tputs(s_italic,1,tprchar); break;
	case E_ITALIC:
        	fpGLOB = fp; tputs(e_italic,1,tprchar); break;
	case 13: break;
	default: putc (c, fp);
    }
}






/*------------------------------*/
/*	put			*/
/*------------------------------*/
int put (char *p)
{

/*
 *	put out line with proper spacing and indenting
 */

	register int	j;
	char		os[MAXLINE];

	if (pg.lineno == 0 || pg.lineno > pg.bottom)
	{
		phead ();
	}
	if (dc.prflg == TRUE)
	{
		if (!dc.bsflg)
		{
			if (strkovr (p, os) == TRUE)
			{
				for (j = 0; j < pg.offset; ++j)
					prchar (' ', out_stream);
				for (j = 0; j < dc.tival; ++j)
					prchar (' ', out_stream);
				putlin (os, out_stream);
			}
		}
		for (j = 0; j < pg.offset; ++j)
			prchar (' ', out_stream);
		for (j = 0; j < dc.tival; ++j)
			prchar (' ', out_stream);
		putlin (p, out_stream);
	}
	dc.tival = dc.inval;
	skip (min (dc.lsval - 1, pg.bottom - pg.lineno));
	pg.lineno = pg.lineno + dc.lsval;
	set_ireg ("ln", pg.lineno, 0);
	if (pg.lineno > pg.bottom)
	{
		pfoot ();
		if (stepping)
			wait_for_char ();
	}
}




/*------------------------------*/
/*	putlin			*/
/*------------------------------*/
int putlin (char *p, FILE *pbuf)
{

/*
 *	output a null terminated string to the file
 *	specified by pbuf.
 */

	while (*p != EOS)
		prchar (*p++, pbuf);
}




/*------------------------------*/
/*	putc_lpr		*/
/*------------------------------*/
#ifdef GEMDOS
#include <osbind.h>
#endif

int putc_lpr (char c, FILE *fp)
{

/*
 *	write char to printer
 */

#ifdef GEMDOS
	Bconout (0, (int) c & 0x00FF);
#else
	putc (c, fp);
#endif
}
