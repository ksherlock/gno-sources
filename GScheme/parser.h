/*
       Parser interface
*/

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

/* Parse node types: */
enum treetag {
       Cond, Assign, Bind, Function, Identifier, Constant, Apply, Emptylist,
       qSymbol, qString, RealCond, Sequence, Delay, ConsStream, MakeEnv };
enum argtype { OneForOne, PackInList };

/* Linked list of arguments to a function */
struct arglist
{
       char *arg;
       struct arglist *next;
       struct arglist *brh;
};
#define END_ARGLIST ((struct arglist *) 0)

/* Linked list of expressions for apply and function */
struct explist
{
       struct treenode *thisexp;
       struct explist *next;
       struct explist *brh;
};

/* End-of-explist marker */
#define END_EXPLIST    ((struct explist *) 0)

struct expLL
{
       struct explist *thisexpl;
       struct expLL *next;
       struct expLL *brh;
};
#define END_EXPLL      ((struct expLL *) 0)

/* The parser returns a tree made up of this structure */
struct treenode
{
       enum treetag tag;
       union {
               struct {        /* an if has 3 parts: condition, then, else */
                       struct treenode *condpart;
                       struct treenode *thenpart;
                       struct treenode *elsepart;
               } cond;
               struct {   /* an assignment has 2 parts: left and right sides */
                       char *lvalue;
                       struct treenode *rvalue;
               } assign;
               struct {     /* like assign */
                       char *lvalue;
                       struct treenode *rvalue;
               } bind;
               struct { /* list of arguments, and list of expressions in body */
                       enum argtype atype;
                       struct arglist *arguments;
                       struct explist *body;
                       struct treenode *tail;
               } function;
               char *identifier; /* identifier name, and string constant */
               double constant; /* constant */
               struct datumstruct *symbol;
               struct explist *sequence;
               struct {  /* operator expression, and list of operands */
                       struct treenode *operator;
                       struct explist *operands;
               } apply;
               struct {  /* cond cond list, and list of expression lists */
                       struct explist *conds;
                       struct expLL *results;
               } Realcond;
               struct {
                       struct explist *delayed;
               } delay;
               struct {
                       struct treenode *st_car;
                       struct explist *st_cdr;
               } st_pair;
               struct explist *MakeEnvBody;  /* for make-environment */
       } data;
       struct treenode *brh;
};

/* The interface procedures for the parse module are: */
struct treenode *HandleExp(void);
