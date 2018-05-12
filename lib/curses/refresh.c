/*
 * Copyright (c) 1981 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
static char sccsid[] = "@(#)refresh.c	5.8 (Berkeley) 8/31/92";
#endif /* not lint */

#include <curses.h>
#include <string.h>
#pragma noroot
#pragma lint -1
#pragma optimize 8

static int curwin;
static short ly, lx;

WINDOW *_win;

#ifdef __STDC__
static void	domvcur (int, int, int, int);
static int	makech (WINDOW *, int);
#else
static void	domvcur __P((int, int, int, int));
static int	makech __P((WINDOW *, int));
#endif
/*
 * wrefresh --
 *	Make the current screen look like "win" over the area coverd by
 *	win.
 */
int
wrefresh(WINDOW *win)
/*	register WINDOW *win; */
{
	register int retval;
	register short wy;

	/* Make sure were in visual state. */
	if (__endwin) {
		tputs(VS, 0, __cputchar);
		tputs(TI, 0, __cputchar);
		__endwin = 0;
	}

	/* Initialize loop parameters. */

	ly = curscr->_cury;
	lx = curscr->_curx;
	wy = 0;
	_win = win;
	curwin = (win == curscr);

	if (win->_clear || curscr->_clear || curwin) {
		if ((win->_flags & _FULLWIN) || curscr->_clear) {
			tputs(CL, 0, __cputchar);
			ly = 0;
			lx = 0;
			if (!curwin) {
				curscr->_clear = 0;
				curscr->_cury = 0;
				curscr->_curx = 0;
				werase(curscr);
			}
			touchwin(win);
		}
		win->_clear = 0;
	}
	if (!CA) {
		if (win->_curx != 0)
			putchar('\n');
		if (!curwin)
			werase(curscr);
	}
#ifdef DEBUG
	__TRACE("wrefresh: (%0.2o): curwin = %d\n", win, curwin);
	__TRACE("wrefresh: \tfirstch\tlastch\n");
#endif
	for (wy = 0; wy < win->_maxy; wy++) {
#ifdef DEBUG
		__TRACE("%d\t%d\t%d\n",
		    wy, win->_firstch[wy], win->_lastch[wy]);
#endif
		if (win->_firstch[wy] != _NOCHANGE)
			if (makech(win, wy) == ERR)
				return (ERR);
			else {
				if (win->_firstch[wy] >= win->_ch_off)
					win->_firstch[wy] = win->_maxx +
					    win->_ch_off;
				if (win->_lastch[wy] < win->_maxx +
				    win->_ch_off)
					win->_lastch[wy] = win->_ch_off;
				if (win->_lastch[wy] < win->_firstch[wy])
					win->_firstch[wy] = _NOCHANGE;
			}
#ifdef DEBUG
		__TRACE("\t%d\t%d\n", win->_firstch[wy], win->_lastch[wy]);
#endif
	}

	if (win == curscr)
		domvcur(ly, lx, win->_cury, win->_curx);
	else {
		if (win->_leave) {
			curscr->_cury = ly;
			curscr->_curx = lx;
			ly -= win->_begy;
			lx -= win->_begx;
			if (ly >= 0 && ly < win->_maxy && lx >= 0 &&
			    lx < win->_maxx) {
				win->_cury = ly;
				win->_curx = lx;
			} else
				win->_cury = win->_curx = 0;
		} else {
			domvcur(ly, lx, win->_cury + win->_begy,
			    win->_curx + win->_begx);
			curscr->_cury = win->_cury + win->_begy;
			curscr->_curx = win->_curx + win->_begx;
		}
	}
	retval = OK;

	_win = NULL;
	(void)fflush(stdout);
	return (retval);
}

/*
 * makech --
 *	Make a change on the screen.
 */
static int
makech(WINDOW *win, int wy)
/*	register WINDOW *win;
	int wy; */
{
	register int nlsp, clsp;		/* Last space in lines. */
	register short wx, lch, y;
	register char *nsp, *csp, *ce;

	wx = win->_firstch[wy] - win->_ch_off;
	if (wx >= win->_maxx)
		return (OK);
	else if (wx < 0)
		wx = 0;
	lch = win->_lastch[wy] - win->_ch_off;
	if (lch < 0)
		return (OK);
	else if (lch >= win->_maxx)
		lch = win->_maxx - 1;;
	y = wy + win->_begy;

	if (curwin)
		csp = " ";
	else
		csp = &curscr->_y[wy + win->_begy][wx + win->_begx];

	nsp = &win->_y[wy][wx];
	if (CE && !curwin) {
		for (ce = &win->_y[wy][win->_maxx - 1]; *ce == ' '; ce--)
			if (ce <= win->_y[wy])
				break;
		nlsp = ce - win->_y[wy];
	}
	if (!curwin)
		ce = CE;
	else
		ce = NULL;

	while (wx <= lch) {
		if (*nsp == *csp) {
			if (wx <= lch) {
				while (*nsp == *csp && wx <= lch) {
					nsp++;
					if (!curwin)
						csp++;
					++wx;
				}
				continue;
			}
			break;
		}
		domvcur(ly, lx, y, wx + win->_begx);
#ifdef DEBUG
		__TRACE("makech: 1: wx = %d, lx = %d, newy = %d, newx = %d\n", 
		    wx, lx, y, wx + win->_begx);
#endif
		ly = y;
		lx = wx + win->_begx;
		while (*nsp != *csp && wx <= lch) {
			if (ce != NULL && wx >= nlsp && *nsp == ' ') {
				/* Check for clear to end-of-line. */
				ce = &curscr->_y[ly][COLS - 1];
				while (*ce == ' ')
					if (ce-- <= csp)
						break;
				clsp = ce - curscr->_y[ly] - win->_begx;
#ifdef DEBUG
			__TRACE("makech: clsp = %d, nlsp = %d\n", clsp, nlsp);
#endif
				if (clsp - nlsp >= strlen(CE) &&
				    clsp < win->_maxx) {
#ifdef DEBUG
					__TRACE("makech: using CE\n");
#endif
					tputs(CE, 0, __cputchar);
					lx = wx + win->_begx;
					while (wx++ <= clsp)
						*csp++ = ' ';
					return (OK);
				}
				ce = NULL;
			}

			/* Enter/exit standout mode as appropriate. */
			if (SO && (*nsp & _STANDOUT) !=
			    (curscr->_flags & _STANDOUT)) {
				if (*nsp & _STANDOUT) {
					tputs(SO, 0, __cputchar);
					curscr->_flags |= _STANDOUT;
				} else {
					tputs(SE, 0, __cputchar);
					curscr->_flags &= ~_STANDOUT;
				}
			}

			wx++;
			if (wx >= win->_maxx && wy == win->_maxy - 1)
				if (win->_scroll) {
					if (curscr->_flags & _STANDOUT
					    && win->_flags & _ENDLINE)
						if (!MS) {
							tputs(SE, 0,
							    __cputchar);
							curscr->_flags &=
							    ~_STANDOUT;
						}
					if (!curwin)
						putchar((*csp = *nsp) & 0177);
					else
						putchar(*nsp & 0177);
					if (win->_flags & _FULLWIN && !curwin)
						scroll(curscr);
					ly = win->_begy + win->_cury;
					lx = win->_begx + win->_curx;
					return (OK);
				} else
					if (win->_flags & _SCROLLWIN) {
						lx = --wx;
						return (ERR);
					}
			if (!curwin)
				putchar((*csp++ = *nsp) & 0177);
			else
				putchar(*nsp & 0177);
#ifdef DEBUG
			__TRACE("makech: putchar(%c)\n", *nsp & 0177);
#endif
			if (UC && (*nsp & _STANDOUT)) {
				putchar('\b');
				tputs(UC, 0, __cputchar);
			}
			nsp++;
		}
#ifdef DEBUG
		__TRACE("makech: 2: wx = %d, lx = %d\n", wx, lx);
#endif
		if (lx == wx + win->_begx)	/* If no change. */
			break;
		lx = wx + win->_begx;
		if (lx >= COLS && AM) {
			lx = 0;
			ly++;
			/*
			 * xn glitch: chomps a newline after auto-wrap.
			 * we just feed it now and forget about it.
			 */
			if (XN) {
				putchar('\n');
				putchar('\r');
			}
		}
#ifdef DEBUG
		__TRACE("makech: 3: wx = %d, lx = %d\n", wx, lx);
#endif
	}
	return (OK);
}

/*
 * domvcur --
 *	Do a mvcur, leaving standout mode if necessary.
 */
static void
domvcur(int oy, int ox, int ny, int nx)
/*	int oy, ox, ny, nx; */
{
	if (curscr->_flags & _STANDOUT && !MS) {
		tputs(SE, 0, __cputchar);
		curscr->_flags &= ~_STANDOUT;
	}
	mvcur(oy, ox, ny, nx);
}
