#pragma stacksize 1024

int fork2(void *, int, int, char *, int, ...);

/*
 * Copycat v1.4.1 - copies information from one tty to another bi-directionally.
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

#define tprintf(tty, string) write(tty, string, strlen(string))

#define _VERSION_ "1.4.1"
#define ORCA_LAMER 32767

#define FALSE 0
#define TRUE  1

/* function prototypes */

void usage(char *callname);
void quit(void);                  /* signal handler           */
void shellout();        
void help(void);                  /* prints a help screen     */
void read_tty1(void);             /* child process functions  */
void read_tty2(void);
void set_tty(int ttyid);          /* set tty up for RAW/CRMOD */
void restore_tty(int ttyid);      /* restore tty to prev mode */

/* globals */

int tty1, tty2,                /* tty file descriptors */
main_pid, pid1, pid2,          /* process numbers      */ 
main_pgrp, pgrpA, pgrpB,       /* process groups       */
echo;                          /* echo flag            */

char *buffer1, *buffer2, tty1name[7];
struct sgttyb sb;
struct sgttyb *s = &sb;

int rexit(int x)
{ exit(x); }

/*
 * usage - tell the uneducated user how to run the durn thing 
 */

void usage(char *callname)

   {
   fprintf(stderr,"Copycat v%s\n",_VERSION_);
   fprintf(stderr,"usage: %s <tty1> <tty2>\n",callname);
   fprintf(stderr,"       where tty2 is the controlling tty.\n");
   rexit(0);
   }

/*
 *  read_tty1 - read from tty1, write to tty2.  This function is split off
 *              as a child process of main().
 */

#pragma databank 1

void read_tty1(void)

   {
   int intVar;

   settpgrp(tty1);
   while(1)

      {
      ioctl(tty1,FIONREAD,&intVar);   /* char(s) in first tty?   */
      if (intVar > 1024) intVar = 1024;
      else if (!intVar) intVar = 32;
      intVar = read(tty1,buffer1,intVar); /* read intVar characters  */
      write(tty2,buffer1,intVar);      /* and write to second tty */
      }

   }

/*
 *  read_tty2 - read from tty2, write to tty1, call shellout() if 
 *              a shellout char (^\) detected.  This is the second
 *              child of main.
 */

void read_tty2(void)

   {
   int intVar;

   settpgrp(tty2);
   while(1)

      {
      ioctl(tty2,FIONREAD,&intVar);         /* char(s) in first tty?    */
      if (!intVar) intVar = 32;		    /* read _up to_ 32 chars    */
      intVar = read(tty2,buffer2,intVar);             /* read intVar characters   */
      if (*buffer2 == 28)                    /* got a shellout char!     */
         shellout();                       
      else                                  /* otherwise write intVar   */
         write(tty1,buffer2,intVar);         /* characters to second tty */
      }

   }


/*
 * shellout - handles shelling out of the copycat program, and allows
 *            quitting, suspending, and the execution of shell commands.
 */

void shellout(int sig, int code)

   {
   static char tmpbuf[81];
   int done, i; 

   done = FALSE;
   kill(pid1,SIGSTOP);              /* pause reading from tty1      */
   signal(SIGQUIT,SIG_IGN);         /* ^/ has served its purpose... */

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
            tmpbuf[i] = 8;
            write(tty2,tmpbuf+i,1);
            continue;
            }

         write(tty2,tmpbuf+i,1);

         }

      while(tmpbuf[i++] != '\r');

      tmpbuf[--i] = '\0';
 
      if (!strncmp(tmpbuf,"quit",4))

         {
         kill(main_pid,SIGTERM);
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

      else if (!strncmp(tmpbuf,"help",4)||((tmpbuf[0]=='?')&&(tmpbuf[1]=='\r')))
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
         kill(-main_pgrp,SIGSTOP);         /* pause main and 2nd child procs */
         tprintf(tty2,"\r");
         sleep(1);                         /* pause until restarted before   */
         }                                 /* continuing on and printing the */
                                           /* "copycat> " prompt             */
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
   kill(pid1,SIGCONT);         /* restart tty1 read/write proc */
   signal(SIGQUIT,SIG_DFL);    /* restore ^/ signal handler    */
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
#if 0
    fprintf(stderr,"sg_flags: %04X\n\r",s->sg_flags);
    fflush(stderr);
#endif
}

/*
 * set_tty - sets tty 'ttyid' to RAW and !CRMOD modes 
 */

void set_tty(int ttyid)

   {
   gtty(ttyid,s);                       /* set up tty in raw/crmod modes */
   printTTY(s);
   s->sg_flags |= (RAW);                /* RAW means non-echoing         */
   /*s->sg_flags &= (~CRMOD);             /* CRMOD means don't append a \n */
   printTTY(s);
   stty(ttyid,s);
   }

/* 
 * restore_tty - restores tty 'ttyid' to ECHO and CRMOD modes
 */

void restore_tty(int ttyid)

   {
   gtty(ttyid,s);           /* restore default settings to tty... */
   s->sg_flags &= (~RAW);
   s->sg_flags |= (CRMOD);
   stty(ttyid,s);
   }

/*
 *  quit - caught a SIGTERM, so now it is time to finish
 */

void quit(void)

   {
   restore_tty(tty1);
   restore_tty(tty2);

   kill(pid1,SIGKILL);       /* Kill the children (cue violins now) */
   kill(pid2,SIGKILL);

   free(buffer1);
   free(buffer2);
   /*free(s);*/

   rexit(0);
   }

/*
 * main - sets things up, main event loop
 */

int main(int argc, char **argv)

   {
   union wait ss;                      /* Status returned by wait() */
   int intVar, i;
   int db;
   echo = FALSE;

        /* Check for GNO system */

      if (!needsgno())
      {
      fprintf(stderr,"\ncopycat requires the GNO/ME system.\n");
      exit(ORCA_LAMER);
      }
 
   if (argc != 3)   
      usage(argv[0]);

   signal(SIGTERM,quit);               /* set up signal handlers    */
   /*signal(SIGHUP,quit);*/
   signal(SIGHUP,SIG_IGN);
   signal(SIGTSTP,SIG_IGN);
   signal(SIGINT,SIG_IGN);
  
   main_pid = getpid();

   buffer1 = (char *) malloc (2048l);
   buffer2 = (char *) malloc (2048l);
   if ((buffer1 == NULL) || (buffer2 == NULL)) {
	fprintf(stderr,"couldn't allocate memory\n");
        exit(1);
   }

   /*s = (struct sgttyb *) malloc (sizeof(struct sgttyb));*/

   tty1 = open(argv[1],O_RDWR|O_BINARY);
   tty2 = open(argv[2],O_RDWR|O_BINARY);
   
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

   pgrpA = tcnewpgrp(tty1);  /* set proper process groups for ttys  */
   pgrpB = tcnewpgrp(tty2);  
   settpgrp(tty2);
   main_pgrp = getpgrp(main_pid);

   pid1 = fork2(read_tty1,1024,0,"cc-modem",0);
   pid2 = fork2(read_tty2,1024,0,"cc-console",0);
 /*  pid1 = fork(read_tty1);   /* fork off child processes            */
 /*  pid2 = fork(read_tty2);*/

   /* wait till child #2 dies */

   while ((wait(&ss) != pid2) || (WIFSTOPPED(ss)));
   }
