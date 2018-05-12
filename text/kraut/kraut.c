#include <stdio.h>
/* Kraut v0.9  */
/*     by      */
/* John Sparks */
/*  5-5-1989   */

/* This is in the public domain. Do with it as you will. */

main()
{
extern int yylex();

    while(yylex());
}

yywrap ()
{
    return (1);
}
