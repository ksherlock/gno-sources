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
segment "evaler    ";

/*
        Evaluator.
*/
#define THIS_IS_LISP

#include "gscheme.h"
#include "evaler.h"
#include "parser.h"
#include "prim.h"
#include "misc.h"
#include "stack.h"

extern int ERROR_FLAG;
extern char *mem[2];
extern int curbnk;
struct treenode *crnt_tail;
int TAIL = 0;
/*
        I evaluate an expression tree, and return the result.
        I'm just a convenient interface to Evaluate(), which
        does the work.
*/
struct datumstruct *
EvalExp(treep, envp)
struct treenode *treep;
struct envstruct *envp;
{
        struct datumstruct *d;
        void Evaluate();

        Evaluate(treep, envp);
        return ePop();
}

/*
        I evaluate an expression tree, pushing things on
        the stack as I go.
*/
static
void
Evaluate(treep, envp)
struct treenode *treep;
struct envstruct *envp;
{
struct explist *tmp;
void doApply();
void dobind();
struct datumstruct *newp,*newp2;
struct procname *prcn;

       asm {
       lda 0xE0C000
       bpl dontstop
       sta 0xE0C010
       and #0x00ff
       cmp #0xAE
       beq intrupt
       cmp #0x9B
       bne dontstop
       bra debugcall
intrupt: lda 0xE0C025
       and #0x80
       beq dontstop
       lda #0x1
       sta ERROR_FLAG
       bra dontstop

       }
debugcall: debug();
dontstop:
       if (ERROR_FLAG) return;
       GCPush(&envp,GCenvstruct);
       GCPush(&treep,GCtreenode);
       if (treep == (struct treenode *)STACKTOP(GLOB_procptr))
       {
               TAIL = 1;       /* har har we're tail recursing */
               GCPop(); GCPop();
               return;
       }
       switch(treep->tag) {
        case qString:  newp = NEW(datumstruct);
                       newp->tag = dString;
                       newp->data.symbol = treep->data.identifier;
                       ePush(newp);
                       break;
        case qSymbol:  ePush(treep->data.symbol);
                       break;
        case Constant: pushconstant(treep->data.constant);
                       break;
        case Identifier:lookup(treep->data.identifier, envp); break;
        case Emptylist: pushnil(); break;
        case Bind:     Evaluate(treep->data.bind.rvalue, envp);
          /* if this is getting bound to a procedure and the parser found
             a tail and if the tail is a call to this binding then we
             leave it alone, otherwise we NIL it.  */
                       if (ERROR_FLAG) break;
                       if (STACKTOP(evalstack)->tag == Closure)

                        if (treep->data.bind.rvalue->data.function.tail != 0)
                         if (strcmp(treep->data.bind.rvalue->data.function.tail
                                         ->data.apply.operator
                                         ->data.identifier,
                                     treep->data.bind.lvalue))
                            treep->data.bind.rvalue->data.function.tail = 0;
                       /*else puts("proc is tail recursive");  */
                       findbinding(treep->data.bind.lvalue, envp);
                       dobind(); break;
        case Assign:   Evaluate(treep->data.bind.rvalue, envp);
                       locallookup(treep->data.bind.lvalue, envp);
                       dobind(); ePop(); ePush(0); break;
        case Apply:    ExpLEval(treep->data.apply.operands, envp);
                       Evaluate(treep->data.apply.operator, envp);
                       if (ERROR_FLAG) break;
                       prcn = NEW(procname);
                       prcn->lambd = treep->data.apply.operator;
                       if (treep->data.apply.operator->tag == Identifier)
                           prcn->named = Yes;
                       else
                           prcn->named = No;
                       Push(prcn,&GLOB_proc);
                       Push(STACKTOP(evalstack)->data.closure.function
                                               ->data.function.tail,
                            &GLOB_procptr);
                       doApply(ExpLLength(treep->data.apply.operands),envp);
                       Pop(&GLOB_proc);
                       Pop(&GLOB_procptr);
                       break;
        case Sequence: ReverseEval(treep->data.sequence,envp);
                       /* we obviously don't need a IF (ERROR_FLAG) here */
                       break;
        case Cond:     Evaluate(treep->data.cond.condpart, envp);
                       if (ERROR_FLAG) break;
                       if (STACKTOP(evalstack)->tag == Nilval)
                       {
                          ePop();
                          Evaluate(treep->data.cond.elsepart,envp);
                       /* no IF ERROR_FLAG needed */
                       }
                       else
                       {
                          ePop();
                          Evaluate(treep->data.cond.thenpart, envp);
                         /* same here- no IF ERRORFLAG needed */
                       }
                       break;
        case RealCond: EvalCond(treep,envp); break;

        case Delay:    newp = NEW(datumstruct);
                       newp->tag = Delay_Pair;
                       newp->data.pair.car = 0;
                       newp->data.pair.cdr =
                         (struct datumstruct *) treep->data.delay.delayed;
                       ePush(newp);
                       break;
        case ConsStream:
                       newp = NEW(datumstruct);
                       newp->tag = Delay_Pair;
                       newp->data.pair.car = 0;
                       newp->data.pair.cdr =
                         (struct datumstruct *) treep->data.st_pair.st_cdr;
                       GCPush(&newp,GCdatumstruct);

                       Evaluate(treep->data.st_pair.st_car,envp);
                       newp2 = NEW(datumstruct);
                       newp2->tag = Pair;
                       newp2->data.pair.car = ePop();
                       newp2->data.pair.cdr = newp;
                       ePush(newp2);
                       GCPop();
                       break;
        case MakeEnv:  newp = NEW(datumstruct);
                       newp->tag = EnvironPtr;
                       GCPush(&newp,GCdatumstruct);
                       newp->data.Environ = NEW(envstruct);
                       newp->data.Environ->prevenv = envp;
                       newp->data.Environ->bindings = 0; /* just for kicks */
                       ExpLEval(treep->data.MakeEnvBody,newp->data.Environ);
                       ePop();
                       ePush(newp);
                       GCPop();
                       break;
        case Function:
                       newp = NEW(datumstruct);
                       newp->tag = Closure;
                       newp->data.closure.env = envp;
                       newp->data.closure.function = treep;
                       /*printf("Closure env = %x\n",envp); */
                       ePush(newp);
                       break;
        default:       printf("illegal tree tag<%d>-eval",treep->tag);
                       Fatal("");
        }
        GCPop(); GCPop();
}

EvalCond(tree,envp)
struct treenode *tree;
struct envstruct *envp;
{
struct explist *expl;
struct expLL *expll;
       expl = tree->data.Realcond.conds;
       expll = tree->data.Realcond.results;

       GCPush(&envp,GCenvstruct);
       GCPush(&expl,GCexplist);
       GCPush(&expll,GCexpLL);
       while (expl != END_EXPLIST)
       {
               Evaluate(expl->thisexp,envp);
               if (ERROR_FLAG) return;
               if (ePop()->tag != Nilval)
               {
                       ReverseEval(expll->thisexpl,envp);
                       if (ERROR_FLAG) return;
                       GCPop(); GCPop(); GCPop();
                       return;
               }
               else
               {
                       expl = expl->next;
                       expll = expll->next;
               }
       }
       GCPop(); GCPop(); GCPop();
       pushnil();
}

ExpLEval(currexp, envp)
struct explist *currexp;
struct envsruct *envp;
{
       GCPush(&currexp,GCexplist);
       GCPush(&envp,GCenvstruct);
       while (currexp != END_EXPLIST)
       {
               Evaluate(currexp->thisexp, envp);
               currexp = currexp->next;
       }
       GCPop(); GCPop();
}

int ExpLLength(curexp)
struct explist *curexp;
{
int r = 0;
       while (curexp != END_EXPLIST)
       {
               curexp = curexp->next;
               r++;
       }
       return r;
}

void
doApply(num_args,envp)
int num_args;
struct envstruct *envp;
{
struct datumstruct *op;
struct explist *curexpr;
struct arglist *curargs;
struct binding *tmp;
unsigned int stp;
void ePush();
struct datumstruct *makelist();

       op = ePop();
       GCPush(&op,GCdatumstruct);
       if (op->tag == Primitive)
       {
               ApplyPrim(op->data.prm.funct, num_args, envp);
               GCPop();
               return;
       }
       if (op->tag == Delay_Pair)
       {
               if (op->data.pair.car != 0)
                 ePush(op->data.pair.car);
               else
               {
                 ReverseEval(op->data.pair.cdr,envp);
                 op->data.pair.car = STACKTOP(evalstack);
               }
               GCPop();
               return;
       }
       if (op->tag != Closure){ printf(" %x,%lx-\n",op->tag,op);
                                Fatal("invalid apply type!"); }

       envp = NEW(envstruct); envp->bindings = END_BINDINGS;
       envp->prevenv = op->data.closure.env;
       curargs = op->data.closure.function->data.function.arguments;
       curexpr = op->data.closure.function->data.function.body;
       GCPush(&envp,GCenvstruct);
       GCPush(&curexpr,GCexplist);
       GCPush(&curargs,GCarglist);
       if (op->data.closure.function->data.function.atype == PackInList)
          ePush(makelist(num_args));
       MakeProcBindings(curargs, envp);

     do {
       asm {
       tsc
       sta stp
       }
     /* printf("(sp) %x, %lx (es), %lx (gps)\n",stp,evalstack,GLOB_procptr);*/
     /* printf("%lx (mem)\n",mem[curbnk]);  */
       TAIL = 0; /* turn the TAILRECURSE flag off */
       ReverseEval(curexpr, envp);
 /*    printf("GCstack size = %d ",GCstack - GCbottom);
       printf("GCtop = %lx, GCtop-1 = %lx / ",GCstack-1,GCstack-2);  */
       if (TAIL != 0)
       {
         ExpLEval(((struct treenode *) STACKTOP(GLOB_procptr))->
           data.apply.operands,envp);
         MakeTailBindings(curargs,envp);
       }
    } while (TAIL == 1);
    GCPop(); GCPop(); GCPop(); GCPop();
}

struct binding *helptail;

MakeTailBindings(args, envp)
struct arglist *args;
struct envstruct *envp;
{
       if (args != END_ARGLIST)
       {
               MakeTailBindings(args->next,envp);
               locallookup(args->arg,envp);      /* rebind all the args with */
               helptail = (struct binding *) ePop(); /* new values */
               helptail->value = ePop();
       }
}

MakeProcBindings(args, envp)
struct arglist *args;
struct envstruct *envp;
{
struct binding *bind;
       if (args != END_ARGLIST)
       {
               MakeProcBindings(args->next, envp);
               GCPush(&args,GCarglist);
               GCPush(&envp,GCenvstruct);
               bind = NEW(binding);
               GCPop(); GCPop();
               bind->value = ePop();
               bind->variable = args->arg;
               bind->next = envp->bindings;
               envp->bindings = bind;
            /*   args = args->next;     */
       }
}

/* this is the only noteworthy routine from mp3
*/

ReverseEval(exprs, envp)
struct explist *exprs;
struct envstruct *envp;
{
       if (exprs == END_EXPLIST)
               ePush(0); /* was pushnil, caused excess memory usage */
                /* all this is for is to balance the impending Pop
                   below when we start reversing the recursion */
       else
       {
               ReverseEval(exprs->next, envp);
               ePop();
               GCPush(&exprs,GCexplist);
               GCPush(&envp,GCenvstruct);
               Evaluate(exprs->thisexp, envp);
               GCPop(); GCPop();
       }
}

void
dobind()
{
struct binding *dlc;
struct datumstruct *t;

        t = NEW(datumstruct);
        dlc =(struct binding *) ePop();
        dlc->value = ePop();
        t->tag = Symbol;
        t->data.symbol = dlc->variable;
        ePush(t);
}

lookup(ident, envp)
char *ident;
struct envstruct *envp;
{
struct binding *bnd;
struct envstruct *env;

        env = envp;
        while (env != END_ENVS)
        {
          bnd = env->bindings;
          while (bnd != END_BINDINGS)
          {
            if (strcmp(ident, bnd->variable) == 0)
            {
              ePush(bnd->value); return;
            }
            bnd = bnd->next;
          }
          env = env->prevenv;
        }
        printf("\n%s not bound\n",ident);
        Fatal("unbound variable");
}

findbinding(ident, envp)
char *ident;
struct envstruct *envp;
{
struct binding *bnd;

        bnd = envp->bindings;
        while (bnd != END_BINDINGS)
        {
          if (!strcmp(ident, bnd->variable))
          {
            ePush(bnd); return;
          }
          bnd = bnd->next;
        }
        GCPush(&ident,GCstring);
        GCPush(&envp,GCenvstruct);
        bnd = NEW(binding);
        bnd->variable = ident;
        bnd->next = envp->bindings;
        envp->bindings = bnd;
        ePush((struct binding *) bnd); /* strange stack mess */
        GCPop(); GCPop();
}

locallookup(ident, envp)
char *ident;
struct envstruct *envp;
{
struct binding *bnd;

       while (envp != END_ENVS)
       {
         bnd = envp->bindings;
         while (bnd != END_BINDINGS)
         {
               if (!strcmp(ident, bnd->variable))
               {
                 ePush(bnd); return;
               }
               bnd = bnd->next;
         }
       envp = envp->prevenv;
       }
       Fatal("Identifier not found! - set!");
}


pushconstant(value)
double value;
{
struct datumstruct *dp;

        dp = NEW(datumstruct);
        dp->tag = Literal;
        dp->data.literal = value;
        ePush(dp);
}

pushnil()
{
struct datumstruct *dp;
       dp = NEW(datumstruct);
       dp->tag = Nilval;
       ePush(dp);
}

pushtrue()
{
struct datumstruct *dp;
       dp = NEW(datumstruct);
       dp->tag = TrueVal;
       ePush(dp);
}
