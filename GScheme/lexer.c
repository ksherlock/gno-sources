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
       Lexical analyzer.
*/
#define THIS_IS_LISP
#define RParTag (struct datumstruct *) 1
#define CARtag (struct datumstruct *) 2
#define CDRtag (struct datumstruct *) 3

#include "gscheme.h"
#include "lexer.h"
#include "evaler.h"
#include "misc.h"
#include "stack.h"

FILE *INPsrc;
int RegOrDat = 0;      /* denotes which lexer we're using */

/* "unread" a character */
#define ungetchar(c)   ungetc(c, INPsrc)

/*
       Maximum length of an input symbol;
       anything longer is truncated to this length
*/
#define MAX_SYMBOL_LENGTH      256

/*
       Maximum token pushback stack is 16 -- we should never
       need more than 1
*/
#define MAX_PUSHBACK      16

/* the pushback stack is called Tokenstack, indexed by TokenIndex */
static struct token *TokenStack[MAX_PUSHBACK];
static int TokenIndex = 0;

/*
       I read characters from the input until I see enough
       to recognize an input token (a parenthesis or integer,
       for instance).  Then I return a structure indicating
       what kind of token I saw, and what value it had (if any).
*/

char buffer[MAX_SYMBOL_LENGTH];

struct token *
GetToken(void)
{
       int c, i;
       struct token *LexString();
       struct token *DatumLex();

       /* if there is a token pushed back, then I simply return that */
       if (TokenIndex) {
               return(TokenStack[--TokenIndex]);
       }
       else { /* otherwise I read another token from the standard input */
             struct token *ptoken = NEW(token);

               if (RegOrDat) return DatumLex(); /* har har !!! */

skipwhite:     /* First, I skip over meaningless characters */
               while (IsNoiseChar(c = GETchar()))
                       ;

               /* Check for comments !!!! and skip the comment and next white
                  space if there is some */

               if (c == ';')
               {
                   while ((c = GETchar()) != NEWLINE) ;
                   goto skipwhite;
               }

               /*
                       I now have a meaningful character. If this
                       character is itself a token, I am done.
               */
               if (IsTokenChar(c,0,ptoken))
                       return(ptoken);
               if (c == '"')
                       return LexString(ptoken);

               /*
                       The character I have is the beginning of
                       a token, so now I read characters into my buffer
                       until I see a noise character or a character that
                       is itself a token.  At that point the characters
                       in my buffer comprise some token, and all I have
                       to do is figure out which one.
               */
               i = 0;
               do
                   {
                       /*
                        As long as the number of characters in the token
                        I am reading is less than MAX_SYMBOL_LENGTH, everything
                        is fine: I simply convert the character to lower-case and
                        stick it in my buffer.  Otherwise, the symbol is too long,
                        so I throw away the excess by reading it in without storing
                        it anywhere.
                       */
                       if (i < MAX_SYMBOL_LENGTH)
                       {
                               if (c >= 'A' && c <= 'Z')
                                       c += 'a' - 'A';
                               buffer[i++] = c;
                       }
               }
               while (!IsNoiseChar(c = GETchar()) && !IsTokenChar(c,1,ptoken));


               /*
                 Now I mark the end of what I just read in.  If its
                 length exceeded the length of my buffer, then the end
                 is the last position in the buffer.
               */
               if (i < MAX_SYMBOL_LENGTH)
                       buffer[i] = '\0';
               else
                       buffer[MAX_SYMBOL_LENGTH-1] = '\0';

               /*
                 I always have to read 1 character more than I actually
                 want in order to be sure I've reached the end.
                 Now I put that character back to be read next time.
               */
               (void) ungetchar(c);

               /*
                 I decide what sort of token is represented by the
                 contents of my buffer, and set things up accordingly.
               */
               if (IsNumeric(buffer))
               {
                       ptoken->type = Number;
                       sscanf(buffer,"%lf",&ptoken->value.integer);
               }
               else
               {
               char *strcpy(),*temp;
               ptoken->type = lSymbol;
               /*
                       I have to allocate memory for the symbol I have,
                       and copy my buffer into that memory. 
                       I allocate 1 more than the length of the string
                       so that there is room for the \0 terminator.
                       Note the type casts used to get appropriate types.
               */
               GCPush(&ptoken,GCtoken);
               temp = strcpy(NAlloc(1+strlen(buffer)), buffer);
               ptoken->value.symbol = temp;
               GCPop();
               }

               /* I return the token */
               return(ptoken);

       }
}

/*
       I return 1 if my argument is a noise character, 0 otherwise.
*/
static
IsNoiseChar(c)
int c;
{
       return c == ' ' || c == '\t' || c == '\n';
}

/*
       I return 1 if my first argument represents a token, 0 otherwise.
       I have the side effect of returning the token via my second argument.
*/
static
IsTokenChar(c, flag, ptoken)
int c;
int flag;
struct token *ptoken;
{
       switch (c)
       {
       case EOF:
               /*
                       I should never see end-of-file: the user
                       must explicitly ask to exit the interpreter.
                       so this must be a file-read in
               */
               fclose(INPsrc);
               INPsrc = stdin;
               break;
       case '(':
               ptoken->type = Lparen;
               break;
       case ')':
               ptoken->type = Rparen;
               break;
       case 0x27:
               ptoken->type = Quote;
               break;
       case '.':
               if (flag == 0)
               {
                 ptoken->type = Dot;
                 break;
               }
               else return 0;
       default:
               return 0;
       }

       return 1;
}

/*
       I return 1 if my argument represents an integer, 0 otherwise.
*/
static
IsNumeric(string)
char *string;
{
int eflag = 0,
    ptflag = 0;
int length;
char *startp = string;

   length = strlen(string);
   while (length != 0)
   {
     length--;
     if ((*string == '-') || (*string == '+'))
     {
       if ((string == startp) && (length == 0)) return 0;/* + or - alone */
       else if (! ((string == startp) || (*(string-1) == 'e')))
         return 0;
     }
     else if (*string == 'e')
     {
       if (string == startp) return 0;
       if (eflag) return 0;
       else eflag = 1;
     }
     else if (*string == '.')
     {
       if (ptflag) return 0;
       else ptflag = 1;
     }
     else if ((*string < '0') || (*string > '9')) return 0;
     string++;
   }
   return 1;
}

void
ReturnToken(struct token *ptoken)
{
       TokenStack[TokenIndex++]=ptoken;
}

struct token *
LexString(t)
struct token *t;
{
char str[255];
char c;
int i = 0;
       GCPush(&t,GCstring);
       while ((c = GETchar()) != '"')
       {
               str[i++] = c;
       }
       str[i] = 0;
       t->type = lString;
       t->value.symbol = NAlloc(strlen(str)+1);
       strcpy(t->value.symbol,str);
       GCPop();
       return t;
}

GETchar()
{
static int c;
       if ((c = getc(INPsrc)) == EOF)
       {
               fclose(INPsrc);
               INPsrc = stdin;
               return GETchar();
       }
       else
         return c;
}


/*
        I convert datastructs into lexer tokens. I am clever.
*/

struct token *
DatumLex()
{
struct datumstruct *datum;
struct token *ptoken = NEW(token);
char *strcpy();
struct token *MakeTok();

/*      if (nullstack(convlex)) turn ourselves off and return     */
        if (STACKTOP(convlex) == RParTag)
        {
           Pop(&convlex);
           ptoken->type = Rparen;  /* take care of the "promise" to do a ) */
           return ptoken;
        }
        if (STACKTOP(convlex) == CARtag) /* this is a car, so do things a bit */
        {                                /* differently   */
           Pop(&convlex);
           if (STACKTOP(convlex)->tag == Pair)
           {
               datum = Pop(&convlex);
               return MakeTok(datum,ptoken);
           }
           else
           if (STACKTOP(convlex)->tag == Nilval)
           {
               Pop(&convlex);
               Push(RParTag,&convlex);  /* we un-parse a nilval, even though */
               ptoken->type = Lparen;   /* in no way is it a valid Scheme    */
                                        /* statement                         */
               return ptoken;
           }
           else return MakeTok(Pop(&convlex),ptoken); /* do anything else */
        }
        if (STACKTOP(convlex) == CDRtag)  /* for cdr, we worry about '.' etc.*/
        {
           Pop(&convlex);
           if (STACKTOP(convlex)->tag == Nilval)
                       /* a null means the end of a list */
           {
              Pop(&convlex);
              ptoken->type = Rparen;
              return ptoken;
           }
           else
           if (STACKTOP(convlex)->tag == Pair)
           {                /* for a new pair, we do NOT send a Lparen (a list
                               is what we are */
              datum = Pop(&convlex);
              Push(datum->data.pair.cdr,&convlex);
              Push(CDRtag,&convlex);
              Push(datum->data.pair.car,&convlex);
              Push(CARtag,&convlex);
              return DatumLex();
           }
           else
           {
              datum = Pop(&convlex);   /* here's the dot thing (cdr is an */
              Push(RParTag,&convlex);  /* object  */
              Push(datum,&convlex);
              ptoken->type = Dot;
              return ptoken;
           }
        }
        else return MakeTok(Pop(&convlex),ptoken);
}

struct token *
MakeTok(dp,ptoken)
struct datumstruct *dp;
struct token *ptoken;
{
char *strcpy();
        switch (dp->tag) /* i.e. the cute stack of ds ptrs */
        {
               case Literal:   /* a literal just gets sent over */
                       ptoken->type = Number;
                       ptoken->value.integer = dp->data.literal;
                       return ptoken;
                       break;
               case Nilval:   /* turn nilval into a ( and ). invalid syntax */
                       Push(RParTag,&convlex);
                       ptoken->type = Lparen;
                       return ptoken;
                       break;
               case Symbol:  /* har har, just a symbol thing. */
               {
               char *str;
                   str = dp->data.symbol;
                   GCPush(&ptoken,GCtoken);
                   GCPush(&str,GCstring);
                   ptoken->value.symbol = strcpy(NAlloc(1+strlen(str)), str);
                   GCPop(); GCPop();
                   ptoken->type = lSymbol;
                   return ptoken;
               }
               break;
/* for Pair, immediately return a '('. Push on the stack promises to do
   the car and cdr of the pair, but not a dummy ')' which will be stuffed
   on later by the CDR routine. */

               case Pair:
                       Push(dp->data.pair.cdr,&convlex);
                       Push(CDRtag,&convlex); /* the 'cdr' routine thing. */
                       Push(dp->data.pair.car,&convlex);
                       Push(CARtag,&convlex); /* the 'car' routine thing */
                       ptoken->type = Lparen;
                       return ptoken;
                       break;
               case dString:   /* just like symbol */
                       {
                       char *str;
                           str = dp->data.symbol;
                           GCPush(&ptoken,GCtoken);
                           GCPush(&str,GCstring);
                     ptoken->value.symbol = strcpy(NAlloc(1+strlen(str)), str);
                           ptoken->type = lString;
                           GCPop(); GCPop();
                           return ptoken;
                       }
                       break;
               default:
                       printf("type <%d> ",dp->tag);
                       Fatal("undefined data type in DatumLex");
        }
}
