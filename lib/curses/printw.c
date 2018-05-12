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
static char sccsid[] = "@(#)printw.c	5.11 (Berkeley) 8/31/92";
#endif	/* not lint */

#include <curses.h>
#include <stdlib.h>

#if __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#pragma noroot
#pragma lint -1
#pragma optimize 8

/*
 * printw and friends.
 *
 * These routines make nonportable assumptions about varargs if __STDC__
 * is not in effect.
 */

#ifdef __STDC__
static int __sprintw (WINDOW *, const char *, va_list);
static int __winwrite (void *, const char *, int);
#else
static int __sprintw __P((WINDOW *, const char *, va_list));
static int __winwrite __P((void *, const char *, int));
#endif

/*
 * printw --
 *	Printf on the standard screen.
 */
int
#if __STDC__
printw(const char *fmt, ...)
#else
printw(fmt, va_alist)
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
	ret = __sprintw(stdscr, fmt, ap);
	va_end(ap);
	return (ret);
}

/*
 * wprintw --
 *	Printf on the given window.
 */
int
#if __STDC__
wprintw(WINDOW * win, const char *fmt, ...)
#else
wprintw(win, fmt, va_alist)
	WINDOW *win;
	char *fmt;
	va_dcl
#endif
{
	va_list ap;
	int ret;

#ifdef __STDC__
	va_start(ap, fmt);
#else
	va_start(ap);
#endif
	ret = __sprintw(win, fmt, ap);
	va_end(ap);
	return (ret);
}

/*
 * mvprintw, mvwprintw --
 *	Implement the mvprintw commands.  Due to the variable number of
 *	arguments, they cannot be macros.  Sigh....
 */
int
#if __STDC__
mvprintw(register int y, register int x, const char *fmt, ...)
#else
mvprintw(y, x, fmt, va_alist)
	register int y, x;
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
	if (move(y, x) != OK)
		return (ERR);
	ret = __sprintw(stdscr, fmt, ap);
	va_end(ap);
	return (ret);
}

int
#if __STDC__
mvwprintw(register WINDOW * win, register int y, register int x,
    const char *fmt, ...)
#else
mvwprintw(win, y, x, fmt, va_alist)
	register WINDOW *win;
	register int y, x;
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
	if (wmove(win, y, x) != OK)
		return (ERR);

	ret = __sprintw(win, fmt, ap);
	va_end(ap);
	return (ret);
}

/*
 * Internal write-buffer-to-window function.
 */
static int
__winwrite(void *cookie, const char *buf, int n)
/*	void *cookie;
	register const char *buf;
	int n; */
{
	register WINDOW *win;
	register int c;

	for (c = n, win = cookie; --c >= 0;)
		if (waddch(win, *buf++) == ERR)
			return (-1);
	return (n);
}

/*
 * __sprintw --
 *	This routine actually executes the printf and adds it to the window.
 *	It must not be declared static as it is used in mvprintw.c.
 *	THIS SHOULD BE RENAMED vwprintw AND EXPORTED
 */
#if 0
static int
__sprintw(WINDOW *win, const char *fmt, va_list ap)
/*	WINDOW *win;
	const char *fmt;
	va_list ap; */
{
	FILE *f;

	if ((f = funopen(win, NULL, __winwrite, NULL, NULL)) == NULL)
		return (ERR);
	(void)vfprintf(f, fmt, ap);
	return (fclose(f) ? ERR : OK);
}
#else
static int
__sprintw(WINDOW *win, const char *fmt, va_list ap)
/*	WINDOW *win;
	const char *fmt;
	va_list ap; */
{
	char *str = malloc(2048);
        if (str == NULL) return ERR;
        (void)vsprintf(str,fmt,ap);
        waddstr(win,str);
        free(str);
        return(OK);
}
#endif
