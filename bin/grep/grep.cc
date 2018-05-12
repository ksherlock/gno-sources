/*

        Grep 1.22
         part of ShellStuff, a GNU-like project devoted to enhancing
         the Apple IIgs programmer's environment.
         This program and it's source is copyrighted 1991 by
         Procyon Software.  Permission granted to distribute freely,
         so long as there is no charge and this notice remains intact.
         ShellStuff is distributed under the GNU general public liscence.

*/

/* Changes:
   2-24-92  Changed error messages to use argv[0], stderr. Also, put in
            error checking for non-existent files!
   1-17-92  Added '_INITGNOSTDIO' call for new compatibility and speed
   9/6/1991 Removed init_wildcard code
   5/6/1991 fixed problem with pipe mode not quitting  (jb)

*/

/* only use Orca/C 1.2 with optimizations on */
#pragma optimize 8
#pragma stacksize 2048

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regexp.h>
#include <gsos.h>
#include <shell.h>
#include <ctype.h>
#include <errno.h>

#ifdef ERRAVAIL
char *progname;
extern char *mkprogname();
#endif

#ifdef DEBUG
extern int regnarrate;
void regdump(regexp *r);
#endif

char buf[BUFSIZ];

int errreport = 0;              /* Report errors via errseen? */
char *errseen = NULL;           /* Error message. */
int status = 0;                 /* Exit status. */

#ifdef EGREP
void multiple(void);
void complain(char *s1, char *s2);
void try(char **fields);
void error(char *s1, char *s2);
void regerror(char *s);

void
regerror(char *s)
{
        if (errreport)
                errseen = s;
        else
                error(s, "");
}

#ifndef ERRAVAIL
void error(char *s1, char *s2)
{
        fprintf(stderr, "regexp: ");
        fprintf(stderr, s1, s2);
        fprintf(stderr, "\n");
        exit(1);
}
#endif

regexp badregexp;               /* Implicit init to 0. */

#endif /* EGREP */

int lineno;

void PrintFileName(char *s)
{
char *r;
    if ((r = strrchr(s,'/')) != NULL)
        printf("%s:",r+1);
    else printf("%s:",s);
}

void convLower(char *s)
{
    while (*s) {
        if (isupper(*s)) *s = tolower(*s);
        s++;
    }
}

int main(int argc, char **argv)
{
extern int optind;
extern char *optarg;
extern int getopt(int,char **,char*);
#ifdef GREP
extern int REGEXP(word flag, char *text, char *pattern);
#endif

#ifdef EGREP
regexp *r;
#endif
FILE *inp;
char buf[255];
char lbuf[255];
int line = 1;
int cflag, hflag, lflag, nflag, vflag, sflag, eflag,iflag;
char *exp,*exp2;
char *file,*prog;
int ch,stdinput,multfiles;
char *truncated;
char nlTab[] = {13};
int result,done,matches = 0;
IORecGS rd;
NewlineRecGS nl;
extern int _INITGNOSTDIO(void);

    if (!_INITGNOSTDIO()) {
        fprintf(stderr,"This version of grep requires GNO/ME\n");
        exit(-1);
    }

    stdinput = 0; multfiles = 0;
    prog = argv[0];
    cflag = hflag = lflag = nflag = vflag = sflag = eflag = iflag = 0;
	if (argc == 1) {
    	fprintf(stderr, "usage: %s [-ichlnvs] [-e expression] [filename...]\n",
        	prog);
        exit(1);
	}
        while ((ch = getopt(argc, argv, "e:chlnvsi")) != EOF)
                switch((char)ch) {
                case 'c':
                        cflag = 1;
                        break;
                case 'h':
                        hflag = 1;
                        break;
                case 'l':
                        lflag = 1;
                        break;
                case 'n':
                        nflag = 1;
                        break;
                case 'v':
                        vflag = 1;
                        break;
                case 's':
                        sflag = 1;
                        break;
                case 'e':
                        eflag = 1;
                        exp = optarg;
                        break;
                case 'i':
                        iflag = 1;
                        break;
                case '?':
                default:
                        fprintf(stderr,
          "usage: %s [-chlnvsi] [-e] expression [filename...]\n",prog);
                        exit(1);
                }

    argc -= optind;
    argv += optind;
    if (!eflag) { exp = argv[0]; argv ++; argc--; }

    if (iflag) convLower(exp);
#ifdef EGREP
    r = regcomp(exp);
    if (r == NULL) {
       complain("bad regular expression format `%s'", exp);
       exit(2);
    }
#else
    exp2 = malloc(strlen(exp)+3);
    strcpy(exp2,"*");
    strcat(exp2, exp);
    strcat(exp2,"*");
    exp = exp2;
#endif
    if (argc == 0) { inp = stdin; stdinput = 1; argc++;}
    else if (argc > 1) multfiles = 1;
    while (argc > 0) {
        if (!stdinput) {
        	file = argv[0]; inp = fopen(file,"r");
        	if (inp == NULL) {
            	fprintf(stderr,"%s: %s (%s)\n",prog,strerror(errno),file);
	            exit(2);
            }
        }
        else file = "(pipe)";

        line = 1;
        nl.pCount = 4;
        nl.refNum = inp->_file;
    	nl.enableMask = 0x7F;
    	nl.numChars = 1;
    	nl.newlineTable = (Pointer) nlTab;
    	NewlineGS(&nl);
        rd.pCount = 4;
        rd.dataBuffer = (void *) buf;
        rd.refNum = inp->_file;
        rd.requestCount = 255l;
	done = 0;
        while (!done) {
            ReadGS(&rd);
            if (toolerror() == 0x4C) { done = 1; continue; }
            buf[rd.transferCount] = 0;
            if (iflag) {
#ifdef EGREP
		strcpy(lbuf,buf);
		convLower(lbuf);
		result = regexec(r, lbuf);
#else
		result = REGEXP(0,buf,exp);
#endif
           }
#ifdef EGREP
            else result = regexec(r, buf);
#else
            else result = REGEXP(0x8000,buf,exp);
#endif
            if (result) matches++;
            if (lflag && result) { printf("%s\n",argv[0]); break; }
            if (!sflag || !lflag) {
                if ((!result && vflag) || (result && !vflag)) {
                    if (nflag) printf("%d:",line);
                    if ((!hflag) && multfiles)
                        PrintFileName(file);
                    printf("%s",buf);
                }
            }
            line++;
        }
        if (!stdinput) { fclose(inp); }
        argc--; argv++;
    }
#ifdef GREP
    free(exp2);
#endif
    if (matches) exit(0);
    else exit(1);
}

void complain(char *s1, char *s2)
{
        fprintf(stderr, "grep: %d: ", lineno);
        fprintf(stderr, s1, s2);
        fprintf(stderr, " (%s)\n", (errseen != NULL) ? errseen : "");
        status = 1;
}
