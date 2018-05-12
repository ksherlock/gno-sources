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
        Parser.
*/

#define THIS_IS_LISP

#include "gscheme.h"
#include "evaler.h"    /* recent addition for implementation of QUOTE */
#include "emitter.h"
#include "lexer.h"
#include "parser.h"
#include "misc.h"
#include "stack.h"

void Print();
extern int ERROR_FLAG;

struct treenode *last_expr;

struct treenode *MakeList(void);
struct datumstruct *MakeSym(char *txt),*MakeNil(void),*MakeStr(char *),
		   *MakeLit(double);
struct datumstruct *DoSymbol(void);

/*
        I parse an expression.
*/
struct treenode *HandleExp(void)
{
struct token *t;
struct treenode *HandleParen();
struct treenode *HandleFunction();
struct treenode *HandleSymbol();

        if (ERROR_FLAG) return 0;

        t = GetToken();
        switch (t->type)
        {
               case Number:
                       return EmitConstant(t->value.integer);
               case lSymbol:
                       return EmitIdentifier(t->value.symbol);
               case lString:
                       return EmitString(t->value.symbol);
               case Lparen:
                       return HandleParen();
               case Quote:
                       return HandleSymbol();
               default:
                       Fatal("unknown type in HandleExp()");
                       break;
        }
/* not reached */
}

/*
        I parse an expression that started with a left parenthesis.
*/
static
struct treenode *
HandleParen()
{
        struct token *t;
        struct treenode *HandleApply();
        struct treenode *HandleDefine();
        struct treenode *HandleIf();
        struct treenode *HandleAssign();
        struct treenode *HandleFunction(),*HandleCond();
        struct treenode *HandleQuote(),*HandleLet();
        struct treenode *HandleSequence();
        struct treenode *HandleDelay();
        struct treenode *HandleConStream();
        struct treenode *HandleMakeEnv();

        t = GetToken();
        switch (t->type)
        {
                case lSymbol:
                       if (!strcmp(t->value.symbol,"lambda"))
                         return HandleFunction();
                       if (!strcmp(t->value.symbol,"define"))
                         return HandleDefine();
                       if (!strcmp(t->value.symbol,"if"))
                         return HandleIf();
                       if (!strcmp(t->value.symbol,"set!"))
                         return HandleAssign();
                       if (!strcmp(t->value.symbol,"quote"))
                         return HandleQuote();
                       if (!strcmp(t->value.symbol,"let"))
                         return HandleLet();
                       if (!strcmp(t->value.symbol,"cond"))
                         return HandleCond();
                       if (!strcmp(t->value.symbol,"sequence"))
                         return HandleSequence();
                       if (!strcmp(t->value.symbol,"delay"))
                         return HandleDelay();
                       if (!strcmp(t->value.symbol,"cons-stream"))
                         return HandleConStream();
                       if (!strcmp(t->value.symbol,"make-environment"))
                         return HandleMakeEnv();
                       ReturnToken(t);
                       return HandleApply();
                       break;
                case Lparen:
                        ReturnToken(t);
                        return HandleApply();
                        break;
                default:
                        Fatal("unknown type in HandleParen()");
                        break;
        }
        /*NOTREACHED*/
}

/*
        I parse an application.
*/
int first = 0;   /* initialize it  */

static
struct treenode *
HandleApply()
{
struct treenode *tmp;

       struct treenode *op;
       struct explist *args, *HandleExpList();


       first++;
       op = HandleExp();
       GCPush(&op,GCtreenode);
       args = HandleExpList();
       GCPop();
       first--;    /* by god it just might work */
       tmp = EmitApply(op, args);
                     /*   level must be 0 to tail recurse */
       if (first == 0) last_expr = tmp;
       return tmp;
}

/* I parse an If statement */

static struct treenode *
HandleIf()
{
        struct treenode *condition,*thenpart,*elsepart;
        struct token *temp;

        condition = HandleExp();
        GCPush(&condition,GCtreenode);
        thenpart = HandleExp();
        GCPush(&thenpart,GCtreenode);
        temp = GetToken();
        if (temp->type == Rparen)
        {
            GCPop(); GCPop();
            return(EmitCond(condition, thenpart, 0));
        }
        else { GCPush(&elsepart,GCtreenode);
               ReturnToken(temp);
               elsepart = HandleExp();
               GetToken();
               GCPop(); GCPop(); GCPop();
               return(EmitCond(condition, thenpart, elsepart));
             }
}

static struct arglist *
HandleArgs()
{
struct arglist *argL = 0,*arg;
struct token *t;

       GCPush(&argL,GCarglist);
       GCPush(&t,GCtoken);
       argL = END_ARGLIST;
       for (t=GetToken(); t->type != Rparen; t = GetToken())
       {
               arg = NEW(arglist);
               arg->next = argL;
               arg->arg = t->value.symbol;
               argL = arg;
       }
       GCPop(); GCPop();
       return argL;
}


/*
        I parse a list of expressions.  I stop when
        I get to a right parenthesis.
*/
static
struct explist *
HandleExpList()
{
       struct token *t;
       struct explist *exp, *expList = NULL;

       GCPush(&expList,GCexplist);
       expList = END_EXPLIST;
       for (t = GetToken(); t->type != Rparen; t = GetToken())
       {
               ReturnToken(t);
               exp = NEW(explist);
               GCPush(&exp,GCtreenode);
               exp->thisexp = HandleExp();
               GCPop();
               exp->next = expList;
               expList = exp;
       }
       GCPop();
       return expList;
}

static struct treenode *
HandleFunction()
{
struct arglist *args = 0;
struct explist *body = 0;
struct arglist *HandleArgs();
struct token *t;
enum argtype at;
       last_expr = 0;
       t = GetToken(); /* snag the open paren */
       GCPush(&args,GCarglist);
       GCPush(&body,GCexplist);
       if (t->type != Lparen)
       {
           args = NEW(arglist);
           args->next = END_ARGLIST;
           args->arg = t->value.symbol;
           at = PackInList;
       }
       else
       {
           args = HandleArgs();
           at = OneForOne;
       }
       body = HandleExpList();
      /* create a lambda treenode with last_expr pointing to the last
         expression in the lambda statement (i.e. the possible tail
         recurser */
/*     printf("%lx - last_expr\n",last_expr);      */
       GCPop(); GCPop();
       return EmitFunction(at,args,body,last_expr);
}

/* I parse a DEFINE statement   */
static struct treenode *
HandleDefine()
{
struct token *lvalue = 0,*fakelp = 0;
struct treenode *rvalue = 0;

       GCPush(&lvalue,GCtoken);
       GCPush(&rvalue,GCtreenode);
       lvalue = GetToken();
       if (lvalue->type == Lparen)
       {
               lvalue = GetToken();
               fakelp = NEW(token);
               fakelp->type = Lparen;
               ReturnToken(fakelp); /* shove something for HF to eat */
               rvalue = HandleFunction();
               GCPop(); GCPop();
               return EmitBind(lvalue,rvalue);
       }
       else
       {
       rvalue = HandleExp();
       if (GetToken()->type != Rparen) Fatal("syntax error");
       GCPop(); GCPop();
       return EmitBind(lvalue,rvalue);
       }
}

static struct treenode *
HandleAssign()
{
struct treenode *g;

       g =HandleDefine();
       g->tag = Assign;
       return g;
}

static struct treenode *
HandleLet()
{
struct arglist *argL = NULL,*arg;
struct explist *exp,*expList = NULL,*body = NULL;
struct token *t;

       argL = END_ARGLIST;
       expList = END_EXPLIST;
       GCPush(&argL,GCarglist);
       GCPush(&expList,GCexplist);
       GCPush(&t,GCtoken);
       if (GetToken()->type != Lparen) Fatal("syntax error-let");
       while ((t = GetToken())->type != Rparen)
       {
         if (t->type != Lparen) Fatal("syntax error-let");
         t = GetToken();
         if (t->type != lSymbol) Fatal("not an lvalue-let");
/* add this arg to the arglist of the apply */
         arg = NEW(arglist);
         arg->next = argL;
         arg->arg = t->value.symbol;
         argL = arg;
         exp = NEW(explist);
         exp->thisexp = HandleExp();
         exp->next = expList;
         expList = exp;
         GetToken(); /* eat another close paren */
       }
       body = HandleExpList();
       GCPop(); GCPop(); GCPop();
       return EmitApply(EmitFunction(OneForOne,argL,body,0L),expList);
                                     /* 0L means no tail recursio here bud */
}

struct explist *
ReverseList(list)
struct explist *list;
{
struct explist *newl = NULL,*node = NULL;

	newl = 0;
       GCPush(&list,GCexplist);
       GCPush(&newl,GCexplist);
       while (list != NULL)
       {
         node = NEW(explist);
         node->thisexp = list->thisexp;
         node->next = newl;
         newl = node;
         list = list->next;
       }
       GCPop(); GCPop();
       return newl;
}

static struct treenode *
HandleCond()
{
struct explist *exp,*condList = NULL;
struct expLL *exl,*explistl = NULL;
struct token *t;

       explistl = NULL; condList = NULL;
       GCPush(&condList,GCexpLL);
       GCPush(&explistl,GCexpLL);
       while ((t = GetToken())->type != Rparen)
       {
         if (t->type != Lparen) Fatal("syntax error-Cond");
         exp = NEW(explist);
         exp->thisexp = HandleExp();
         exp->next = condList;
         condList = exp;

         exl = NEW(expLL);
         exl->thisexpl = HandleExpList();
         exl->next = explistl;
         explistl = exl;
       }
       condList = ReverseList(condList);
       explistl = (struct expLL *) ReverseList((struct explist *) explistl);
       GCPop(); GCPop();
       return EmitRealCond(condList,explistl); /* send it condition */
}

struct treenode *
HandleSequence()
{
       return EmitSequence(HandleExpList());
}

struct treenode *
HandleDelay()
{
       return EmitDelay(HandleExpList());
}

struct treenode *
HandleConStream()
{
struct explist *hel;
struct treenode *exp;

       exp = HandleExp();
       GCPush(&exp,GCtreenode);
       hel = HandleExpList();
       GCPop();
       return EmitConStream( exp,hel );
}

struct treenode *
HandleMakeEnv()
{
       return EmitMakeEnv(HandleExpList());
}

struct treenode *
HandleSymbol()
{
struct datumstruct *t;
         t = DoSymbol();
         return (EmitSymbol(t));
}

struct treenode *
HandleQuote()
{
struct datumstruct *d;
       d = DoSymbol();
       GCPush(&d,GCdatumstruct);
       GetToken();
       GCPop();
       return (EmitSymbol(d));
}

struct datumstruct *
DoSymbol(void)
{
struct token *t;

       t = GetToken();
       switch (t->type)
       {
               case lSymbol:   return MakeSym(t->value.symbol);
               case Number:    return MakeLit(t->value.integer);
               case lString:   return MakeStr(t->value.symbol);
               case Quote:
                              {
                               struct datumstruct *rs=0,*cs=0,*qs=0,*ps=0,*t;
                                 qs = NEW(datumstruct);
                                 qs->tag = Symbol;
                                 qs->data.symbol = "quote";
                                 GCPush(&qs,GCdatumstruct);
                                 ps = NEW(datumstruct);
                                 ps->tag = Pair;
                                 ps->data.pair.car = qs; /* first part o' the list */
                                 GCPush(&ps,GCdatumstruct);
                                 rs = DoSymbol(); /* recurse the quote thing */
                                 GCPush(&rs,GCdatumstruct);
                                 GCPush(&cs,GCdatumstruct);
                                 cs = NEW(datumstruct);
                                 cs->tag = Pair;
                                 cs->data.pair.car = rs;
                                 t = MakeNil();
                                 cs->data.pair.cdr = t;
                                 ps->data.pair.cdr = cs;
                                 GCPop(); GCPop(); GCPop(); GCPop();
                                 return ps; /* finish the list */
                               }
               default:        return (struct datumstruct *) MakeList();
       }
}

struct treenode *
MakeList(void)
{
struct datumstruct *head = 0,*last = 0,*ds;
struct token *t;
struct datumstruct *m;

       if ((t = GetToken())->type == Rparen)
       		return (struct treenode *)MakeNil();
       ReturnToken(t);
       GCPush(&ds,GCdatumstruct);
       GCPush(&last,GCdatumstruct);
       GCPush(&head,GCdatumstruct);
       while (t = GetToken())
       {
               ds = NEW(datumstruct);
               ds->tag = Pair;
               if (last == 0) head = ds;
               if (last != 0) last->data.pair.cdr = ds;

               switch (t->type)
               {
                 case lSymbol:
                               m = (struct datumstruct *)MakeSym(t->value.symbol);
                               ds->data.pair.car = m;
                               break;
                 case Number:
                               m = (struct datumstruct *)MakeLit(t->value.integer);
                               ds->data.pair.car = m;
                               break;
                 case Rparen:
                               m = MakeNil();
                               last->data.pair.cdr = m;
                               GCPop(); GCPop(); GCPop();
                               return (struct treenode *)head;
                               break;
                 case Lparen:
                               m = (struct datumstruct *)MakeList();
                               ds->data.pair.car = m;
                               break;
                 case Quote:   ReturnToken(t);
                               m = DoSymbol();
                               ds->data.pair.car = m;
                               break;
                 case Dot:     if (last == 0) Fatal("syntax error in QUOTE");
                               m = DoSymbol();
                               last->data.pair.cdr = m;
                               GetToken();
                               GCPop(); GCPop(); GCPop();
                               return (struct treenode *) head;
                               break;
                 case lString:
                               m = MakeStr(t->value.symbol);
                               ds->data.pair.car = m;
                               break;
               }
               last = ds;
       }
       GCPop(); GCPop(); GCPop();
       return (struct treenode *)head;
}

struct datumstruct *
MakeSym(txt)
char *txt;
{
struct datumstruct *ds;
       GCPush(&txt,GCstring);
       ds = NEW(datumstruct);
       ds->tag = Symbol;
       ds->data.symbol = txt;
       GCPop();
       return ds;
}

struct datumstruct *
MakeStr(txt)
char *txt;
{
struct datumstruct *ds;
       GCPush(&txt,GCstring);
       ds = NEW(datumstruct);
       ds->tag = dString;
       ds->data.symbol = txt;
       GCPop();
       return ds;
}

struct datumstruct *
MakeNil()
{
struct datumstruct *ds;
       ds = NEW(datumstruct);
       ds->tag = Nilval;
       return ds;
}

struct datumstruct *
MakeLit(val)
double val;
{
struct datumstruct *ds;
       ds = NEW(datumstruct);
       ds->tag = Literal;
       ds->data.literal = val;
       return ds;
}
