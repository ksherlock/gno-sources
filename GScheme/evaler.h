/*
       Evaluator interface.
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

/* Linked list of variable bindings */
struct binding
{
       char *variable;                  /* name */
       struct datumstruct *value;      /* value */
       struct binding *next;
       struct binding *brh;
};

/* End-of-bindings marker */
#define END_BINDINGS   ((struct binding *) 0)

/* Linked list of environments, starting with the most recent. */
struct envstruct
{
       struct envstruct *prevenv;      /* pointer to earlier environments */
       struct binding *bindings;       /* variable bindings */
       struct envstruct *brh;
};

/* End-of-environments marker */
#define END_ENVS       ((struct envstruct *) 0)

/* Datum types: */
enum datatag {Literal, Closure, Primitive, Pair, Nilval, TrueVal, Symbol,
              dString, Vector, Delay_Pair, EnvironPtr };

enum YesNo {No, Yes, Top};

/* Instances of this structure represent data */
struct datumstruct 
{
       enum datatag tag;
       union
       {
               double literal;
               char *symbol;
               struct {
                       char *primitive;
                       int (*funct)();
               } prm;
               struct {
                       struct envstruct *env;
                       struct treenode *function;
               } closure;
               struct {
                       struct datumstruct *car, *cdr;
               } pair;
               struct {
                       int size;
                       struct datumstruct **vect0;
               } vector;
               struct envstruct *Environ;
       } data;
       struct datumstruct *brh;
};

struct procname
{
    enum YesNo named;
    struct treenode *lambd;  /* if just john doe lambda */
};

/* The interface procedures for the evaluation module are: */
struct datumstruct *EvalExp();
extern void Evaluate();
