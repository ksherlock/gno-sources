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

#include "gscheme.h"
#include <sane.h>
#include <misctool.h>
#include <locator.h>
#include <time.h>

/*
        Primitives.

      All routines that are called directly as Scheme primitives
      start with a p_ prefix, like p_list etc...

*/

#include "misc.h"
#include "prim.h"
#include "evaler.h"
#include "parser.h"
#include "stack.h"
#include "garbage.h"

struct datumstruct *DoSymbol(); /* an interface to the parser */

int no_args;   /* global only to the Prim.C module */
extern struct envstruct *glob_env;
extern int ERROR_FLAG;
struct envstruct *primenv;

/*
        I apply a primitive.
*/
void
ApplyPrim(funct,num_args,primenviron)
int (*funct)();
int num_args;
struct envstruct *primenviron;
{
       primenv = primenviron;
       no_args = num_args;
       (*funct)();
}


/* we can assume we won't run out of memory when binding primitives.
   So we don't do a garbagey gcpush,gcpop                              */

void
AddPrim(primname, funct, env)
char *primname;
int (*funct)();
struct envstruct *env;
{
struct binding *bnd;

       bnd = NEW(binding);
       bnd->variable = primname;
       bnd->value = NEW(datumstruct);
       bnd->next = env->bindings;
       bnd->value->tag = Primitive;
       bnd->value->data.prm.primitive = primname;
       bnd->value->data.prm.funct = funct;
       env->bindings = bnd;
}

struct datumstruct *
AddBind(bindname, env)
char *bindname;
struct envstruct *env;
{
struct binding *bnd;
       bnd = NEW(binding);
       bnd->variable = bindname;
       bnd->value = NEW(datumstruct);
       bnd->next = env->bindings;
       env->bindings = bnd;
       return bnd->value;
}

GetTwo(a,b)
struct datumstruct **a,**b;
{
        *a = ePop();
        *b = ePop();
}

ChkNum(a,b)
struct datumstruct *a,*b;
{
        if ((a->tag != Literal) || (b->tag != Literal))
           Fatal("non-numeric type in binary procedure");
}

PopChk(a)
struct datumstruct **a;
{
       if ((*a = ePop())->tag != Literal)
         Fatal("non-numeric type in arithmetic");
}

p_plus()
{
struct datumstruct *a;
int i;
double s = 0.0;
       for (i = 1; i <= no_args; i++)
       {
               PopChk(&a);
               s = s + a->data.literal;
       }
       pushconstant(s);
}

p_minus()
{
struct datumstruct *a,*b;
       if (no_args != 2) Fatal("wrong # of args in binary proc");
       GetTwo(&a,&b); ChkNum(a,b);
       pushconstant(a->data.literal - b->data.literal);
}

p_mult()
{
struct datumstruct *a,*b;
double s;
int i;
       s = 1.0;
       for (i = 1; i <= no_args; i++)
       {
               PopChk(&a);
               s = s * a->data.literal;
       }
       pushconstant(s);
}

p_div()
{
struct datumstruct *a,*b;
       if (no_args != 2) Fatal("wrong # of args in binary proc");
       GetTwo(&a,&b); ChkNum(a,b);
       pushconstant(a->data.literal / b->data.literal);
}

p_equal()
{
struct datumstruct *a,*b;
        GetTwo(&a,&b); ChkNum(a,b);
        if (a->data.literal == b->data.literal) pushtrue();
        else pushnil();
}

p_grthan()
{
struct datumstruct *a,*b;
        GetTwo(&a,&b); ChkNum(a,b);
        if (a->data.literal > b->data.literal) pushtrue();
        else pushnil();
}

p_jawaid()
{
       printf("Jawaid is one hell of a guy\n");
       pushnil();
}

p_lessthan()
{
struct datumstruct *a,*b;
        GetTwo(&a,&b); ChkNum(a,b);
        if (a->data.literal < b->data.literal) pushtrue();
        else pushnil();
}

p_and()
{
struct datumstruct *a,*b;
        GetTwo(&a,&b);
        if ((a->tag == Nilval) || (b->tag == Nilval))
          pushnil();
        else pushtrue();
}

p_or()
{
struct datumstruct *a,*b;
        GetTwo(&a,&b);
        if ((a->tag == Nilval) && (b->tag == Nilval))
          pushnil();
        else pushtrue();
}

p_not()
{
struct datumstruct *a;
        a = ePop();
        if (a->tag == Nilval) pushtrue();
        else pushnil();
}

p_exit()
{
        printf("\n");
        exit(0);
}

struct datumstruct *
d_cons(a,b)
struct datumstruct *a,*b;
{
struct datumstruct *cell;

       GCPush(&a,GCdatumstruct);
       GCPush(&b,GCdatumstruct);
       cell = NEW(datumstruct);
       cell->tag = Pair;
       cell->data.pair.car = b; /* we must reverse these cause APW C evals */
       cell->data.pair.cdr = a; /* arguments in reverse order    */
       GCPop(); GCPop();
       return cell;
}

p_cons()
{
       ePush(d_cons(ePop(),ePop()));
            /* put the cons cell on the stack */
}

/* *****************************************************

         Here comes all the Car and Cdr junk
         Finally implemented as Primitives

   ***************************************************** */

struct datumstruct *
Car(cell)
struct datumstruct *cell;
{
       if (cell->tag != Pair) Fatal("Non-pair in car");
       else return cell->data.pair.car;
}

struct datumstruct *
Cdr(cell)
struct datumstruct *cell;
{
       if (cell->tag != Pair) Fatal("Non-pair in cdr");
       else return cell->data.pair.cdr;
}

p_car()
{
struct datumstruct *cell;

       cell = ePop();
       ePush(Car(cell));
}

p_cdr()
{
struct datumstruct *cell;

       cell = ePop();
       ePush(Cdr(cell));
}

p_cadr()
{
struct datumstruct *cell;

       cell = ePop();
       ePush(Car(Cdr(cell)));
}

p_caar()
{
struct datumstruct *cell;

       cell = ePop();
       ePush(Car(Car(cell)));
}

p_cdar()
{
struct datumstruct *cell;

       cell = ePop();
       ePush(Cdr(Car(cell)));
}

p_cddr()
{
struct datumstruct *cell;

       cell = ePop();
       ePush(Cdr(Cdr(cell)));
}

p_caddr()
{
struct datumstruct *cell;

       cell = ePop();
       ePush(Car(Cdr(Cdr(cell))));
}
p_cadddr()
{
struct datumstruct *cell;

       cell = ePop();
       ePush(Car(Cdr(Cdr(Cdr(cell)))));
}

p_cdadr()
{
struct datumstruct *cell;

       cell = ePop();
       ePush(Cdr(Car(Cdr(cell))));
}

/* *****************************************************

       These are a bunch of functions to do testing of
       certain things.  Included are type conditionals
       and the EQ? EQUAL? set.

   ***************************************************** */

p_nullq()
{
struct datumstruct *a;
       a = ePop();
       if (a->tag != Nilval) pushnil();
       else pushtrue();
}

p_symbolq()
{
struct datumstruct *a;
       a = ePop();
       if (a->tag != Symbol) pushnil();
       else pushtrue();
}

p_numberq()
{
struct datumstruct *a;
       a = ePop();
       if (a->tag != Literal) pushnil();
       else pushtrue();
}

p_atomq()
{
struct datumstruct *a;
       a = ePop();
       if (a->tag != Pair) pushtrue();
       else pushnil();
}

p_pairq()
{
struct datumstruct *a;
       a = ePop();
       if (a->tag == Pair) pushtrue();
       else pushnil();
}


p_eqq()
{
struct datumstruct *a,*b;
       GetTwo(&a,&b);
       if (d_eqq(a,b)) pushtrue();
       else pushnil();
}

d_eqq(a,b)
struct datumstruct *a,*b;
{
       if (a->tag != b->tag)
          return 0;
       else
       switch (a->tag)
       {
         case Literal:         if (a->data.literal == b->data.literal)
                                 return 1;
                               else return 0;
                               break;
         case dString:
         case Symbol:          if (!strcmp(a->data.symbol,b->data.symbol))
                                 return 1;
                               else return 0;
                               break;
         case TrueVal:
         case Nilval:          return 1; break;
         default:              Fatal("invalid type in EQ?"); break;
       }
}

d_equalq(a,b)
struct datumstruct *a,*b;
{
       if (a->tag != b->tag) return 0;
       if (a->tag == Pair)
       {
          return (d_equalq(a->data.pair.car, b->data.pair.car) &&
                  d_equalq(a->data.pair.cdr, b->data.pair.cdr));
       }
       return d_eqq(a,b);
}

p_equalq()
{
struct datumstruct *a,*b;
       a = ePop(); b = ePop();
       if (d_equalq(a,b)) pushtrue();
       else pushnil();
}

p_display()
{
void Print();
struct datumstruct *a;
       a = ePop();
       if (a->tag == dString)
         printf("%s",a->data.symbol);
       else Print(a);
       pushtrue();
}

p_print()
{
       p_display();
       putchar('\n');
}

p_error()
{
int i;
       printf("\nerror: ");
       for (i = 0; i < no_args; i++)
          Print(ePop());
       putchar('\n');
       ERROR_FLAG = 1;
}

p_newline()
{
       putchar('\n');
       pushtrue();
}


/* **********************************************************

    List handling and creation

   ********************************************************** */

struct datumstruct *
makelist(num)
int num;
{
struct datumstruct *newl = 0;
       if (num == 0)
       {
               newl = NEW(datumstruct);
               newl->tag = Nilval;
               return newl;
       }
       else
       {
               GCPush(&newl,GCdatumstruct);
               newl = NEW(datumstruct);
               newl->tag = Pair;
               newl->data.pair.car = ePop();
               newl->data.pair.cdr = makelist(num-1);
               GCPop();
               return newl;
       }
}

p_list()
{
       ePush(makelist(no_args));
}

struct datumstruct *
append(b,a)
struct datumstruct *b,*a;
{
struct datumstruct *n;
       GCPush(&a,GCdatumstruct);
       GCPush(&b,GCdatumstruct);
       n = NEW(datumstruct);
       n->tag = Pair;
       if (a->tag == Nilval) n = b;
       else
       {
         n->data.pair.car = a->data.pair.car;
         n->data.pair.cdr = append(b,a->data.pair.cdr);
       }
       GCPop(); GCPop();
       return n;
}

p_append()
{
       ePush(append(ePop(),ePop()));
}

p_length()
{
int i = 0;
struct datumstruct *a;
       a = ePop();
       while (a->tag != Nilval)
       {
          i++;
          a = a->data.pair.cdr;
       }
       pushconstant((double) i);
}

d_assq(compare)
int (*compare)();
{
struct datumstruct *key,*recs;
       key = ePop(); recs = ePop();
       while (recs->tag != Nilval)
       {
            if ((*compare)(key, Car(Car(recs))))
            {
               ePush(Car(recs));
               return;
            }
            recs = Cdr(recs);
       }
       pushnil();
}

p_assq()
{
       d_assq(d_eqq);
}

p_assoc()
{
       d_assq(d_equalq);
}

p_memq()
{
struct datumstruct *item,*x;

       item = ePop(); x = ePop();
       while (x->tag != Nilval)
       {
           if (d_eqq(item, Car(x)))
           {
              ePush(x);
              return;
           }
           x = Cdr(x);
       }
       pushnil();
}

struct datumstruct *
d_mapcar(proc,x)
struct datumstruct *proc,*x;
{
extern void doApply();

       if (x->tag == Nilval) {
           pushnil();
           return (struct datumstruct *) ePop();  /* make a nilvalue */
       }
       else {
           ePush(Car(x));
           ePush(proc); /* push the procedure on the stack for apply */
           doApply(1,primenv);
           return d_cons(ePop(),d_mapcar(proc,Cdr(x)));
       }
}

p_mapcar()
{
struct datumstruct *proc,*x;
       proc = ePop(); x = ePop();
       ePush(d_mapcar(proc,x));
}

p_setcarb()
{
struct datumstruct *a,*b;
       a = ePop(); b = ePop();
       a->data.pair.car = b;
       ePush(b);
}

p_setcdrb()
{
struct datumstruct *a,*b;
       a = ePop(); b = ePop();
       a->data.pair.cdr = b;
       ePush(b);
}

/* *****************************************************

     Some miscellaneous procedures.  These handle everything
      from I/O redirection to garbage collection

   ***************************************************** */

p_fre()
{
struct datumstruct *a;
double b;

       if ((a = ePop())->tag != Literal)
               Fatal("invalid arg-fre");
       switch ((int) a->data.literal)
       {
               case 0: b = (double) freemem();
                       pushconstant(b);
                       break;
               case 1: glob_env = garbage(glob_env,1);
                       /* the 1 means force collection anyway */
                       b = (double) freemem();
                       pushconstant(b);
                       break;
               default:Fatal("invalid arg value-fre");
        }
}

p_read()
{
       ePush(DoSymbol());
}

p_runtime()
{
       pushconstant((double) GetTick());
}

/* the editing and disk access features */

p_load()
{
extern FILE *INPsrc;
struct datumstruct *a;
       a = ePop();
       INPsrc = fopen(a->data.symbol, "r");
       pushtrue();
}

p_edit()
{
handle TextStateRec;
struct datumstruct *a;
char *s,*t;   

       if ((a = ePop())->tag != dString)
         Fatal("non-numeric type in arithmetic");
       s = a->data.symbol;
       t = malloc(strlen(s)+6);
       strcpy(t,"edit ");
       strcat(t,s);
       TextStateRec = SaveTextState();
       system(t);
       RestoreTextState(TextStateRec);
       free(t);
       pushtrue();
}

p_force()
{
extern ReverseEval();
struct datumstruct *a;

       a = ePop(); /* the delay object- really a pair */
       GCPush(&a,GCdatumstruct);
       if (a->data.pair.car != 0) ePush(a->data.pair.car);
         else
       ReverseEval(a->data.pair.cdr,primenv);
       a->data.pair.car = STACKTOP(evalstack);
       GCPop();
}

p_tail()
{
struct datumstruct *a;
       a = ePop(); /* the cons-pair */
       ePush(a->data.pair.cdr);
       p_force();
}

p_eval()
{
struct datumstruct *code,*env;
extern int RegOrDat;
struct treenode *temp;

       code = ePop();
       env = ePop();
       RegOrDat = 1;
       ClearStack(&convlex);
       GCPush(&code,GCdatumstruct);
       GCPush(&env,GCdatumstruct);
       Push(code,&convlex);
       temp = HandleExp();
       GCPop(); GCPop();
       ePush(EvalExp(temp,env->data.Environ));
       RegOrDat = 0;
}

/* *************************************************************

       The following prims take care of what we need Vectors
       (arrays) to do.

   ************************************************************* */

p_makevector()
{
struct datumstruct *a,**b;
int size;
       PopChk(&a);
       size = (int) a->data.literal;
       a = NEW(datumstruct);
       GCPush(&a,GCdatumstruct);
       b = (struct datumstruct **)
             NAlloc(size * sizeof(struct datumstruct *));
       a->tag = Vector;
       a->data.vector.size = size;
       a->data.vector.vect0 = b;
       GCPop();
       ePush(a);
}

p_vectorsetb()
{
struct datumstruct *index,*vector,**vect,*exprn;
       vector = ePop(); index = ePop(); exprn = ePop();
       vect = vector->data.vector.vect0 + (int) index->data.literal;
       *vect = exprn;
       ePush(exprn);
}

p_vectorref()
{
struct datumstruct *a,*b;
       a = ePop(); b = ePop();
       ePush(*(a->data.vector.vect0 + (int) b->data.literal));
}

p_vectorlength()
{
struct datumstruct *a;
       a = ePop();
       pushconstant((double) a->data.vector.size);
}

p_vectorq()
{
struct datumstruct *a;
       a = ePop();
       if (a->tag == Vector) pushtrue();
       else pushnil();
}


/* *********************************************************
   *********************************************************

       GScheme Mathpack

       square          atan
       sqrt            random
       sin             exp
       cos             abs
       tan

   *********************************************************
   ********************************************************* */

p_square()
{
struct datumstruct *a;
       PopChk(&a);
       pushconstant(a->data.literal * a->data.literal);
}

p_abs()
{
struct datumstruct *a;
       PopChk(&a);
       (a->data.literal < 0) ? pushconstant(-a->data.literal) :
                               pushconstant(a->data.literal);
}

p_exp()
{
struct datumstruct *a,*b;
int iter,i;
double crnt;
       PopChk(&b);  PopChk(&a);
       iter = (int) a->data.literal;
       crnt = 1.0;
       for (i = 1; i <= iter; i++)
          crnt = crnt * b->data.literal;
       pushconstant(crnt);
}


double r;  /* result for many sane-type operations */

p_sqrt()
{
struct datumstruct *a;
       PopChk(&a);
       r = (double) sqrt( (extended) a->data.literal);
       pushconstant(r);
}

p_sin()
{
struct datumstruct *a;
       PopChk(&a);
       r = (double) sin( (extended) a->data.literal);
       pushconstant(r);
}

p_cos()
{
struct datumstruct *a;
       PopChk(&a);
       r = (double) cos( (extended) a->data.literal);
       pushconstant(r);
}

p_tan()
{
struct datumstruct *a;
       PopChk(&a);
       r = (double) tan( (extended) a->data.literal);
       pushconstant(r);
}

p_atan()
{
struct datumstruct *a;
       PopChk(&a);
       r = (double) atan( (extended) a->data.literal);
       pushconstant(r);
}

p_random()
{
struct datumstruct *a;
       PopChk(&a);
       r = ((double) rand()) / 32767.0;
       pushconstant(r);
}

p_max()
{
struct datumstruct *a,*b;
       GetTwo(&a,&b);
       if (a->data.literal >= b->data.literal)
          ePush(a);
       else ePush(b);
}

p_min()
{
struct datumstruct *a,*b;
       GetTwo(&a,&b);
       if (a->data.literal <= b->data.literal)
          ePush(a);
       else ePush(b);
}

p_1plus()
{
struct datumstruct *a;
       PopChk(&a);
       pushconstant(a->data.literal + 1.0);
}

/* **************************************************************

           These data tables are used by InitialEnv to
         create the primitive environment.

   ************************************************************** */


char *Prims[] = {
"+",           "-",            "*",
"/",           "exit",         "=",
"and",         "not",          "or",
">",           "<",            "cons",
"car",         "cdr",          "jawaid",
"newline",     "null?",        "symbol?",
"list",        "eq?",          "fre",
"number?",     "atom?",        "display",
"append",      "length",       "set-car!",
"set-cdr!",    "read",         "equal?",
"runtime",     "abs",          "princ",
"print",       "error",
"load",        "edit",
"make-vector", "vector-ref",   "vector-set!",
"vector-length","vector?",
"eval",

"force",       "head",         "tail",

"square",      "^",            "sqrt",
"sin",         "cos",          "tan",
"atan",        "random",       "1+",
"max",         "min",

"cadr",        "caar",         "cdar",
"cddr",        "caddr",        "cadddr",
"cdadr",       "pair?",

"assq",        "assoc",        "mapcar",
"map",         "memq",
"endofprims" };

int (*PrimFcts[])() = {
p_plus,        p_minus,        p_mult,
p_div,         p_exit,         p_equal,
p_and,         p_not,          p_or,
p_grthan,      p_lessthan,     p_cons,
p_car,         p_cdr,          p_jawaid,
p_newline,     p_nullq,        p_symbolq,
p_list,        p_eqq,          p_fre,
p_numberq,     p_atomq,        p_display,
p_append,      p_length,       p_setcarb,
p_setcdrb,     p_read,         p_equalq,
p_runtime,     p_abs,          p_display,
p_print,       p_error,
p_load,        p_edit,
p_makevector,  p_vectorref,    p_vectorsetb,
p_vectorlength,p_vectorq,

/* har har har har har... */
p_eval,
/*  Delayed Evaluation Helpers  */

p_force,       p_car,          p_tail,

/*  GScheme MathPack   */

p_square,      p_exp,          p_sqrt,
p_sin,         p_cos,          p_tan,
p_atan,        p_random,       p_1plus,
p_max,         p_min,

/* Cadrs */
p_cadr,        p_caar,         p_cdar,
p_cddr,        p_caddr,        p_cadddr,
p_cdadr,       p_pairq,
p_assq,        p_assoc,        p_mapcar,
p_mapcar,      p_memq
};


/* ***************************************************************

        I return an environment containing only the primitives.

   *************************************************************** */

/* again, assume we won't run out of memory to save me drudgery */
struct envstruct *
InitialEnv()
{
struct envstruct *ep;
int i = 0;
struct datumstruct *a;
time_t t;
struct tm trec;

       /* Initialize the random number generator */
       t = time(NULL);
       trec = *localtime(&t);
       srand(trec.tm_sec);

       ep = NEW(envstruct);
       ep->prevenv = END_ENVS;
       ep->bindings = END_BINDINGS;
       while (strcmp("endofprims",Prims[i]))
       {
               AddPrim(Prims[i],PrimFcts[i],ep);
               i++;
       }
       AddBind("nil",ep)->tag = Nilval;
       a = AddBind("else",ep);
       a->tag = Literal; a->data.literal = 0;
       a = AddBind("user-initial-environment",ep);
       a->tag = EnvironPtr; a->data.Environ = ep;
       a = AddBind("#t",ep);
       a->tag = TrueVal;
       a = AddBind("t",ep);
       a->tag = TrueVal;

       return ep;
}
