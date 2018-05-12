/*#pragma optimize -1 */
#pragma stacksize 1024
#pragma optimize 1

/*                                                                     */
/*         GNO Installer v0.10                           James Brookes */
/*                                                                     */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <gsos.h>
#include <gno/gno.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <gsos.h>

/* variables */
int verbose = 1;
char cwd[256];
char *script;
unsigned scriptlen,scriptpos;
unsigned dabort = 0;

extern int inst_orca,inst_orcalib,inst_mu;
extern char *orca_dir;

int vars[10];

/* #defines */

#define SCRIPTFILE "9:install.script"

/* function prototypes */

void usage(char *callname);
int mygets(char *buffer, int lines);
void puke(int error, int lines);
void Abort(int lines);
int gshrc(char *cwd, int lines);
int create_dirs(int lines);
int unpack_files(int lines);
int copy_files(int lines);
int disk_request(int lines);


void strip_trailing(char *cwd)
{
unsigned l = strlen(cwd);

    cwd[l-1] = 0;
    l -= 2;
    while (cwd[l] != ':') l--;
    cwd[++l] = 0;  /* leave the trailing separator - ugh */
}

int setpfx(char *pfx, int pf)
{
extern GSString255Ptr __C2GSMALLOC(char *);
PrefixRecGS prefx;
unsigned te;

    prefx.pCount = 2;
    prefx.prefixNum = pf;
    prefx.buffer.setPrefix = (GSString255Ptr) __C2GSMALLOC(pfx);
    SetPrefixGS(&prefx);
    te = _toolErr;
    free(prefx.buffer.setPrefix);
    if (te) {
        printf("\t\t\t\t\ttoolerror: %04X\n",te);
        errno = _mapErr(te);
        return -1;
    } else return 0;
}

char *getwdn(char *pathname,int pf)
{
PrefixRecGS prefx;
int d[] = {4,0,0};
ResultBuf255Ptr where;
int e;

    prefx.pCount = 2;
    prefx.prefixNum = pf;
    prefx.buffer.getPrefix = (ResultBuf255Ptr) d;
    GetPrefixGS(&prefx);
    where = malloc(d[1]+8);
    where->bufSize = d[1]+8;
    prefx.buffer.getPrefix = where;
    GetPrefixGS(&prefx);
    if (_toolErr) {
        errno = _mapErr(_toolErr);
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

char pfx[256];

char *expand_prefix(char *path)
{
char *tmp;

    if (isdigit(path[0])) {
        getwdn(pfx,path[0]-'0');
        tmp = malloc(strlen(path)+strlen(pfx));
        strcpy(tmp,pfx);
        strcat(tmp,path+2);
        strcpy(path,tmp);
        free(tmp);
        return path;
    }
    else return path;
}

/*                                                                         */
/* mygets - get a line (skipping commented lines) and increment line count */
/*                                                                         */

char * xgets(char *buffer, int len)
{
    if (scriptpos == scriptlen) return NULL;
    while ((scriptpos < scriptlen) && (len --)) {
	if (script[scriptpos] == 13) {
	    scriptpos++;
	    break;
	}
	*buffer ++ = script[scriptpos++];
    }
    *buffer = 0;
    return buffer;
}

int mygets(char *buffer, int lines)

   {
   do

      {
      if (xgets(buffer,80)==NULL)
         puke(errno,lines);
      lines++;
      }

   while(buffer[0] == '#'); 
   return(lines);
   }

/*                      */
/* puke - stdlib errors */
/*                      */

void puke (int error,int lines)

   {
   fprintf(stderr,"\nError '%s' in line %d of script\n",strerror(errno),lines);
   exit(error);
   }

/*                                              */
/* Abort - errors in program called by system() */
/*                                              */

void Abort (int lines)

   {
   fprintf(stderr,"\nInstallation aborted at line %d of script\n",lines);
   exit(1);
   }

/*                                          */
/* gshrc - generates gshrc and initrc files */
/*                                          */

int gshrc(char *cwd, int lines)

   {
   FILE *FOutPtr;
   static char buffer[81];

     /* generate initrc file */

   strcpy(buffer,cwd);
   strcat(buffer,"gno:initrc");

   if ((FOutPtr = fopen(buffer,"w"))==NULL)
      
      {
      printf("Error writing initrc file.\n");
      puke(errno,lines);
      }
 
   if (inst_mu)
       fprintf(FOutPtr,":usr:sbin:initd\ninit\n");
   else fprintf(FOutPtr,":bin:nogetty\nnogetty\n");
   fclose(FOutPtr);

       /* generate gshrc file */

   strcpy(buffer,cwd);
   strcat(buffer,"gno:user:root:gshrc");

   if ((FOutPtr = fopen(buffer,"w")) == NULL)
 
      {
      printf("Error writing gshrc file.\n");
      puke(errno,lines);
      }

   fprintf(FOutPtr,"###\n#\n# GNO 2.0 gshrc file\n#\n###\n");
   fprintf(FOutPtr,"#\n# Initialize our environment\n#\n");
   fprintf(FOutPtr,"set path=\"%sgno:bin %sgno:usr:bin",cwd,cwd);
   if (inst_orca) fprintf(FOutPtr," %s:utilities\"\n",orca_dir);
   else fprintf(FOutPtr,"\"\n");
   fprintf(FOutPtr,"set prompt=\"[%ch] %cS%ct%cs %cC> \"\n",37,37,37,37,37);
   fprintf(FOutPtr,"set home=\"%sgno:user:root\"\n",cwd);
   fprintf(FOutPtr,"set term=gnocon\n");
   fprintf(FOutPtr,"export path prompt home term\n");
   fprintf(FOutPtr,"setenv history=100 savehist=25\n");
   if (inst_orca) {
   	fprintf(FOutPtr,"###\n#\n#Set up standard prefixes for utilities.\n#\n###\n");
   	fprintf(FOutPtr,"prefix 2 %s:libraries\n",orca_dir);
   	fprintf(FOutPtr,"prefix 3 %s\n",orca_dir);
   	fprintf(FOutPtr,"prefix 4 %s:shell\n",orca_dir);
   	fprintf(FOutPtr,"prefix 5 %s:languages\n",orca_dir);
   	fprintf(FOutPtr,"prefix 6 %s:utilities\n",orca_dir);
   	fprintf(FOutPtr,"prefix 7 :tmp\n");
   	fprintf(FOutPtr,"###\n#\n# Set up prefixes for Orca2.0(tm)'s benefit\n#\n###\n");
   	fprintf(FOutPtr,"prefix 13 %s:libraries\n",orca_dir);
   	fprintf(FOutPtr,"prefix 14 %s\n",orca_dir);
   	fprintf(FOutPtr,"prefix 15 %s:shell\n",orca_dir);
   	fprintf(FOutPtr,"prefix 16 %s:languages\n",orca_dir);
   	fprintf(FOutPtr,"prefix 17 %s:utilities\n",orca_dir);
   }
   fprintf(FOutPtr,"alias ls 'ls -CF'\n");
   fprintf(FOutPtr,"alias dir 'ls -al'\n");
   fprintf(FOutPtr,"alias cp 'cp -i'\n");
   fprintf(FOutPtr,"alias rm 'cp -p rm'\n");
   fprintf(FOutPtr,"alias mv 'cp -p mv'\n");
   fprintf(FOutPtr,"setenv usrman='/usr/man'\n");
   fprintf(FOutPtr,"set fignore='.a .root .sym'\n");
   fprintf(FOutPtr,"alias zcat 'compress -cd'\n");
   fprintf(FOutPtr,"setenv pager=less\n");
   fprintf(FOutPtr,"setenv less=-e\n");
   fprintf(FOutPtr,"set nonewline=1\n");

   fprintf(FOutPtr,"#\n# Move to home directory\n#\ncd\n");
   fclose(FOutPtr); 

   strcpy(buffer,cwd);
   strcat(buffer,"gno:etc:namespace");

   if ((FOutPtr = fopen(buffer,"w"))==NULL)
      
      {
      printf("Error writing /etc/namespace file.\n");
      puke(errno,lines);
      }
   fprintf(FOutPtr,":usr\t%sgno:usr\n",cwd);
   fprintf(FOutPtr,":bin\t%sgno:bin\n",cwd);
   fprintf(FOutPtr,":etc\t%sgno:etc\n",cwd);
   fprintf(FOutPtr,":tmp\t%sgno:tmp\n",cwd);
   fprintf(FOutPtr,":user\t%sgno:user\n",cwd);
   fprintf(FOutPtr,":dev\t%sgno:dev\n",cwd);
   fprintf(FOutPtr,":var\t%sgno:var\n",cwd);
   fclose(FOutPtr);

   return(lines);
   }

/*                                                              */
/* create_dirs - create standard directory structure with mkdir */
/*                                                              */

int create_dirs(int lines)

   {
   static char cline[1024], buffer[81];
   int err;

   strcpy(cline,"mkdir -s ");
   printf("\nCreating directories...\n");
   lines = mygets(buffer,lines);
   
   while(buffer[0] != '&')

      {
      strcat(cline," ");
      strcat(cline,buffer); 
      lines = mygets(buffer,lines);
      }     
          
   if (verbose) printf("...%s\n",cline);
   err = system(cline);
   if ((err != 0) && (err != 2))
      Abort(lines);
   return(lines);
   }

/*                                                                      */
/* unpack_files - unpack files with yankit, and move the unpacked files */
/*                to the proper directory                               */
/*                                                                      */

int unpack_files(int lines)
{
static char cline[1024], buffer[256];
static char oldpath[256];
int readVar;

    printf("\nUnpacking compressed files");
    while(1) {
#if 0
        ioctl(1,FIONREAD,&readVar);
      	if (readVar != 0) {
	    fprintf(stderr,"\n\nInstallation terminated.\n");
	    exit(1);
        }
#endif
    	/* get archive name */
    	lines = mygets(buffer,lines);
	if (buffer[0] == '&') break;
        expand_prefix(buffer);
        strcpy(cline,"yankit xv ");
	strcat(cline,buffer);
	lines = mygets(buffer,lines);  /* get new name */
        if (verbose) printf("...cd %s\n",buffer);
        chdir(buffer);
	if (verbose) printf("...%s\n",cline);
      	if (system(cline))
	    Abort(lines);
        if (verbose) printf("...cd %s\n",cwd);
        chdir(cwd);
    } 

    return(lines);
}

/*                                                      */ 
/* copy_files - copy files to proper location with 'cp' */
/*                                                      */

int copy_files(int lines)

   {
   static char cline[1024], buffer[256];
   int readVar;

   printf("\nCopying uncompressed files");
   lines = mygets(buffer,lines);

   do

      {
#if 0
      ioctl(1,FIONREAD,&readVar);
      if (readVar != 0)

         {
         fprintf(stderr,"Installation terminated.\n");
         exit(1);
         }
#endif
      strcpy(cline,"cp "); 
      expand_prefix(buffer);
      strcat(cline,buffer);
 
      lines = mygets(buffer,lines);
 
      strcat(cline," ");
      strcat(cline,buffer);
      if (verbose) printf("...%s\n",cline);
      if (system(cline)) 
         Abort(lines); 
      printf(".");

      lines = mygets(buffer,lines);
      }

   while (buffer[0] != '&');
   return(lines);
   }

/*              */
/* disk_request */
/*              */

int disk_request(int lines)

   {
   char volume[32];
   FILE *acc;
   int c;

   lines = mygets(volume, lines);

   while (access(volume,F_OK)) {
      printf("\nPlease insert disk %s, and press RETURN to continue.",volume);
      c = getc(stdin);
      if (c == 3) { dabort = 1; return; }
   }
   printf("\n");
   return(lines);
   }

/*      */
/* main */
/*      */

void mainInstall (void)
{
static char buffer[81], cline[1024], prompt[20];
int readVar, flag, done = FALSE, lines = 0;
int infd;
int num; /* variable number */


    vars[0] = inst_orca;
    vars[1] = inst_orcalib;
    vars[2] = inst_mu;

    printf("Starting installation\n");

      /* first get current working directory */

    getwd(cwd);
    /* strip trailing component and use that as location to place GNO */
    strip_trailing(cwd);
    chdir(cwd);

    /* Read the entire script file into memory */
    if ((infd = open(SCRIPTFILE,O_RDONLY)) < 1)
        puke(errno,lines);
    script = malloc(32767l);
    scriptlen = read(infd,script,32767l);
    scriptpos = 0;
    close(infd);

/*                                                                 */
/*      main loop                                                  */
/*                                                                 */

   while(!done)

      {
      if (dabort) return;
      lines = mygets(buffer,lines);
      switch(buffer[0])

         {
         case '&':

            if (!strcmp(buffer,"&directories"))  /* create directories */
               lines = create_dirs(lines);

            else if (!strcmp(buffer,"&compressed_files"))
               lines = unpack_files(lines);
         
            else if (!strcmp(buffer,"&uncompressed_files"))
               lines = copy_files(lines);

            else if (!strcmp(buffer,"&disk"))
               lines = disk_request(lines);

            else if (!strncmp(buffer,"&if",3)) {
		num = atoi(buffer+4);
		if (!(vars[num])) {
                    do {	/* Skip the body of the if statement */
                    	lines = mygets(buffer,lines);
                    } while (strcmp(buffer,"&endif"));
                }
            }
            else if (!strcmp(buffer,"&endif")) {
            	/* we simply ignore this, assume we executed the body of the
	           if statement */
	    }
            else
               
               {
               fprintf(stderr,"\nBad '&' command, line %d of script.\n",lines);
               exit(1);
               }

            break;
        
         case '@':

            done = TRUE; 
            break;
         
         default :

            fprintf(stderr,"\nUnknown command %c ",buffer[0]);
            fprintf(stderr,"line %d of script.\n",lines);
            exit(1);
         } 
 
      }

   lines = gshrc(cwd,lines);  /* generate gshrc and initrc files */
   printf("\n\nInstallation successful.\n");   
   printf("Total lines in script: %d\n",lines);
   }
