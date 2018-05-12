#undef OLD_WAY
/*
 *	text.c - text output processing portion of nroff word processor
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
#include "nroff.h"
#pragma lint -1
#include <string.h>
#include <stdlib.h>
#pragma optimize -1
#pragma noroot

/*------------------------------*/
/*	text			*/
/*------------------------------*/
int text (char *p)
{

/*
 *	main text processing
 */

	register int	i;
	char		wrdbuf[MAXLINE];


	/*
	 *   skip over leading blanks if in fill mode. we indent later.
	 *   since leadbl does a robrk, do it if in .nf mode
	 */
	if (dc.fill == YES)
	{
		if (*p == ' ' || *p == '\n' || *p == '\r')
			leadbl (p);
	}
	else
		robrk ();


	/*
	 *   expand escape sequences
	 */
	expesc (p, wrdbuf);


	/*
	 *   test for how to output
	 */
	if (dc.ulval > 0)
	{
		/*
		 *   underline (.ul)
		 *
		 *   Because of the way underlining is handled,
		 *   MAXLINE should be declared to be three times
		 *   larger than the longest expected input line
		 *   for underlining.  Since many of the character
		 *   buffers use this parameter, a lot of memory
		 *   can be allocated when it may not really be
		 *   needed.  A MAXLINE of 180 would allow about
		 *   60 characters in the output line to be
		 *   underlined (remember that only alphanumerics
		 *   get underlined - no spaces or punctuation).
		 */
		underl (p, wrdbuf, MAXLINE);
		--dc.ulval;
	}
	if (dc.cuval > 0)
	{
		/*
		 *   continuous underline (.cu)
		 */
		underl (p, wrdbuf, MAXLINE);
		--dc.cuval;
	}
	if (dc.boval > 0)
	{
		/*
		 *   bold (.bo)
		 */
		bold (p, wrdbuf, MAXLINE);
		--dc.boval;
	}
	if (dc.ceval > 0)
	{
		/*
		 *   centered (.ce)
		 */
		center (p);
		do_mc (p);
		put (p);
		--dc.ceval;
	}
	else if ((*p == '\r' || *p == '\n') && dc.fill == NO)
	{
		/*
		 *   all blank line
		 */
		do_mc (p);
		put (p);
	}
	else if (dc.fill == NO)
	{
		/*
		 *   unfilled (.nf)
		 */
		do_mc (p);
		put (p);
	}
	else
	{
		/*
		 *   anything else...
		 *
		 *   init escape char counter for this line...
		 */
/*		co.outesc = 0;*/


		/*
		 *   get a word and put it out. increment ptr to the next
		 *   word.
		 */
		while ((i = getwrd (p, wrdbuf)) > 0)
		{
/*			co.outesc += countesc (wrdbuf);*/

			putwrd (wrdbuf);
			p += i;
		}
	}
}




/*------------------------------*/
/*	bold			*/
/*------------------------------*/
int bold (char *p0, char *p1, int size)
/*register char  *p0;
register char  *p1;
int		size; */
{

/*
 *	insert bold face text (by overstriking)
 */

	register int	i;
	register int	j;

	j = 0;
	for (i = 0; (p0[i] != '\n') && (j < size - 1); ++i)
	{
		if (isalpha (p0[i]) || isdigit (p0[i]))
		{
			p1[j++] = p0[i];
			p1[j++] = '\b';
		}
		p1[j++] = p0[i];
	}
	p1[j++] = '\n';
	p1[j] = EOS;
	while (*p1 != EOS)
		*p0++ = *p1++;
	*p0 = EOS;
}






/*------------------------------*/
/*	center			*/
/*------------------------------*/
int center (char *p)
/*register char  *p;*/
{

/*
 *	center a line by setting tival
 */

	dc.tival = max ((dc.rmval + dc.tival - width (p)) >> 1, 0);
}




/*------------------------------*/
/*	expand			*/
/*------------------------------*/
int expand (char *p0, char c, char *s)
/*register char  *p0;
char		c;
register char  *s; */
{

/*
 *	expand title buffer to include character string
 */

	register char  *p;
	register char  *q;
	register char  *r;
	char    	tmp[MAXLINE];

	p = p0;
	q = tmp;
	while (*p != EOS)
	{
		if (*p == c)
		{
			r = s;
			while (*r != EOS)
				*q++ = *r++;
		}
		else
			*q++ = *p;
		++p;
	}
	*q = EOS;
	strcpy (p0, tmp);		/* copy it back */
}




/*------------------------------*/
/*	justcntr		*/
/*------------------------------*/
int justcntr (char *p, char *q, int *limit)
/*register char  *p;
char	       *q;
int	       *limit;*/
{

/*
 *	center title text into print buffer
 */

	register int	len;

	len = width (p);
	q   = &q[(limit[RIGHT] + limit[LEFT] - len) >> 1];
	while (*p != EOS)
		*q++ = *p++;
}





/*------------------------------*/
/*	justleft		*/
/*------------------------------*/
int justleft (char *p, char *q, int limit)
/*register char  *p;
char	       *q;
int		limit; */
{

/*
 *	left justify title text into print buffer
 */

	q = &q[limit];
	while (*p != EOS)
		*q++ = *p++;
}




/*------------------------------*/
/*	justrite		*/
/*------------------------------*/
int justrite (char *p, char *q, int limit)
/*register char  *p;
char	       *q;
int     	limit; */
{

/*
 *	right justify title text into print buffer
 */

	register int	len;

	len = width (p);
	q = &q[limit - len];
	while (*p != EOS)
		*q++ = *p++;
}






/*------------------------------*/
/*	leadbl			*/
/*------------------------------*/
int leadbl (char *p)
/*register char  *p;*/
{

/*
 *	delete leading blanks, set tival
 */

	register int	i;
	register int	j;

	/*
	 *   end current line and reset co struct
	 */
	robrk ();

	/*
	 *   skip spaces
	 */
	for (i = 0; p[i] == ' ' || p[i] == '\t'; ++i)
		;

	/*
	 *   if not end of line, reset current temp indent
	 */
	if (p[i] != '\n' && p[i] != '\r')
		dc.tival = i;

	/*
	 *   shift string
	 */
	for (j = 0; p[i] != EOS; ++j)
		p[j] = p[i++];
	p[j] = EOS;
}





/*------------------------------*/
/*	pfoot			*/
/*------------------------------*/
int pfoot (void)
{

/*
 *	put out page footer
 */

	if (dc.prflg == TRUE)
	{
		skip (pg.m3val);
		if (pg.m4val > 0)
		{
			if ((pg.curpag % 2) == 0)
			{
				puttl (pg.efoot, pg.eflim, pg.curpag);
			}
			else
			{
				puttl (pg.ofoot, pg.oflim, pg.curpag);
			}
			skip (pg.m4val - 1);
		}
	}
}





/*------------------------------*/
/*	phead			*/
/*------------------------------*/
int phead (void)
{

/*
 *	put out page header
 */

	pg.curpag = pg.newpag;
	if (pg.curpag >= pg.frstpg && pg.curpag <= pg.lastpg)
	{
		dc.prflg = TRUE;
	}
	else
	{
		dc.prflg = FALSE;
	}
	++pg.newpag;
	set_ireg ("%", pg.newpag, 0);
	if (dc.prflg == TRUE)
	{
		if (pg.m1val > 0)
		{
			skip (pg.m1val - 1);
			if ((pg.curpag % 2) == 0)
			{
				puttl (pg.ehead, pg.ehlim, pg.curpag);
			}
			else
			{
				puttl (pg.ohead, pg.ohlim, pg.curpag);
			}
		}
		skip (pg.m2val);
	}
	/* 
	 *	initialize lineno for the next page
	 */
	pg.lineno = pg.m1val + pg.m2val + 1;
	set_ireg ("ln", pg.lineno, 0);
}




/*------------------------------*/
/*	puttl			*/
/*------------------------------*/
int puttl (char *p, int *lim, int pgno)
/*register char  *p;
int	       *lim;
int		pgno;*/
{

/*
 *	put out title or footer
 */

	register int	i;
	char		pn[8];
	char		t[MAXLINE];
	char		h[MAXLINE];
	char		delim;

	itoda (pgno, pn, 6);
	for (i = 0; i < MAXLINE; ++i)
		h[i] = ' ';
	delim = *p++;
	p = getfield (p, t, delim);
	expand (t, dc.pgchr, pn);
	justleft (t, h, lim[LEFT]);
	p = getfield (p, t, delim);
	expand (t, dc.pgchr, pn);
	justcntr (t, h, lim);
	p = getfield (p, t, delim);
	expand (t, dc.pgchr, pn);
	justrite (t, h, lim[RIGHT]);
	for (i = MAXLINE - 4; h[i] == ' '; --i)
		h[i] = EOS;
	h[++i] = '\n';
        h[++i] = '\r';
	h[++i] = EOS;
	if (strlen (h) > 2)
	{
		for (i = 0; i < pg.offset; ++i)
			prchar (' ', out_stream);
	}
	putlin (h, out_stream);
}





/*------------------------------*/
/*	putwrd			*/
/*------------------------------*/
int putwrd (char *wrdbuf)
/*register char  *wrdbuf;*/
{

/*
 *	put word in output buffer
 */

	register char  *p0;
	register char  *p1;
	int     	w;
	int     	last;
	int     	llval;
	int     	nextra;



	/*
	 *   check if this word puts us over the limit
	 */
	w     = width (wrdbuf);
	last  = strlen (wrdbuf) + co.outp;
	llval = dc.rmval - dc.tival;
/*	if (((co.outp > 0) && ((co.outw + w) > llval))*/
	co.outesc += countesc (wrdbuf);
	if (((co.outp > 0) && ((co.outw + w - co.outesc) > llval))
	||   (last > MAXLINE))
	{
		/*
		 *   last word exceeds limit so prepare to break line, print
		 *   it, and reset outbuf.
		 */
		last -= co.outp;
		if (dc.juval == YES)
		{
			nextra = llval - co.outw + 1;

			/*
			 *      Do not take in the escape char of the
			 *      word that didn't fit on this line anymore
			 */
			 co.outesc -= countesc (wrdbuf);

			/* 
			 *	Check whether last word was end of
			 *	sentence and modify counts so that
			 *	it is right justified.
			 */
			if (co.outbuf[co.outp - 2] == ' ')
			{
				--co.outp;
				++nextra;
			}
#ifdef OLD_WAY
			spread (co.outbuf, co.outp - 1, nextra, co.outwds, co.outesc);
			if ((nextra > 0) && (co.outwds > 1))
			{
				co.outp += (nextra - 1);
			}
/*			if (co.outesc > 0)
			{
				co.outp += co.outesc;
			}
*/
#else
			spread (co.outbuf, co.outp - 1, nextra, 
						co.outwds, co.outesc);
			if ((nextra + co.outesc > 0) && (co.outwds > 1))
			{
				co.outp += (nextra + co.outesc - 1);
			}
#endif
		}

		/*
		 *   break line, output it, and reset all co members. reset
		 *   esc count.
		 */
		robrk ();

		co.outesc = countesc (wrdbuf);
	}


	/*
	 *   copy the current word to the out buffer which may have been
	 *   reset
	 */
	p0 = wrdbuf;
	p1 = co.outbuf + co.outp;
	while (*p0 != EOS)
		*p1++ = *p0++;

	co.outp              = last;
	co.outbuf[co.outp++] = ' ';
	co.outw             += w + 1;
	co.outwds           += 1;
}




/*------------------------------*/
/*	skip			*/
/*------------------------------*/
int skip (int n)
/*register int	n; */
{

/*
 *	skips the number of lines specified by n.
 */

	register int	i;
	register int	j;


	if (dc.prflg == TRUE && n > 0)
	{
		for (i = 0; i < n; ++i)
		{
			/*
			 *   handle blank line with changebar
			 */
			if (mc_ing == TRUE)
			{
				for (j = 0; j < pg.offset; ++j)
					prchar (' ', out_stream);
				for (j = 0; j < dc.rmval; ++j)
					prchar (' ', out_stream);
				for (j = 0; j < mc_space; j++)
					prchar (' ', out_stream);
				prchar (mc_char, out_stream);
			}
			prchar ('\n', out_stream);
		        prchar ('\r', out_stream);
		}
	}
}





/*------------------------------*/
/*	spread			*/
/*------------------------------*/
int spread (char *p, int outp, int nextra, int outwds, int escapes)
/*register char  *p;
int     	outp;
int		nextra;
int		outwds;
int		escapes; */
{

/*
 *	spread words to justify right margin
 */

	register int	i;
	register int	j;
	register int	nb;
	register int	ne;
	register int	nholes;
	int		jmin;


	/*
	 *   quick sanity check...
	 */
#ifdef OLDWAY
	if ((nextra <= 0) || (outwds <= 1))
		return;
#else
	if ((nextra + escapes < 1) || (outwds < 2))
		return;
#endif


/*fflush (out_stream); fprintf (err_stream, "in spread: escapes = %d\n", escapes); fflush (err_stream);*/


	/*
	 *   set up for the spread and do it...
	 */
	dc.sprdir = ~dc.sprdir;
#ifdef OLD_WAY
	ne        = nextra;
#else
	ne        = nextra + escapes;
#endif
	nholes    = outwds - 1;			/* holes between words */
	i         = outp - 1;			/* last non-blank character */
	j         = min (MAXLINE - 3, i + ne);	/* leave room for CR,LF,EOS */
/*
	j        += escapes;
	if (p[i-1] == 27)
		j += 2;
	j = min (j, MAXLINE - 3);
*/
	while (i < j)
	{
		p[j] = p[i];
		if (p[i] == ' ')
		{
			if (dc.sprdir == 0)
				nb = (ne - 1) / nholes + 1;
			else
				nb = ne / nholes;
			ne -= nb;
			--nholes;
			for (; nb > 0; --nb)
			{
				--j;
				p[j] = ' ';
			}
		}
		--i;
		--j;
	}
}





/*------------------------------*/
/*	strkovr			*/
/*------------------------------*/
int strkovr (char *p, char *q)
/*register char  *p;
register char  *q; */
{

/*
 *	split overstrikes (backspaces) into seperate buffer
 */

	register char  *pp;
	int		bsflg;

	bsflg = FALSE;
	pp = p;
	while (*p != EOS)
	{
		*q = ' ';
		*pp = *p;
		++p;
		if (*p == '\b')
		{
			if (*pp >= ' ' && *pp <= '~')
			{
				bsflg = TRUE;
				*q = *pp;
				++p;
				*pp = *p;
				++p;
			}
		}
		++q;
		++pp;
	}
	*q++ = '\r';
	*q = *pp = EOS;

	return (bsflg);
}





/*------------------------------*/
/*	underl			*/
/*------------------------------*/
int underl (char *p0, char *p1, int size)
/*register char  *p0;
register char  *p1;
int		size; */
{

/*
 *	underline a line
 */

	register int	i;
	register int	j;

	j = 0;
	for (i = 0; (p0[i] != '\n') && (j < size - 1); ++i)
	{
		if (p0[i] >= ' ' && p0[i] <= '~')
		{
			if (isalpha (p0[i]) || isdigit (p0[i]) || dc.cuval > 0)
			{
				p1[j++] = '_';
				p1[j++] = '\b';
			}
		}
		p1[j++] = p0[i];
	}
	p1[j++] = '\n';
	p1[j] = EOS;
	while (*p1 != EOS)
		*p0++ = *p1++;
	*p0 = EOS;
}




/*------------------------------*/
/*	width			*/
/*------------------------------*/
int width (char *s)
{

/*
 *	compute width of character string
 */

	register int	w;

	w = 0;
	while (*s != EOS)
	{
		if (*s == '\b')
			--w;
		else if ((*s >= 32) && (*s < 128))
                /* *s != '\n' && *s != '\r')  */
			++w;
		++s;
	}

	return (w);
}




/*------------------------------*/
/*	do_mc			*/
/*------------------------------*/
int do_mc (char *p)
/*char   *p;*/
{

/*
 *	add margin char (change bar) for .nf and .ce lines.
 *
 *	filled lines handled in robrk(). blank lines (.sp) handled in skip().
 *	note: robrk() calls this routine, too.
 */

	register char  *ps;
	register int	nspaces;
	register int	i;
	register int	has_cr;
	register int	has_lf;
	int		len;
	int		nesc;


	if (mc_ing == FALSE)
		return;


	len = strlen (p);


	/*
	 *   get to the end...
	 */
	ps = p;
	while (*ps)
		ps++;


	/*
	 *   check for cr and lf
	 */
	ps--;
	has_lf = 0;
	has_cr = 0;
	while (ps >= p && (*ps == '\r' || *ps == '\n'))
	{
		if (*ps == '\n')
			has_lf++;
		else
			has_cr++;

		len--;
		ps--;
	}
	if (has_lf < has_cr)
		has_lf = has_cr;
	else if (has_cr < has_lf)
		has_cr = has_lf;


	/*
	 *   remove any trailing blanks here
	 */
	while (ps >= p && *ps == ' ')
	{
		ps--;
		len--;
	}
	*++ps = EOS;


	/*
	 *   add trailing spaces for short lines. count escapes, subtract
	 *   from len. use rmval for rigth margin (minus tival which is
	 *   added later in put).
	 */
	nesc    = countesc (p);
	len    -= nesc;
	nspaces = dc.rmval - dc.tival - len;
	for (i = 0; i < nspaces; i++, ps++)
		*ps = ' ';


	/*
	 *   add the bar...
	 */
	for (i = 0; i < mc_space; i++, ps++)
		*ps = ' ';
	*ps++ = mc_char;


	/*
	 *   replace cr, lf, and EOS
	 */
	while (has_lf--)
	{
	       	*ps++ = '\r';
		*ps++ = '\n';
	}
	*ps = EOS;


	return;
}
