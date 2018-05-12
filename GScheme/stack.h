/* global stacks */
/* and stack interface */

struct stack
{
   struct datumstruct *data;
   struct stack *next;
};

extern struct stack *evalstack;
extern struct stack *GLOB_proc;
extern struct stack *GLOB_procptr;
extern struct stack *convlex;

struct datumstruct *Pop(struct stack **);
void *Push(void *,struct stack **);
struct datumstruct *STACKTOP(struct stack *);
struct datumstruct *ePop(void);
void *ePush(void *);
void ClearStack(struct stack **);

typedef long *pntr;
extern pntr *GCstack;
extern pntr *GCbottom;

#define GCPop() GCstack--
#define GCPush(addr,type) *(GCstack++)=(pntr) (type | (long) addr)
#define GCtoken        0x01000000l
#define GCdatumstruct  0x02000000l
#define GCtreenode     0x03000000l
#define GCexplist      0x04000000l
#define GCarglist      0x05000000l
#define GCexpLL        0x06000000l
#define GCenvstruct    0x07000000l
#define GCbinding      0x08000000l
#define GCstring       0x09000000l
