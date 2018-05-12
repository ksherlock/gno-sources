/*
       Miscellaneous.
*/

#include "gscheme.h"
#include <memory.h>
#include <orca.h>
#include "misc.h"
#include "stack.h"
#include "garbage.h"
#include "evaler.h"
#include "parser.h"

char *mem[2],*bnk[2];
int curbnk;
long BANKSIZE;
extern ERROR_FLAG;
extern struct envstruct *glob_env;

void PrintDebugMenu(void);
void put20(char *);

long freemem(void)
{
       return BANKSIZE - (mem[curbnk] - bnk[curbnk]);
}

/*
       I print a message and die.
*/
void
Fatal(char *msg)
{
char s[2];
extern FILE *INPsrc;
extern int RegOrDat;

       printf("error: %s\n", msg);
       asm {
try:   lda 0xE0C000
       bpl try
       lda 0xE0C010
       }
       INPsrc = stdin;   /* if we were loading from disk, reset this */
       RegOrDat = 0;     /* if we had a parsing error while DatumLexing */
       ERROR_FLAG = 1;
/*     printf("\n Procedure: %s",GLOB_proc[proc_top]); */
       debug();
}

int debug(void)
{
char b[20];
int flg = 0;
int i;
struct stack *db;
struct procname *r;

    while (flg == 0)
    {
       printf("\nDEBUG-> ");
       gets(b);
       switch (b[0])
       {
          case 'P':
          case 'p':   db = GLOB_proc;
                      while (db != 0)
                      {
                         r = (struct procname *) db->data;
                         if (r->named == No)
                           puts("unnamed procedure");
                         else if (r->named == Top)
                           puts("Top Level Expression");
                         else put20(r->lambd->data.identifier);
                         db = db->next;
                      }
                      break;
          case 'E':
          case 'e':   db = evalstack;
                      while (db != 0)
                      {
                         Print(db->data); putchar('\n');
                         db = db->next;
                      }
                      break;

          case 'Q':
          case 'q':   flg = 1; break;
          case '?':   PrintDebugMenu();
                      break;
       }
    }
}

void PrintDebugMenu(void)
{
       printf("\nP)rint Proc Stack\nQ)uit\n\n");
}

void put20(char *st)
{
int i;
    for (i = 0; i < 20; i++)
    {
       if (st[i] == 0) break;
       else putchar(st[i]);
    }
    putchar('\n');
}

char *
NAlloc(int n)
{
int i;

       /* n = (n+3)/4; n = n*4; */ /* only for stupid 3b2s */

       if ((mem[curbnk]+ (long) n-bnk[curbnk]) > BANKSIZE)
       {
           glob_env = garbage(glob_env,0);
           if ((mem[curbnk]+ (long) n-bnk[curbnk]) > BANKSIZE)
           {
               puts("Fatal error - out of memory");
               exit(1);
           }
       }
       for (i = 0; i < n; *(mem[curbnk]+i++) = 0);
       mem[curbnk] += n;

/*     printf("Alloc %d bytes at %xd\n",n,mem[curbnk]-n);           */
       return mem[curbnk] - n;
}

/*
       I allocate memory.  I check malloc()'s return value
       and die if there was not enough space available.
*/

InitMem(size_t banksize)
{
handle bank1,bank2;
handle memory;

       BANKSIZE = banksize; /* our global variable */
       bank1 = NewHandle(banksize,userid() | 0x0100, attrLocked | attrNoSpec,
                         0L);
       bank2 = NewHandle(banksize,userid() | 0x0100, attrLocked | attrNoSpec,
                         0L);
       if ((bank1 == 0) || (bank2 == 0))
       {
               puts("unrecoverable error- out of memory in InitMem");
               puts("GScheme requires at least 768K");
               exit(1);
       }
       bnk[0] = mem[0] = *bank1;
       bnk[1] = mem[1] = *bank2;
       curbnk = 0;

       memory = NewHandle(16384l,userid() | 0x0100, attrLocked | attrNoSpec,0L);
       GCstack = (void *) *memory;
       GCbottom = (void *) *memory;
}
