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
#include "parser.h"
#include "evaler.h"
#include "misc.h"
#include "stack.h"
#include "lexer.h"

extern int curbnk;
extern char *bnk[2],*mem[2];
extern long BANKSIZE;

struct envstruct *
garbage(env,forcegarb)
struct envstruct *env;
int forcegarb;
{
struct envstruct *RelocEnv();
struct datumstruct *RelocDatum();
char *RelocString();
struct treenode *RelocTree();
struct procname *RelocProcName();

/* do we really need to garbage collect? */
/*     if (forcegarb == 0)
         if ((mem[curbnk] - bnk[curbnk]) < (long) (BANKSIZE * 0.9))
            return env;     */
       curbnk = 1 - curbnk;
       mem[curbnk] = bnk[curbnk]; /* reset this pointer to totally empty */
       env = RelocEnv(env);
       RelocStack(evalstack,RelocDatum);
       RelocStack(GLOB_proc,RelocProcName);
       RelocStack(GLOB_procptr,RelocTree);
       RelocStack(convlex,RelocDatum);
       RelocGCStack();
       return env;
}

int
RelocStack(stackname,relocproc)
struct stack *stackname;
struct datumstruct * (*relocproc)();
{
struct stack *temp;
struct datumstruct *yy;

       temp = stackname;
       while (temp != 0)
       {
               yy = (*relocproc)(temp->data);
               temp->data = yy;
               temp = temp->next;
       }
}

struct envstruct *
RelocEnv(env)
struct envstruct *env;
{
struct envstruct *newenv;
struct binding *RelocBinds();
        if (env == END_ENVS) return END_ENVS;
        if (env->brh != 0) return env->brh;
        newenv = NEW(envstruct);
        env->brh = newenv; /* here or later?  */
          newenv->prevenv = RelocEnv(env->prevenv);
        newenv->bindings = RelocBinds(env->bindings);
        return newenv;
}

struct binding *
RelocBinds(bind)
struct binding *bind;
{
struct binding *newbind;
struct datumstruct *RelocDatum();
char *RelocString();

        if (bind == END_BINDINGS) return 0;
        else
        if (bind->brh != 0) return bind->brh;
        else
        {
          newbind = NEW(binding);
          bind->brh = newbind;
          newbind->variable = RelocString(bind->variable);
          newbind->value = RelocDatum(bind->value);
          newbind->next = RelocBinds(bind->next);
          return newbind;
        }
}

struct datumstruct *
RelocDatum(datum)
struct datumstruct *datum;
{
struct datumstruct *newdat;
struct treenode *RelocTree();
struct arglist *RelocArglist();
struct explist *RelocExplist();
char *RelocString();

        if (datum->brh != 0) return datum->brh;
        else
        {
          newdat = NEW(datumstruct);
          datum->brh = newdat; 
          newdat->tag = datum->tag;
          switch(datum->tag)
          {
             case Literal:      newdat->data.literal = datum->data.literal;
                                break;
             case Closure:      newdat->data.closure.env = 
                                  RelocEnv(datum->data.closure.env);
  /*printf("env->%x\n",newdat->data.closure.env); */
                                newdat->data.closure.function =
                                   RelocTree(datum->data.closure.function);
                                break;
             case Primitive:    newdat->data.prm.primitive = 
                                  NAlloc(strlen(datum->data.prm.primitive)+1);
                                strcpy(newdat->data.prm.primitive,
                                       datum->data.prm.primitive);      
                                newdat->data.prm.funct = datum->data.prm.funct;
                                break;
             case Pair:         newdat->data.pair.car =
                                  RelocDatum(datum->data.pair.car);
                                newdat->data.pair.cdr = 
                                  RelocDatum(datum->data.pair.cdr);
                                break;
             case Nilval:       break; /* a nilval is only the tag */
             case TrueVal:      break; /* same goes for TrueVal */
             case Symbol:       /* these two are the same! */
             case dString:      newdat->data.symbol =
                                  RelocString(datum->data.symbol);
                                break;
             case EnvironPtr:   newdat->data.Environ =
                                  RelocEnv(datum->data.Environ);
                                break;
             case Vector:  {             /* is this a mess or what?? */
                           int i;
                           newdat->data.vector.size =
                             datum->data.vector.size;
                           newdat->data.vector.vect0 =
                             (struct datumstruct **)
                             NAlloc(newdat->data.vector.size *
                               sizeof(struct datumstruct *));
                           for (i = 0; i < datum->data.vector.size; i++)
                             if (datum->data.vector.vect0[i] != 0)
                               newdat->data.vector.vect0[i] =
                                RelocDatum(datum->data.vector.vect0[i]);
                           }
                           break;
             default:           Fatal("Invalid datumtag- RelocDatum");
           }
           return newdat;     
    }
}

struct treenode *
RelocTree(tree)
struct treenode *tree;
{
struct treenode *newtree;
struct explist *RelocExplist();
struct arglist *RelocArglist();
char *RelocString();
struct expLL *RelocExpLL();

   if (tree->brh != 0) return tree->brh;
   else
   {
     newtree = NEW(treenode);
     newtree->tag = tree->tag;
     tree->brh = newtree;
     newtree->brh = 0;
     switch(tree->tag)
     {
        case Cond:     newtree->data.cond.condpart =
                         RelocTree(tree->data.cond.condpart);
                       newtree->data.cond.thenpart = 
                         RelocTree(tree->data.cond.thenpart);
                       newtree->data.cond.elsepart =
                         RelocTree(tree->data.cond.elsepart);
                       break;
       case Sequence:  newtree->data.sequence =
                         RelocExplist(tree->data.sequence);
                       break;
       case Assign:    newtree->data.assign.lvalue =
                         RelocString(tree->data.assign.lvalue);
                       newtree->data.assign.rvalue =
                         RelocTree(tree->data.assign.rvalue);
                       break;
       case Bind:      newtree->data.bind.lvalue =
                         RelocString(tree->data.bind.lvalue);
                       newtree->data.bind.rvalue =
                         RelocTree(tree->data.bind.rvalue);
                       break;
       case Function:  newtree->data.function.atype =
                         tree->data.function.atype;
                       newtree->data.function.arguments =
                         RelocArglist(tree->data.function.arguments);  
                       newtree->data.function.body =
                         RelocExplist(tree->data.function.body);
                       newtree->data.function.tail =
                         RelocTree(tree->data.function.tail);
                       break;
       case Identifier:newtree->data.identifier =
                         RelocString(tree->data.identifier);
                       break;
       case Constant:  newtree->data.constant = tree->data.constant;
                       break;
       case Apply:     newtree->data.apply.operator = 
                         RelocTree(tree->data.apply.operator);
                       newtree->data.apply.operands =
                         RelocExplist(tree->data.apply.operands);
                       break;
       case Emptylist: break;
       case qSymbol:   newtree->data.symbol =
                         RelocDatum(tree->data.symbol);
                       break;
       case qString:   newtree->data.identifier =
                         RelocString(tree->data.identifier);
                       break;
       case Delay:     newtree->data.delay.delayed =
                         RelocExplist(tree->data.delay.delayed);
                       break;
       case RealCond:  newtree->data.Realcond.conds =
                         RelocExplist(tree->data.Realcond.conds);
                       newtree->data.Realcond.results =
                         RelocExpLL(tree->data.Realcond.results);
                       break;
       case ConsStream:
                       newtree->data.st_pair.st_car =
                         RelocTree(tree->data.st_pair.st_car);
                       newtree->data.st_pair.st_cdr =
                         RelocExplist(tree->data.st_pair.st_cdr);
                       break;
       case MakeEnv:   newtree->data.MakeEnvBody =
                         RelocExplist(tree->data.MakeEnvBody);
                       break;
       default:        printf("Invalid treetag <%d> RelocTree\n",tree->tag);
                       Fatal("Invalid treetag- RelocTree");
      }
      return newtree;     
   }
}

struct expLL *
RelocExpLL(list)
struct expLL *list;
{
struct expLL *newlist;
struct explist *RelocExplist();

       if (list == 0) return 0;
       else
       if (list->brh != 0) return list->brh;
       else
       {
               newlist = NEW(expLL);
               list->brh = newlist;
               newlist->thisexpl = RelocExplist(list->thisexpl);
               newlist->next = RelocExpLL(list->next);
               return newlist;
       }
}

struct explist *
RelocExplist(list)
struct explist *list;
{
struct explist *newlist;
   if (list == 0) return 0;
   else
   if (list->brh != 0) return list->brh;
   else
   {
        newlist = NEW(explist);
        list->brh = newlist;
        newlist->thisexp = RelocTree(list->thisexp);
        newlist->next = RelocExplist(list->next);
        return newlist;
   }
}

struct arglist *
RelocArglist(list)
struct arglist *list;
{
struct arglist *newlist;
char *RelocString();

   if (list == 0) return 0;
   else
   if (list->brh != 0) return list->brh;
   else
   {
       newlist = NEW(arglist); 
       list->brh = newlist;
       newlist->arg = RelocString(list->arg);
       newlist->next = RelocArglist(list->next);
       return newlist;
   }
}

char *
RelocString(string)
char *string;
{
char *a;
       a = NAlloc(strlen(string)+1);
       strcpy(a,string);
       return a;
}

struct token *
RelocToken(tok)
struct token *tok;
{
struct token *t;
       if (tok->brh != 0) return tok->brh;
       t = NEW(token);
       t->type = tok->type;
       tok->brh = t;
       switch (t->type)
       {
          case Number: t->value.integer = tok->value.integer;
                       break;
          case lSymbol:
          case lString: t->value.symbol = RelocString(tok->value.symbol);
                        break;
       }
       return t;
}

struct procname *
RelocProcName(pn)
struct procname *pn;
{
struct procname *t;
    t = NEW(procname);
    t->named = pn->named;
    if (t->named != Top)
       t->lambd = RelocTree(pn->lambd);
    return t;
}

RelocGCStack()
{
pntr *crnt;
pntr varadr;
long pointer;
long flgg;
int count = 0;
       crnt = GCbottom;
       while (crnt != GCstack)
       {
          flgg = ((long) (*crnt)) & (0xFF000000);
          varadr = (pntr) ((long) (*crnt)  & (0x00FFFFFF));
          pointer = *varadr;
          if (flgg == GCtoken) pointer = (long) RelocToken(pointer);
          else if (flgg == GCdatumstruct) pointer = (long) RelocDatum(pointer);
          else if (flgg == GCtreenode) pointer = (long) RelocTree(pointer);
          else if (flgg == GCexplist) pointer = (long) RelocExplist(pointer);
          else if (flgg == GCarglist) pointer = (long) RelocArglist(pointer);
          else if (flgg == GCexpLL) pointer = (long) RelocExpLL(pointer);
          else if (flgg == GCenvstruct) pointer = (long) RelocEnv(pointer);
          else if (flgg == GCbinding) pointer = (long) RelocBinds(pointer);
          else if (flgg == GCstring) pointer = (long) RelocString(pointer);
          else { puts("invalid GCstack tag"); exit(1); }
          *varadr = pointer;
          crnt++;
          count++;
       }
 /*    printf("size of GCstack = %d\n",count);  */
}
