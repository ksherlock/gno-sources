#pragma stacksize 1024

/*
 * Copycat v1.5.0 - copies information from one tty to another bi-directionally.
 *          
 *     Parameters:   Takes two parameters on startup: two tty names.  
 *                   For example:
 *
 *                       % copycat .ttya .ttyb &
 *
 *                   is the command I use to start copycat running; this command
 *                   links my modem and terminal (which is connected to the
 *                   printer port) and runs it in the background.
 *
 * *** IMPORTANT *** Note that the second tty MUST be a terminal in control
 *                   of the user, as it is the only one in which the break
 *                   character (^\) will be interpreted.
 *  
 *     Return codes: None
 *
 *     Author:       James Brookes
 *                   jamesb@cscihp.ecst.csuchico.edu
 *
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <gno/gno.h>
#include <sgtty.h>
#include <signal.h>
#include <unistd.h>

#pragma lint -1

#define tprintf(tty, string) write(tty, string, strlen(string))

#define _VERSION_ "1.5.0"

#define FALSE 0
#define TRUE  1

#define BREAK_CHAR 28

#define FD_SET(f,r) (r.fds_bits[0] |= (1 << f))
#define FD_ISSET(f,r) (r.fds_bits[0] & (1 << f))

/* library function prototypes */

/*void rexit(int exitcode);*/
#define rexit(x) exit(x)

/* function prototypes */

void usage(char *callname);
void quit(void);                  /* signal handler           */
void shellout(void);
void help(void);                  /* prints a help screen     */
void read_tty1(int,char *);             /* child process functions  */
void read_tty2(int,char *);
void set_tty(int ttyid);          /* set tty up for RAW/CRMOD */
void restore_tty(int ttyid);      /* restore tty to prev mode */

/* globals */

int tty1, tty2,                /* tty file descriptors */
echo;                          /* echo flag            */
FILE *FInPtr;                  /* file pointer for console input */
char *buffer;

char tty1name[7];
struct sgttyb sb;
struct sgttyb *s = &sb;

/*
 * usage - tell the uneducated user how to run the durn thing 
 */

void usage(char *callname)

   {
   fprintf(stderr,"Copycat v%s\n",_VERSION_);
   fprintf(stderr,"usage: %s <tty>\n",callname);
   rexit(0);
   }

/*
 *  read_tty1 - read from tty1, write to tty2.  This function is split off
 *              as a child process of main().
 */

void read_tty1(int nchar, char *buf)

   {
      write(tty2,buf,nchar);         /* write to second tty */
   }

/*
 *  read_tty2 - read from tty2, write to tty1, call shellout() if 
 *              a shellout char (^\) detected.  This is the second
 *              child of main.
 */

void read_tty2(int nchar, char *buf)

   {
      if (*buf == BREAK_CHAR)            /* got a shellout char!     */
         shellout();                       
      else                                   /* otherwise write intVar   */
         write(tty1,buf,nchar);         /* characters to second tty */
   }

#pragma databank 1

/*
 * shellout - handles shelling out of the copycat program, and allows
 *            quitting, suspending, and the execution of shell commands.
 */


void shellout(void)

   {
   static char tmpbuf[81];
   int done, i; 

   done = FALSE;

   restore_tty(tty2);               /* set up for RAW/CRMOD modes   */
 
   while(!done)

      {
      sprintf(tmpbuf,"\rcopycat> ");
      tprintf(tty2,tmpbuf);
      i=0;

      do                            /* get command */
     
         {
         read(tty2,&tmpbuf[i],1);  
         if (tmpbuf[i] == 0x08 || tmpbuf[i] == 0x7F)

            {
            i--;
            continue;
            }

         }

      while(tmpbuf[i++] != '\r');

      tmpbuf[--i] = '\0';
      tprintf(tty2,"\r");
/*
      fgets(tmpbuf,40,FInPtr);
*/
      if (!strncmp(tmpbuf,"quit",4))

         {
         kill(getpid(),SIGTERM);
         return;
         }

      else if (!strncmp(tmpbuf,"mt",2))    /* turn off mousetext */
         tprintf(tty2,"\030");

      else if (!strncmp(tmpbuf,"echo",4))  /* toggle echo flag */
          
         {
         switch (echo)
           
            {
            case TRUE:
               set_tty(tty1); 
               set_tty(tty2);
               break;
            case FALSE:
               restore_tty(tty1);
               restore_tty(tty2);
            }

         echo ^= 0x0001;
         }

      else if ( (!strncmp(tmpbuf,"help",4)) || (tmpbuf[0]=='?') )
         help();

      else if (!strncmp(tmpbuf,"write",5))

         {
         for (i = 0; i < 5; i++)           /* clear 'write' from buffer  */
            tmpbuf[i] = ' ';   

         strcat(tmpbuf,">");               /* append redirection and tty */
         strcat(tmpbuf,tty1name);
         tprintf(tty2,"\r");
         system(tmpbuf);
         }

      else if (!strncmp(tmpbuf,"z",1))

         {
         kill(getpid(),SIGSTOP);         /* pause main and 2nd child procs */
         tprintf(tty2,"\r");
         }

      else if (tmpbuf[0] == '\0')
         done = TRUE;

      else if (tmpbuf[0] == '!')           /* shell command */

         {
         tmpbuf[0] = ' ';                  /* erase '!' from string */
         tprintf(tty2,"\r");
         system(tmpbuf);
         }
 
      else
         tprintf(tty2,"Invalid command!");
      }

   if (!echo) set_tty(tty2);   /* set back in RAW, !CRMOD mode */
   }

/*
 * help - prints help screen for 'copycat> ' menu
 */

void help(void)

   {
   static char tmpbuf[81];
   static char echo_str[2][4] = { "OFF", "ON" };

   sprintf(tmpbuf,"Commands for copycat v%s\r",_VERSION_);
   tprintf(tty2,tmpbuf);
   tprintf(tty2,"\r?,help                this screen");
   
   sprintf(tmpbuf,"\recho                  toggle echo (currently %s)",echo_str[echo]);
   tprintf(tty2,tmpbuf);
   tprintf(tty2,"\rmt                    turn off mousetext");
   tprintf(tty2,"\r!<command>            shell executes <command>");
   sprintf(tmpbuf,"\rwrite <command>       execute command and redirect to %s",tty1name); 
   tprintf(tty2,tmpbuf);
   tprintf(tty2,"\rquit                  exit copycat");
   tprintf(tty2,"\rz                     suspend copycat and return to shell");
   tprintf(tty2,"\r[CR]                  resume copying");
   }

#pragma databank 0

void printTTY(struct sgttyb *s)

   {
   fprintf(stderr,"sg_flags: %04X\n\r",s->sg_flags);
   fflush(stderr);
   }

/*
 * set_tty - sets tty 'ttyid' to RAW and !CRMOD modes 
 */

void set_tty(int ttyid)

   {
   gtty(ttyid,s);                       /* set up tty in raw/crmod modes */
   s->sg_flags |= (RAW);                /* RAW means non-echoing         */
   s->sg_flags &= (~CRMOD);             /* CRMOD means don't append a \n */
   stty(ttyid,s);
   }

/* 
 * restore_tty - restores tty 'ttyid' to ECHO and CRMOD modes
 */

void restore_tty(int ttyid)

   {
   gtty(ttyid,s);           /* restore default settings to tty... */
   s->sg_flags &= ~(RAW|CBREAK);
   s->sg_flags |= (CRMOD);
   s->sg_flags |= ECHO;
   stty(ttyid,s);
   }

/*
 *  quit - caught a SIGTERM, so now it is time to finish
 */

void quit(void)

   {
   restore_tty(tty1);
   restore_tty(tty2);

   free(buffer);

   rexit(0);
   }

/*
 * main - sets things up, main event loop
 */

int main(int argc, char **argv)

   {
   char *ttyName;
   union wait ss;                      /* Status returned by wait() */
   int intVar, i, slot;
   unsigned quitCopycat = 0;
   fd_set a;
   int xx;
  
   echo = FALSE;

        /* Check for GNO system */

   if (!kernStatus())

      {
      fprintf(stderr,"\ncopycat requires the GNO/ME system.\n");
      exit(1);
      }
 
   if (argc != 2)   
      usage(argv[0]);

   signal(SIGTERM,quit);               /* set up signal handlers    */
   signal(SIGHUP,SIG_IGN);
   signal(SIGTSTP,SIG_IGN);
   signal(SIGINT,SIG_IGN);
   signal(SIGQUIT,SIG_IGN);
  
   buffer = (char *) malloc (2048l);

     /* check to see if the requested tty is free.  We'll assume that */
     /* the calling tty is free                                       */

   tty1 = open(argv[1],O_RDWR|O_BINARY);
   ttyName = ttyname(STDIN_FILENO);
   tty2 = open(ttyName,O_RDWR|O_BINARY);
   
    /* check to make sure each tty open()ed properly */      

   if (tty1 == -1 || tty2 == -1)
      
      {
      if (tty1 == -1) i = 1;
      else i = 2;
      fprintf(stderr,"\rError opening tty %s, aborting.\r",argv[i]);
      rexit(1);
      }

   strcpy(tty1name,argv[1]);
   tprintf(tty2,"\rBreak character is '^\\'\r\r");

   set_tty(tty1);            /* set ttys to be RAW, !CRMOD          */
   set_tty(tty2);

#if 0
   pgrpA = tcnewpgrp(tty1);  /* set proper process groups for ttys  */
   pgrpB = tcnewpgrp(tty2);  
   settpgrp(tty2);
   main_pgrp = getpgrp(main_pid);
#endif

   while (!quitCopycat) {
	a.fds_bits[0] = 0l;
	FD_SET(tty1,a);
	FD_SET(tty2,a);
	xx = select(32,&a,NULL,NULL,NULL);
	if (xx == -1) {
	    fprintf(stderr,"select() failed\n");
	    quit();
	}
	if (FD_ISSET(tty1,a)) {
	    ioctl(tty1,FIONREAD,&intVar);
	    if (intVar > 1024) intVar = 1024;
	    read(tty1,buffer,intVar);
	    read_tty1(intVar,buffer);
	}
	if (FD_ISSET(tty2,a)) {
	    ioctl(tty2,FIONREAD,&intVar);
	    if (intVar > 1024) intVar = 1024;
	    read(tty2,buffer,intVar);
	    read_tty2(intVar,buffer);
	}
   }

   }
