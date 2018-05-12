/*
    emitter interface
*/

struct treenode *EmitConstant(double);
struct treenode *EmitSymbol(struct datumstruct *);
struct treenode *EmitRealCond(struct explist *,struct expLL *);
struct treenode *EmitIdentifier(char *);
struct treenode *EmitBind(struct token *,struct treenode *);
struct treenode *EmitFunction(enum argtype,struct arglist *,struct explist *,struct treenode *);
struct treenode *EmitApply(struct treenode *,struct explist *);
struct treenode *EmitCond(struct treenode *,struct treenode *,struct treenode *);
struct treenode *EmitString(char *);
struct treenode *EmitSequence(struct explist *);
struct treenode *EmitDelay(struct explist *);
struct treenode *EmitConStream(struct treenode *,struct explist *);
struct treenode *EmitMakeEnv(struct explist *);
