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
static char sccsid[] = "@(#)scanw.c	5.10 (Berkeley) 8/31/92";
#endif	/* not lint */

/*
 * scanw and friends.
 */

#include <curses.h>

#if __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#pragma noroot
#pragma lint -1
#pragma optimize 8

#ifdef __STDC__
static int __sscans (WINDOW *, const char *, va_list);
#else
static int __sscans __P((WINDOW *, const char *, va_list));
#endif

/*
 * scanw --
 *	Implement a scanf on the standard screen.
 */
int
#if __STDC__
scanw(const char *fmt, ...)
#else
scanw(fmt, va_alist)
	char *fmt;
	va_dcl
#endif
{
	va_list ap;
	int ret;

#if __STDC__
	va_start(ap, fmt);
#else
	va_start(ap);
#endif
	ret = __sscans(stdscr, fmt, ap);
	va_end(ap);
	return (ret);
}

/*
 * wscanw --
 *	Implements a scanf on the given window.
 */
int
#if __STDC__
wscanw(WINDOW *win, const char *fmt, ...)
#else
wscanw(win, fmt, va_alist)
	WINDOW *win;
	char *fmt;
	va_dcl
#endif
{
	va_list ap;
	int ret;

#if __STDC__
	va_start(ap, fmt);
#else
	va_start(ap);
#endif
	ret = __sscans(win, fmt, ap);
	va_end(ap);
	return (ret);
}

/*
 * mvscanw, mvwscanw -- 
 *	Implement the mvscanw commands.  Due to the variable number of
 *	arguments, they cannot be macros.  Another sigh....
 */
int
#if __STDC__
mvscanw(register int y, register int x, const char *fmt,...)
#else
mvscanw(y, x, fmt, va_alist)
	register int y, x;
	char *fmt;
	va_dcl
#endif
{
	va_list ap;
	int ret;

	if (move(y, x) != OK)
		return (ERR);
#if __STDC__
	va_start(ap, fmt);
#else
	va_start(ap);
#endif
	ret = __sscans(stdscr, fmt, ap);
	va_end(ap);
	return (ret);
}

int
#if __STDC__
mvwscanw(register WINDOW * win, register int y, register int x,
    const char *fmt, ...)
#else
mvwscanw(win, y, x, fmt, va_alist)
	register WINDOW *win;
	register int y, x;
	char *fmt;
	va_dcl
#endif
{
	va_list ap;
	int ret;

	if (move(y, x) != OK)
		return (ERR);
#if __STDC__
	va_start(ap, fmt);
#else
	va_start(ap);
#endif
	ret = __sscans(win, fmt, ap);
	va_end(ap);
	return (ret);
}

/*
 * __sscans --
 *	This routine actually executes the scanf from the window.
 *	THIS SHOULD BE RENAMED vwscanw AND EXPORTED
 */
#pragma lint 0
#include <misctool.h>
#ifdef __ORCAC__
static int
__sscans(WINDOW *win, const char *fmt, va_list ap)
{
    SysBeep();
}

#else
static int
__sscans(WINDOW *win, const char *fmt, va_list ap)
/*	WINDOW *win;
	const char *fmt;
	va_list ap; */
{

	char buf[1024];

	return (wgetstr(win, buf) == OK ? vsscanf(buf, fmt, ap) : ERR);
}
#endif
