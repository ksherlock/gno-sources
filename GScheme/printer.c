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
        Printer.
*/

#include "gscheme.h"
#include "printer.h"
#include "evaler.h"
#include "misc.h"

/*
        I print out datums.
*/
void
Print(dp)
struct datumstruct *dp;
{
        switch (dp->tag)
        {
               case Literal:
                       NumToStr(dp->data.literal);
                       break;
               case Delay_Pair:
               case Closure:
                       printf("#<procedure>");
                       break;
               case EnvironPtr:
                       printf("#<environment>");
                       break;
               case Primitive:
                       printf("primitive closure %s", dp->data.prm.primitive);
                       break;
               case TrueVal:
                       printf("#t");
                       break;
               case Nilval:
                       printf("()");
                       break;
               case Symbol:
                       printf("%s",dp->data.symbol);
                       break;
               case Pair:
                       putchar('(');
                       print_car(dp->data.pair.car);
                       print_cdr(dp->data.pair.cdr);
                       putchar(')');
                       break;
               case dString:
                       putchar('"');
                       printf("%s",dp->data.symbol);
                       putchar('"');
                       break;
               case Vector:
                       {
                         int i;
                         printf("#(");
                         for (i = 0; i < dp->data.vector.size; i++)
                           {
                              Print(*(dp->data.vector.vect0 + i));
                              if (i < (dp->data.vector.size-1))
                                 putchar(' ');
                           }
                         putchar(')');
                       } break;
               default:
                       printf("undefined");
        }
}

print_car(datum)
struct datumstruct *datum;
{
   if (datum->tag == Nilval) printf("()");
   else
   if (datum->tag == Pair)
   {
       putchar('(');
       print_car(datum->data.pair.car);
       print_cdr(datum->data.pair.cdr);
       putchar(')');
   }
   else Print(datum);
}

print_cdr(datum)
struct datumstruct *datum;
{
   if (datum->tag == Nilval) {}  /* a null isn't displayed at all */
   else
   if (datum->tag == Pair)
   {
       putchar(' ');
       print_car(datum->data.pair.car);
       print_cdr(datum->data.pair.cdr);
   }
   else
   {
       printf(" . ");
       Print(datum);
   }
}

NumToStr(number)            /* number is a float */
double number;
{
char *i;
char cstring[40];

   sprintf(cstring,"%.14f",number);

   i = (cstring + strlen(cstring) - 1);
   while ((*i == '0') && (i != cstring)) i--;
   if (*i == '.') *i = 0;
   else *(++i) = 0;
   printf("%s",cstring);
}
