#pragma optimize -1
#pragma noroot

/*

BSD compatibility by Derek Taubert

6/17/92	(jb)
	Removed stuff for compatibility with ORCA (who cares)
        Made many routines reentrant
4/2/94 (jb)
	Changed references to _toolErr to toolerror()

*/
segment "lbsd      ";

#include <GSOS.h>
#include <shell.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

extern GSString255Ptr __C2GSMALLOC(char *);

#define SetVarGS(parm)            (PDosInt(0x0146,parm))
typedef struct {
   unsigned pCount;
   GSString255Ptr var_name;             /* variable name */
   GSString255Ptr value;                /* variable value */
   unsigned export;
   } ReadVariableGSPB, SetVariableGSPB;

int chdir(const char *pathname)
{
PrefixRecGS prefx;
GSString255Ptr p;
int e;
int l;

    p = __C2GSMALLOC(pathname);

    prefx.pCount = 2;
    prefx.prefixNum = 0;
    prefx.buffer.setPrefix = p;
    SetPrefixGS(&prefx);
    if (toolerror()) {
        errno = _mapErr(toolerror());
        free(p);
        return(-1);
    }
    free(p);
    return(0);
}

char *getwd(char *pathname)
{
PrefixRecGS prefx;
int d[] = {4,0,0};
ResultBuf255Ptr where;
int e;

    prefx.pCount = 2;
    prefx.prefixNum = 0;
    prefx.buffer.getPrefix = (ResultBuf255Ptr) d;
    GetPrefixGS(&prefx);
    where = malloc(d[1]+8);
    where->bufSize = d[1]+8;
    prefx.buffer.getPrefix = where;
    GetPrefixGS(&prefx);
    if (toolerror()) {
        errno = _mapErr(toolerror());
        free(where);
        return NULL;
    }

    /* if last character is a ':', remove it for Unix compatibility */
    e = where->bufString.length;
    if (where->bufString.text[e] == ':')
        where->bufString.length--;
    __GS2C(pathname,&where->bufString);
    free(where);
    return(pathname);
}

/*
 * setenv --
 *	Set the value of the environmental variable "name" to be
 *	"value".  If rewrite is set, replace any current value.
 */
int setenv(name, value, rewrite)
	register const char *name;
	register const char *value;
	int rewrite;
{
GSString255Ptr a, b;
int l;
SetVariableGSPB s;

	a = __C2GSMALLOC(name);
        b = __C2GSMALLOC(value);
        s.pCount = 3;
        s.var_name = a;
        s.value = b;
        s.export = 1;
        SetVarGS(&s);
        free(a); free(b);
        return (0);
}

/*
 * unsetenv(name) --
 *	Delete environmental variable "name".
 */
void
unsetenv(name)
	const char	*name;
{
Unset_VariablePB u;
int l; char *a;

	l = strlen(name);
        a = malloc(l+1);
        a[0] = l; memcpy(a+1,name,l);
	u.name = a;
        UNSET_VARIABLE(&u);
}

#include <sys/cdefs.h>
#include <stdio.h>

/*
 * Get next token from string *stringp, where tokens are nonempty
 * strings separated by characters from delim.  
 *
 * Writes NULs into the string at *stringp to end tokens.
 * delim need not remain constant from call to call.
 * On return, *stringp points past the last NUL written (if there might
 * be further tokens), or is NULL (if there are definitely no more tokens).
 *
 * If *stringp is NULL, strtoken returns NULL.
 */
char *
strsep(stringp, delim)
	register char **stringp;
	register const char *delim;
{
	register char *s;
	register const char *spanp;
	register int c, sc;
	char *tok;

	if ((s = *stringp) == NULL)
		return (NULL);
	for (tok = s;;) {
		c = *s++;
		spanp = delim;
		do {
			if ((sc = *spanp++) == c) {
				if (c == 0)
					s = NULL;
				else
					s[-1] = 0;
				*stringp = s;
				return (tok);
			}
		} while (sc != 0);
	}
	/* NOTREACHED */
}

char *index(char *a, int b)
{
    return strchr(a,b);
}

char *rindex(char *a, int b)
{
    return strrchr(a,b);
}

char *mktemp(char *s)
{
int l;
int c = 'A';

    l = strlen(s);
    sprintf(s+l-5,"%05d",getpid());
    do {
    	*(s+l-6) = c;
    } while (access(s,0) == 0);
    return s;
}

int mkstemp(char *s)
{
char *x = mktemp(s);     
    return open(s,O_RDWR | O_CREAT,0666);
}

char *ttyname(int fino)
{
static ResultBuf32 resBuf;
static RefInfoRecGS refInfo = {3,1,0,(ResultBuf255Ptr)&resBuf};

    if (!isatty(fino)) return NULL;
    resBuf.bufSize = 32;
    refInfo.refNum = fino;
    GetRefInfoGS(&refInfo);
    refInfo.pathname->bufString.text[refInfo.pathname->bufString.length] = 0;
    return refInfo.pathname->bufString.text;
}
