/*
       Lexer interface
*/

#define NEWLINE 0xA    /* c's newline when lexing and shit */

/* Token types: */
enum tokentype {Number, lSymbol, lString, Lparen, Rparen, Quote, Dot};

/* A token is an instance of this structure: */
struct token
{
       enum tokentype type;
       union {
               double integer;
               char *symbol;
       } 
       value;
       struct token *brh;
};

/* The interface procecedures for the lexer module are: */
struct token *GetToken(void);
void ReturnToken(struct token *);
