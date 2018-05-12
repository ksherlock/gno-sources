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
        Emitter.
*/

#include "gscheme.h"
#include "emitter.h"
#include "parser.h"
#include "lexer.h"
#include "misc.h"
#include "stack.h"

/*
        I return the tree representing a constant.
*/
struct treenode *
EmitConstant(double n)
{
        struct treenode *node;

        node = NEW(treenode);
        node->tag = Constant;
        node->data.constant = n;
        return node;
}

/*
        I return the tree representing a variable (identifier).
*/
struct treenode *
EmitIdentifier(char *name)
{
struct treenode *node;

       GCPush(&name,GCstring);
       node = NEW(treenode);
       node->tag = Identifier;
       node->data.identifier = name;
       GCPop();
       return node;
}

/*
        I return the tree representing a function application.
*/
struct treenode *
EmitApply(struct treenode *op, struct explist *args)
{
struct treenode *node;

       GCPush(&op,GCtreenode);
       GCPush(&args,GCexplist);
       node = NEW(treenode);
       node->tag = Apply;
       node->data.apply.operator = op;
       node->data.apply.operands = args;
       GCPop(); GCPop();
       return node;
}

struct treenode *
EmitBind(left,right)
struct token *left;
struct treenode *right;
{
struct treenode *node;

       GCPush(&left,GCtoken);
       GCPush(&right,GCtreenode);
       node = NEW(treenode);
       node->tag = Bind;
       node->data.bind.lvalue = left->value.symbol;
       node->data.bind.rvalue = right;
       GCPop(); GCPop();
       return node;
}

struct treenode *
EmitFunction(at,args,bodylist,last_expr)
enum argtype at;
struct arglist *args;
struct explist *bodylist;
struct treenode *last_expr;
{
struct treenode *node;
       GCPush(&args,GCarglist);
       GCPush(&bodylist,GCexplist);
       GCPush(&last_expr,GCtreenode);
       node = NEW(treenode);
       node->tag = Function;
       node->data.function.atype = at;
       node->data.function.arguments = args;
       node->data.function.body = bodylist;
       node->data.function.tail = last_expr;
       GCPop(); GCPop(); GCPop();
       return node;
}

struct treenode *
EmitCond(dcond,dthen,delse)
struct treenode *dcond,*dthen,*delse;
{
struct treenode *node;
       GCPush(&dcond,GCtreenode);
       GCPush(&dthen,GCtreenode);
       GCPush(&delse,GCtreenode);
       node = NEW(treenode);
       node->tag = Cond;
       node->data.cond.condpart = dcond;
       node->data.cond.thenpart = dthen;
       node->data.cond.elsepart = delse;
       GCPop(); GCPop(); GCPop();
       return node;
}

struct treenode *
EmitSymbol(sym)
struct datumstruct *sym;
{
struct treenode *node;
       GCPush(&sym,GCdatumstruct);
       node = NEW(treenode);
       node->tag = qSymbol;
       node->data.symbol = sym;
       GCPop();
       return node;
}

struct treenode *
EmitString(str)
char *str;
{
struct treenode *node;
       GCPush(&str,GCstring);
       node = NEW(treenode);
       node->tag = qString;
       node->data.identifier = str;
       GCPop();
       return node;
}

struct treenode *
EmitRealCond(cl,crl)
struct explist *cl;
struct expLL *crl;
{
struct treenode *node;
       GCPush(&cl,GCexplist);
       GCPush(&crl,GCexpLL);
       node = NEW(treenode);
       node->tag = RealCond;
       node->data.Realcond.conds = cl;
       node->data.Realcond.results = crl;
       GCPop(); GCPop();
       return node;
}

struct treenode *
EmitSequence(el)
struct explist *el;
{
struct treenode *node;
       GCPush(&el,GCexplist);
       node = NEW(treenode);
       node->tag = Sequence;
       node->data.sequence = el;
       GCPop();
       return node;
}

struct treenode *
EmitDelay(el)
struct explist *el;
{
struct treenode *node;
       GCPush(&el,GCexplist);
       node = NEW(treenode);
       node->tag = Delay;
       node->data.delay.delayed = el;
       GCPop();
       return node;
}

struct treenode *
EmitConStream(car,cdr)
struct treenode *car;
struct explist *cdr;
{
struct treenode *node;
       GCPush(&car,GCtreenode);
       GCPush(&cdr,GCexplist);
       node = NEW(treenode);
       node->tag = ConsStream;
       node->data.st_pair.st_car = car;
       node->data.st_pair.st_cdr = cdr;
       GCPop(); GCPop();
       return node;
}

struct treenode *
EmitMakeEnv(body)
struct explist *body;
{
struct treenode *node;
       GCPush(&body,GCexplist);
       node = NEW(treenode);
       node->tag = MakeEnv;
       node->data.MakeEnvBody = body;
       GCPop();
       return node;
}
