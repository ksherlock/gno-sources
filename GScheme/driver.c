/*     (c) 1989, Procyon Software and Jawaid Bazyar

          This code is copyrighted FreeWare. Please distribute it
          as widely as possible.  There is no fee for use of
          this product, but if you want to see future versions
          some $$$ would be a great enticement to complete the
          project.

          Jawaid Bazyar
          306 N. 7th
          Mt. Vernon, IL 62864

          jb10320@uxa.cso.uiuc.edu
*/

/*

      modification history started 3/9/89

beta vsn   date     mod
--------   -------- -----------------------------------------------------------
  0.4b     6/12/89  Modification history now contained inside SCHEME0.xB.DOCS
  0.35b    not rlsd Finished make-environment & handling procedures.
                    Tail Recursion speed study indicates it is much faster
                    than regular recursion, but has more work to do (in FACT
                    study anyway).
                    Added OA-. interruption key.
   c        3/13/89 Begin study of Full-implementation Garbage Collector.
   b        3/13/89 Completed Tail Recursion/Memory usage study. Removed
                    extraneous 'pushnil' from ReverseEval.
   b        3/09/89 started work on Tail Recursion, completed modification of
                    internal stacks to linked-list representation

*/

/* if the code is well written, you don't need comments.
   Good code documents itself!
*/
#define THIS_IS_LISP   /* make sure the silly types.h file doesn't interfere */

#include <memory.h>
#include <misctool.h>

/*
        Driver.
*/
#include "gscheme.h"
#include "misc.h"
#include "evaler.h"
#include "prim.h"
#include "parser.h"
#include "lexer.h"
#include "printer.h"
#include "garbage.h"
#include "stack.h"

extern FILE *INPsrc;
struct envstruct *glob_env;
int ERROR_FLAG;

void centerprint(char *,int);
int loop(void);

/*
        I obtain an initial environment, then prompt for,
        read in, and evaluate Scheme expressions.
*/
int main(int argc,char *argv[])
{
       INPsrc = stdin;
       ERROR_FLAG = 0;
       InitMem(65500);         /* initial memory bank size */
       glob_env = InitialEnv();
       putchar(12); /* clear the screen */
       centerprint("GScheme",80);
       centerprint("Beta Version 0.4b 6/12/89",80);
       centerprint("Copyright 1988,9 Procyon Software",80);

       printf("\nGScheme 1.0\nTotal Memory: %dk\n",(TotalMem() / 1024));
       printf("Free Memory: %dk\n\n",(MaxBlock() / 1024));
       INPsrc = fopen("SCHEME.LOGIN", "r");
       loop();
       printf(";EXIT\n");
}

int loop(void)
{
struct datumstruct *result;
struct procname *prcn;

       while (1) {
       ClearStack(&GLOB_proc);

       prcn = NEW(procname);
       prcn->named = Top;
       Push(prcn,&GLOB_proc);

       printf("==> ");
       result = EvalExp(HandleExp(),glob_env);
       if (ERROR_FLAG)
       {
               printf("\n GScheme Reset \n");
               ERROR_FLAG = 0;
               ClearStack(&evalstack); /* reset the eval stack */
               ClearStack(&GLOB_procptr);
               ClearStack(&GLOB_proc);
               GCstack = GCbottom; /* reset the Garbage collection stack */
       }
       else Print(result);
       putchar('\n');
       if (evalstack != 0) printf("Warning: stack corrupted\n");
/*     glob_env = garbage(glob_env,0);   */
       }
}

void centerprint(char *string,int width)
{

int i,l = strlen(string);
       for (i = 1; i < (width - l) / 2; i++)
         putchar(' ');
       puts(string);
}
