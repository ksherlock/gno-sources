/*
       Miscellaneous interface.
*/

/* This macro is handy for allocating pointers to structs. */
#define NEW(s) ((struct s *) NAlloc(sizeof(struct s)))
char *NAlloc(int);
void Fatal(char *);
InitMem(size_t);
long freemem(void);
int debug(void);
