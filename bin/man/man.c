/*
 * Some code copyright others (it was public domain source), and this source
 * code is given freely.  If you make changes and distribute, please include
 * information like above.  Remember, someone did work to get you started,
 * remember them also.
 *
 * Original Author:  Gil Kloepfer, Jr  Sometime in 1989
 * Revision History:
 * 14/02/92  MEH  	Started coding for first time.  Fixed
 *                	for GNO/ME.
 *                	Piping problems.  PAGER removed for now
 * 13/03/92  MEH  	Put in code for using both nroff and
 *                	aroff together, so that both types of
 *                	manpages can be used in the same dirs.
 *                	Need to fix it for piping though, it is
 *                	not working right now.
 * 15/03/92  JB   	Reenabled pager, fixed pager use for
 *		  	preformatted, uncompressed files
 *		  	Fixed getfileinfo bug
 *  7/04/92  MEH  	Added the code so that a compressed
 *                	nroff type manpage can be piped
 *                	correctly without errors.
 * 27/04/92  MEH  	added code for using either zcat or
 *                	fcat  (freeze makes smaller files)
 *                	added enviroment support for a pager
 *                	that the user can specify.
 * 18/06/92 MEH		Fixed the 'Reformatting...' thing so
 *			that it only prints when needed.
 * Jul 11/93 JB		Added code to support 'manpage links'
 * Nov 27/93 JB		No longer uses case-sensitive compare
 * 			when doing a lookup
 *			Now accepts a BIN filetype for compressed manpages
 *
 */

#pragma stacksize 1536
#pragma optimize -1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sgtty.h>			/*done for tty usage, because isatty()
					  is there.  Thanks JB for doing this */
#include <sys/types.h>
#include <dirent.h>
#include <gsos.h>
#include <sys/stat.h>
#include <ctype.h>

#define	PROCD	procdBuf		/* dir prefix for preformatted pages */
#define	NPROCD	nprocdBuf		/* dir prefix for nroff format pages */
#define	MAXPATH	128			/* maximum size of path to man page */

char separator;
char procdBuf[MAXPATH];
char nprocdBuf[MAXPATH];

/* this assumes you have these commands in your $path */

#define	TYPECOMP	1	 	/* command to display a compressed file
					   Remember, zcat-ing a compress AROFF input
					   doesn't work!  Try if you don't believe me */
#define	FORMAT		"aroff"		/* command to format AWGS man pages*/
#define FORMAT1		"nroff -man"	/* command to format normal man pages*/
#define	PAGER		pagerblah	/* system command to page a file */

char pagerblah[33];

void resolveLink(char *);

/* Don't play with these constants!! */
#define	C_PROC	1
#define	C_NPROC	2

int main(int argc, char *argv[])
{
int  argno, section, kind, lookup();
char command[20], pagename[MAXPATH];
char *usrman, *userman;
char *pagerblah2;
size_t umlength;
int x;

       if(!needsgno())
       {
       		fprintf(stderr,"man:  requires GNO/ME");
       		exit(-1);
       }
       userman=getenv("USRMAN");
       if(userman==NULL)
       {
		fprintf(stderr, "\nset USRMAN before executing\n");
                fprintf(stderr, "example:  set USRMAN=\"/f/gno/man\"\n");
                fprintf(stderr, "          (no trailing / or it won't work\n");
                exit(2);
       }
       usrman=(char *) malloc(strlen(userman+2));
       strcpy(usrman, userman);
       umlength = strlen(usrman);
       /* remove any trailing separator */
       if ((usrman[umlength-1] == '/') || (usrman[umlength-1] == ':'))
           usrman[umlength-1] = 0;

       pagerblah2=getenv("PAGER");
       if(pagerblah2==NULL)
		strcpy(pagerblah, "more");
       else
		strcpy(pagerblah, pagerblah2);

       /* Append the proper separator to the end of the string */
       x = strcspn(usrman,":/");
       if (x == umlength-1) {
           separator = '/';
           usrman[umlength] = '/';
           usrman[umlength+1] = 0;
       } else {
           separator = usrman[x];
           usrman[umlength] = separator;
           usrman[umlength+1] = 0;
       }
       strcpy(procdBuf, usrman);
       strcat(procdBuf, "cat");
       strcpy(nprocdBuf, usrman);
       strcat(nprocdBuf, "man");

       /* check the command line arguments */

       switch(argc)
	{
		case 2:  /* just the command name */
			strcpy(command, argv[1]);
			section=0;
			break;

		case 3:  /* the section and command name */
			sscanf(argv[1], "%d", &section);
			strcpy(command, argv[2]);
			break;

		default:  /* display the usage statement */
			fprintf(stderr,"usage: %s [section] command\n", argv[0]);
			exit(1);
       }

       /*
        * If the section is not specified explicitly, try to find
        * the manual page in the first directory where it's encountered.
        */

       if (section == 0)
	{
		for (section=1; section<=8; section++)
			if (lookup(command,section,pagename,&kind))
			  {
#ifdef DEBUG
				fprintf(stderr,"%s: assuming %s from section %d\n",
					argv[0], command, section);
#endif
			printpage(pagename,kind);
			exit(0);
			  }
		fprintf(stderr,"%s: cannot find %s in the manual\n", argv[0], command);
		exit(2);
	}

       /*
        * If the user specified a section, then see if we can find it
        * there .. if so, print the page
        */

	if (lookup(command,section,pagename,&kind))
	{
		printpage(pagename,kind);
		exit(0);
	}
	else
	{
		fprintf(stderr,"%s: cannot find %s in section %d\n", argv[0], command, section);
		exit(2);
	}
}


/*
 * Function lookup returns 0 on failure and 1 on success.  It will look
 * through the directories for section and try to find the manual page
 * for command.  If it finds it, the entire filename is returned as
 * pagename.  "kind" passes along whether the name was found in the
 * processed or unprocessed directory phases
 */

int lookup(char *command, int section, char *pagename, int *kind)
{
int	passno, cmpman();
char	fullpath[MAXPATH];
DIR	*mdir;
struct	dirent *dentry;

   /*
    * Look through the formatted directory first to see if we
    * can get one that's formatted.
    */

    for (passno=1; passno<=2; passno++)
    {
	switch(passno)
	{
	    case 1:  /* check formatted directory */
		    sprintf(fullpath, "%s%d", PROCD, section);
		    *kind = C_PROC;
		    break;
	    case 2:  /* check unformatted directory */
		    sprintf(fullpath, "%s%d", NPROCD, section);
		    *kind = C_NPROC;
		    break;
	}
#ifdef DEBUG
	printf("fullpath=%s\n", fullpath);
#endif
	if ((mdir=opendir(fullpath)) != NULL)
	    while ((dentry=readdir(mdir)) != NULL)
		if (cmpman(dentry->d_name,command) == 0)
		{
		    closedir(mdir);
                    if (toupper(dentry->d_name[dentry->d_namlen-1]) == 'L') {
		        sprintf(pagename, "%s%c%s", fullpath, separator, dentry->d_name);
                        resolveLink(pagename);
                        return(1); /* we have the name */
                    }
		    sprintf(pagename, "%s%c%s", fullpath, separator, dentry->d_name);
		    return(1);
		}
	closedir(mdir);
    }
    return(0);
}


/*
 * Compare name with command and see if it matches before the period
 * so we can crown it the matching manual page for command
 */

int cmpman(char *name, char *command)
{
#ifdef DEBUG
    printf("name: %s  command: %s\n",name,command);
#endif
    while ((toupper(*name) == toupper(*command)) && *name != '\0' && *command != '\0')
    {
	name++;
	command++;
    }
    if (*name == '.' && *command == '\0')
	return(0);  /* OK */
    if (toupper(*command) == toupper(*name))
	return(0);  /* OK */
    return(1);  /* all other cases fail */
}


/*
 * Printpage prints a manual page.  It formats the page depending on
 * where it comes from, and invokes the system pager depending on
 * whether output is to a tty or a file/pipe
 */

printpage(char *pageloc, int form)
{
int	compress, endstr, ttypage;
char	cmdline[256];
int	filetype;

   /* check for compressed file (.Z || .z) */

    endstr=strlen(pageloc);
    if (pageloc[endstr-2]=='.' && (pageloc[endstr-1] == 'F' || pageloc[endstr-1] == 'f' || pageloc[endstr-1]=='Z' || pageloc[endstr-1]=='z'))
	compress=1;
    else
	compress=0;

   /* If we're a tty, page this using whatever our pager is */

    if (isatty(1))
        ttypage=1;
    else
	ttypage=0;

    if (compress)
    {
	if (form == C_NPROC)
        {
	    filetype=GetFileTypeMan(pageloc);
            switch (filetype)
            {
	        case 176:
                case 6: /* for compress manpages, accept 'bin' types too */
                case 4: sprintf(cmdline, "%ccat %s | %s -", _tolower(pageloc[endstr-1])/*TYPECOMP*/, pageloc, FORMAT1);
                        break;
                case 80: sprintf(cmdline, "%ccat %s | %s", _tolower(pageloc[endstr-1])/*TYPECOMP*/, pageloc, FORMAT);
                    	break;    
                default: puts("Not TXT, SRC, or AWGS file");
                        exit(1);
       	    }
	    fprintf(stderr, "\nReformatting page, please wait...\n");
   	}
   	else
	    sprintf(cmdline, "%ccat %s", _tolower(pageloc[endstr-1]), pageloc);

    }
    else
    {
	if (form == C_NPROC)
        {
	    filetype=GetFileTypeMan(pageloc);
            switch (filetype)
            {
	        case 176:
	        case 4:  sprintf(cmdline, "%s %s", FORMAT1, pageloc);
	                 break;
                case 80: sprintf(cmdline, "%s %s", FORMAT, pageloc);
                         break;
                default: puts("Not TXT, SRC, or AWGS file");
                         exit(1);
      	    }
	    fprintf(stderr, "\nReformatting page, please wait...\n");
    	}
        else
	    sprintf(cmdline, "%s %s", PAGER, pageloc);
    }

#ifdef PAGER
    if ((ttypage=1) && ((form == C_NPROC) || (compress)))
    {
	strcat(cmdline," | ");
	strcat(cmdline,PAGER);
    }
#endif
    system(cmdline);
}

int GetFileTypeMan(char *filepath)
{
struct FileInfoRecGS fi;

    fi.pCount=(Word) 3;
    fi.pathname = malloc(strlen(filepath)+3);
    fi.pathname->length=(Word) strlen(filepath);
    strcpy(fi.pathname->text, filepath);
    GetFileInfoGS(&fi);

    free(fi.pathname);
    return (int) fi.fileType;
}

void resolveLink(char *name)
{
FILE *f;
static char fname[128];
int l;

    f = fopen(name,"r");
    fgets(fname,127,f);
    fclose(f);
    l = strlen(fname);
    fname[l-1] = 0; /* remove the newline at the end of the name */
    strcpy(name,fname); /* set the _real_ name */
}
