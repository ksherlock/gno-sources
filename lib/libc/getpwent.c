#pragma noroot
#pragma optimize -1
/*
 * Copyright (c) 1988 The Regents of the University of California.
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

#define _PATH_PASSWD "/etc/passwd"

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)getpwent.c	5.21 (Berkeley) 3/14/91";
#endif /* LIBC_SCCS and not lint */

#include <stdio.h>
#include <sys/types.h>
#include <sys/param.h>
#include <fcntl.h>
#include <sys/syslog.h>
#include <pwd.h>
#include <utmp.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

static struct passwd _pw_passwd;	/* password structure */
static char pwline[128];		/* line in /etc/passwd */
static int _pw_stayopen;		/* keep fd's open */

static int mode; /* 0 = closed, 1 = open */
static FILE *f_passwd;

static void parsepw(struct passwd *_pw, char *line)
{
char *foo;

	_pw->pw_name = line;
        foo = strchr(line,':');
        *foo++ = 0;
        _pw->pw_passwd = foo;

        foo = strchr(foo,':');
        *foo++ = 0;
        _pw->pw_uid = atoi(foo);
        foo = strchr(foo,':');
        *foo++ = 0;
        _pw->pw_gid = atoi(foo);
        foo = strchr(foo,':');
        foo++;
        _pw->pw_comment = foo;
        foo = strchr(foo,':');
        *foo++ = 0;
        _pw->pw_dir = foo;
        foo = strchr(foo,':');
        *foo++ = 0;
        _pw->pw_shell = foo;
        foo = strchr(foo,'\n');
        *foo = 0;
}

struct passwd *
getpwent()
{
char *foo;

    if (!mode) {
        f_passwd = fopen(_PATH_PASSWD,"r");
        mode = 1;
    }
    if (fgets(pwline, 127, f_passwd) == NULL) {
	fclose(f_passwd);
        mode = 0;
        return NULL;
    } else {
	parsepw(&_pw_passwd,pwline);
	return (&_pw_passwd);
    }
}

static char line[128];

struct passwd *
getpwnam(char *name)
{
FILE *ff_passwd;
unsigned leng;

    ff_passwd = fopen(_PATH_PASSWD,"r");
    while (fgets(line,127,ff_passwd) != NULL)
    {
    	leng = strlen(name);
	if (!strncmp(name,line,leng)) {
            if (strpos(line,':') != leng) continue;
            parsepw(&_pw_passwd,line);
            fclose(ff_passwd);
            return (&_pw_passwd);
        }
    }
    fclose(ff_passwd);
    return NULL;
}

struct passwd *
getpwuid(int uid)
{
FILE *ff_passwd;
char *foo;
int u;

    ff_passwd = fopen(_PATH_PASSWD,"r");
    while (fgets(line,127,ff_passwd) != NULL)
    {
	foo = strchr(line, ':');
        foo = strchr(foo+1,':');
	u = atoi(foo+1);
        if (u == uid) {
	    parsepw(&_pw_passwd,line);
            fclose(ff_passwd);
            return (&_pw_passwd);
        }
    }
    fclose(ff_passwd);
    return NULL;
}

#ifdef NOTDEFINED
int
setpassent(stayopen)
	int stayopen;
{
	_pw_keynum = 0;
	_pw_stayopen = stayopen;
	return(1);
}
#endif

int
setpwent(void)
{
    if (mode) { rewind(f_passwd); }
    return (1);
}

void
endpwent(void)
{
    if (mode) {
    	fclose(f_passwd);
        mode = 0;
    }
}
