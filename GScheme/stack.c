/*   The Stack stuff  */

#include "gscheme.h"

struct stack
{
   struct datumstruct *data;
   struct stack *next;
};

struct stack *evalstack = 0;
struct stack *GLOB_proc = 0;
struct stack *GLOB_procptr = 0;
struct stack *convlex = 0;
extern int ERROR_FLAG;

void *GCstack;
void *GCbottom;

void
Push(dp,stackptr)
struct datumstruct *dp;
struct stack **stackptr;
{
char *malloc();
struct stack *st;
       st = (struct stack *) malloc(sizeof(struct stack));
       st->data = dp;
       st->next = *stackptr;
       *stackptr = st;
}

struct datumstruct *
Pop(stackptr)
struct stack **stackptr;
{
struct stack *r = *stackptr;
struct datumstruct *d;
       if (ERROR_FLAG) return; /* just ignore this if we're in an error */
       if (r == 0) Fatal("Stack Underflow");
       *stackptr = (*stackptr)->next;
       d = r->data;
       free(r);
       return d;
}

void
ClearStack(stackptr)
struct stack **stackptr;
{
struct stack *d;
       d = *stackptr;
       while (d != 0)
       {
           free(d);
           d = d->next;
       }
       *stackptr = 0;
}

struct datumstruct *
STACKTOP(stackptr)
struct stack *stackptr;
{
       if (stackptr == 0) return 0;
       else return stackptr->data;
}

struct datumstruct *
ePop()
{
       return Pop(&evalstack);
}

void
ePush(datum)
struct datumstruct *datum;
{
       Push(datum, &evalstack);
}
