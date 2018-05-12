#ifdef __ORCAC__
segment "lex.yy";
#endif

# include "stdio.h"
# define U(x) x
# define NLSTATE yyprevious=YYNEWLINE
# define BEGIN yybgin = yysvec + 1 +
# define INITIAL 0
# define YYLERR yysvec
# define YYSTATE (yyestate-yysvec-1)
# define YYOPTIM 1
# define YYLMAX BUFSIZ
# define output(c) putc(c,yyout)
# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
# define unput(c) {yytchar= (c);if(yytchar=='\n')yylineno--;*yysptr++=yytchar;}
# define yymore() (yymorfg=1)
# define ECHO fprintf(yyout, "%s",yytext)
# define REJECT { nstr = yyreject(); goto yyfussy;}
int yyleng; extern char yytext[];
int yymorfg;
extern char *yysptr, yysbuf[];
int yytchar;
FILE *yyin /*= {stdin}*/, *yyout/* = {stdout}*/;
extern int yylineno;
struct yysvf { 
	struct yywork *yystoff;
	struct yysvf *yyother;
	int *yystops;};
struct yysvf *yyestate;
extern struct yysvf yysvec[], *yybgin;
# define A 2
# define str 4
# define sc 6
# define reg 8
# define comment 10
/****************************************************************
Copyright (C) AT&T 1993
All Rights Reserved

Permission to use, copy, modify, and distribute this software and
its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appear in all
copies and that both that the copyright notice and this
permission notice and warranty disclaimer appear in supporting
documentation, and that the name of AT&T or any of its entities
not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior
permission.

AT&T DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
IN NO EVENT SHALL AT&T OR ANY OF ITS ENTITIES BE LIABLE FOR ANY
SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
THIS SOFTWARE.
****************************************************************/

/* some of this depends on behavior of lex that
   may not be preserved in other implementations of lex.
*/

#undef	input	/* defeat lex */
#undef	unput

#include <stdlib.h>
#include <string.h>
#include "awk.h"
#include "y.tab.h"

extern YYSTYPE	yylval;
extern int	infunc;

void initlex(void)
{
  yyin = stdin;
  yyout = stdout;
}

int	lineno	= 1;
int	bracecnt = 0;
int	brackcnt  = 0;
int	parencnt = 0;
#define DEBUG
#ifdef	DEBUG
#	define	RET(x)	{if(dbg)printf("lex %s [%s]\n", tokname(x), yytext); return(x); }
#else
#	define	RET(x)	return(x)
#endif

#define	CADD	cbuf[clen++] = yytext[0]; \
		if (clen >= CBUFLEN-1) { \
			ERROR "string/reg expr %.30s... too long", cbuf SYNTAX; \
			BEGIN A; \
		}

uchar	cbuf[CBUFLEN];
uchar	*s;
int	clen, cflag;
# define YYNEWLINE 10
extern int yywrap(void);
int yylex(void){
int nstr; extern int yyprevious;
switch (yybgin-yysvec-1) {	/* witchcraft */
	case 0:
		BEGIN A;
		break;
	case sc:
		BEGIN A;
		RET('}');
	}
while((nstr = yylook()) >= 0)
yyfussy: switch(nstr){
case 0:
if(yywrap()) return(0); break;
case 1:
	{ lineno++; RET(NL); }
break;
case 2:
	{ ; }
break;
case 3:
{ ; }
break;
case 4:
	{ RET(';'); }
break;
case 5:
{ lineno++; }
break;
case 6:
{ RET(XBEGIN); }
break;
case 7:
	{ RET(XEND); }
break;
case 8:
{ if (infunc) ERROR "illegal nested function" SYNTAX; RET(FUNC); }
break;
case 9:
{ if (!infunc) ERROR "return not in function" SYNTAX; RET(RETURN); }
break;
case 10:
	{ RET(AND); }
break;
case 11:
	{ RET(BOR); }
break;
case 12:
	{ RET(NOT); }
break;
case 13:
	{ yylval.i = NE; RET(NE); }
break;
case 14:
	{ yylval.i = MATCH; RET(MATCHOP); }
break;
case 15:
	{ yylval.i = NOTMATCH; RET(MATCHOP); }
break;
case 16:
	{ yylval.i = LT; RET(LT); }
break;
case 17:
	{ yylval.i = LE; RET(LE); }
break;
case 18:
	{ yylval.i = EQ; RET(EQ); }
break;
case 19:
	{ yylval.i = GE; RET(GE); }
break;
case 20:
	{ yylval.i = GT; RET(GT); }
break;
case 21:
	{ yylval.i = APPEND; RET(APPEND); }
break;
case 22:
	{ yylval.i = INCR; RET(INCR); }
break;
case 23:
	{ yylval.i = DECR; RET(DECR); }
break;
case 24:
	{ yylval.i = ADDEQ; RET(ASGNOP); }
break;
case 25:
	{ yylval.i = SUBEQ; RET(ASGNOP); }
break;
case 26:
	{ yylval.i = MULTEQ; RET(ASGNOP); }
break;
case 27:
	{ yylval.i = DIVEQ; RET(ASGNOP); }
break;
case 28:
	{ yylval.i = MODEQ; RET(ASGNOP); }
break;
case 29:
	{ yylval.i = POWEQ; RET(ASGNOP); }
break;
case 30:
{ yylval.i = POWEQ; RET(ASGNOP); }
break;
case 31:
	{ yylval.i = ASSIGN; RET(ASGNOP); }
break;
case 32:
	{ RET(POWER); }
break;
case 33:
	{ RET(POWER); }
break;
case 34:
{ yylval.cp = fieldadr(atoi(yytext+1)); RET(FIELD); }
break;
case 35:
{ unputstr("(NF)"); return(INDIRECT); }
break;
case 36:
{ int c, n;
		  c = input(); unput(c);
		  if (c == '(' || c == '[' || infunc && (n=isarg(yytext+1)) >= 0) {
			unputstr(yytext+1);
			return(INDIRECT);
		  } else {
			yylval.cp = setsymtab(yytext+1,"",0.0,STR|NUM,symtab);
			RET(IVAR);
		  }
		}
break;
case 37:
	{ RET(INDIRECT); }
break;
case 38:
	{ yylval.cp = setsymtab(yytext, "", 0.0, NUM, symtab); RET(VARNF); }
break;
case 39:
{
		  yylval.cp = setsymtab(yytext, tostring(yytext), atof(yytext), CON|NUM, symtab);
		/* should this also have STR set? */
		  RET(NUMBER); }
break;
case 40:
{ RET(WHILE); }
break;
case 41:
	{ RET(FOR); }
break;
case 42:
	{ RET(DO); }
break;
case 43:
	{ RET(IF); }
break;
case 44:
	{ RET(ELSE); }
break;
case 45:
	{ RET(NEXT); }
break;
case 46:
	{ RET(EXIT); }
break;
case 47:
{ RET(BREAK); }
break;
case 48:
{ RET(CONTINUE); }
break;
case 49:
{ yylval.i = PRINT; RET(PRINT); }
break;
case 50:
{ yylval.i = PRINTF; RET(PRINTF); }
break;
case 51:
{ yylval.i = SPRINTF; RET(SPRINTF); }
break;
case 52:
{ yylval.i = SPLIT; RET(SPLIT); }
break;
case 53:
{ RET(SUBSTR); }
break;
case 54:
	{ yylval.i = SUB; RET(SUB); }
break;
case 55:
	{ yylval.i = GSUB; RET(GSUB); }
break;
case 56:
{ RET(INDEX); }
break;
case 57:
{ RET(MATCHFCN); }
break;
case 58:
	{ RET(IN); }
break;
case 59:
{ RET(GETLINE); }
break;
case 60:
{ RET(CLOSE); }
break;
case 61:
{ RET(DELETE); }
break;
case 62:
{ yylval.i = FLENGTH; RET(BLTIN); }
break;
case 63:
	{ yylval.i = FLOG; RET(BLTIN); }
break;
case 64:
	{ yylval.i = FINT; RET(BLTIN); }
break;
case 65:
	{ yylval.i = FEXP; RET(BLTIN); }
break;
case 66:
	{ yylval.i = FSQRT; RET(BLTIN); }
break;
case 67:
	{ yylval.i = FSIN; RET(BLTIN); }
break;
case 68:
	{ yylval.i = FCOS; RET(BLTIN); }
break;
case 69:
{ yylval.i = FATAN; RET(BLTIN); }
break;
case 70:
{ yylval.i = FSYSTEM; RET(BLTIN); }
break;
case 71:
	{ yylval.i = FRAND; RET(BLTIN); }
break;
case 72:
{ yylval.i = FSRAND; RET(BLTIN); }
break;
case 73:
{ yylval.i = FTOUPPER; RET(BLTIN); }
break;
case 74:
{ yylval.i = FTOLOWER; RET(BLTIN); }
break;
case 75:
{ yylval.i = FFLUSH; RET(BLTIN); }
break;
case 76:
{ int n, c;
		  c = input(); unput(c);	/* look for '(' */
		  if (c != '(' && infunc && (n=isarg(yytext)) >= 0) {
			yylval.i = n;
			RET(ARG);
		  } else {
			yylval.cp = setsymtab(yytext,"",0.0,STR|NUM,symtab);
			if (c == '(') {
				RET(CALL);
			} else {
				RET(VAR);
			}
		  }
		}
break;
case 77:
	{ BEGIN str; clen = 0; }
break;
case 78:
	{ if (--bracecnt < 0) ERROR "extra }" SYNTAX; BEGIN sc; RET(';'); }
break;
case 79:
	{ if (--brackcnt < 0) ERROR "extra ]" SYNTAX; RET(']'); }
break;
case 80:
	{ if (--parencnt < 0) ERROR "extra )" SYNTAX; RET(')'); }
break;
case 81:
	{ if (yytext[0] == '{') bracecnt++;
		  else if (yytext[0] == '[') brackcnt++;
		  else if (yytext[0] == '(') parencnt++;
		  RET(yylval.i = yytext[0]); /* everything else */ }
break;
case 82:
{ cbuf[clen++] = '\\'; cbuf[clen++] = yytext[1]; }
break;
case 83:
	{ ERROR "newline in regular expression %.10s...", cbuf SYNTAX; lineno++; BEGIN A; }
break;
case 84:
{ BEGIN A;
		  cbuf[clen] = 0;
		  yylval.s = tostring(cbuf);
		  unput('/');
		  RET(REGEXPR); }
break;
case 85:
	{ CADD; }
break;
case 86:
	{ BEGIN A;
		  cbuf[clen] = 0; s = tostring(cbuf);
		  cbuf[clen] = ' '; cbuf[++clen] = 0;
		  yylval.cp = setsymtab(cbuf, s, 0.0, CON|STR, symtab);
		  RET(STRING); }
break;
case 87:
	{ ERROR "newline in string %.10s...", cbuf SYNTAX; lineno++; BEGIN A; }
break;
case 88:
{ cbuf[clen++] = '"'; }
break;
case 89:
{ cbuf[clen++] = '\n'; }
break;
case 90:
{ cbuf[clen++] = '\t'; }
break;
case 91:
{ cbuf[clen++] = '\f'; }
break;
case 92:
{ cbuf[clen++] = '\r'; }
break;
case 93:
{ cbuf[clen++] = '\b'; }
break;
case 94:
{ cbuf[clen++] = '\v'; }
break;
case 95:
{ cbuf[clen++] = '\007'; }
break;
case 96:
{ cbuf[clen++] = '\\'; }
break;
case 97:
{ int n;
		  sscanf(yytext+1, "%o", &n); cbuf[clen++] = n; }
break;
case 98:
{ int n;	/* ANSI permits any number! */
		  sscanf(yytext+2, "%x", &n); cbuf[clen++] = n; }
break;
case 99:
{ cbuf[clen++] = yytext[1]; }
break;
case 100:
	{ CADD; }
break;
case -1:
break;
default:
fprintf(yyout,"bad switch yylook %d",nstr);
} return(0); }
/* end of yylex */

void startreg(void)	/* start parsing a regular expression */
{
	BEGIN reg;
	clen = 0;
}

/* input() and unput() are transcriptions of the standard lex
   macros for input and output with additions for error message
   printing.  God help us all if someone changes how lex works.
*/

uchar	ebuf[300];
uchar	*ep = ebuf;

int input(void)	/* get next lexical input character */
{
	register int c;
	extern uchar *lexprog;

	if (yysptr > yysbuf)
		c = U(*--yysptr);
	else if (lexprog != NULL) {	/* awk '...' */
		if (c = *lexprog)
			lexprog++;
	} else				/* awk -f ... */
		c = pgetc();
	if (c == '\n')
		yylineno++;
	else if (c == EOF)
		c = 0;
	if (ep >= ebuf + sizeof ebuf)
		ep = ebuf;
	return *ep++ = c;
}

void unput(int c)	/* put lexical character back on input */
{
	yytchar = c;
	if (yytchar == '\n')
		yylineno--;
	*yysptr++ = yytchar;
	if (--ep < ebuf)
		ep = ebuf + sizeof(ebuf) - 1;
}


void unputstr(char *s)	/* put a string back on input */
{
	int i;

	for (i = strlen(s)-1; i >= 0; i--)
		unput(s[i]);
}
int yyvstop[] = {
0,

81,
0,

3,
81,
0,

1,
0,

12,
81,
0,

77,
81,
0,

2,
81,
0,

37,
81,
0,

81,
0,

81,
0,

80,
81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

39,
81,
0,

4,
81,
0,

16,
81,
0,

31,
81,
0,

20,
81,
0,

76,
81,
0,

76,
81,
0,

76,
81,
0,

76,
81,
0,

81,
0,

79,
81,
0,

33,
81,
0,

76,
81,
0,

76,
81,
0,

76,
81,
0,

76,
81,
0,

76,
81,
0,

76,
81,
0,

76,
81,
0,

76,
81,
0,

76,
81,
0,

76,
81,
0,

76,
81,
0,

76,
81,
0,

76,
81,
0,

76,
81,
0,

76,
81,
0,

76,
81,
0,

81,
0,

78,
81,
0,

14,
81,
0,

100,
0,

87,
0,

86,
100,
0,

100,
0,

85,
0,

83,
0,

84,
85,
0,

85,
0,

3,
0,

13,
0,

15,
0,

2,
0,

34,
0,

36,
0,

36,
0,

28,
0,

10,
0,

32,
0,

26,
0,

22,
0,

24,
0,

23,
0,

25,
0,

39,
0,

27,
0,

39,
0,

39,
0,

17,
0,

18,
0,

19,
0,

21,
0,

76,
0,

76,
0,

76,
0,

38,
76,
0,

5,
0,

29,
0,

76,
0,

76,
0,

76,
0,

76,
0,

76,
0,

42,
76,
0,

76,
0,

76,
0,

76,
0,

76,
0,

76,
0,

76,
0,

76,
0,

43,
76,
0,

58,
76,
0,

76,
0,

76,
0,

76,
0,

76,
0,

76,
0,

76,
0,

76,
0,

76,
0,

76,
0,

76,
0,

76,
0,

76,
0,

76,
0,

76,
0,

76,
0,

11,
0,

99,
0,

88,
99,
0,

97,
99,
0,

96,
99,
0,

95,
99,
0,

93,
99,
0,

91,
99,
0,

89,
99,
0,

92,
99,
0,

90,
99,
0,

94,
99,
0,

99,
0,

82,
0,

35,
36,
0,

30,
0,

39,
0,

76,
0,

7,
76,
0,

76,
0,

76,
0,

76,
0,

76,
0,

68,
76,
0,

76,
0,

76,
0,

76,
0,

65,
76,
0,

76,
0,

41,
76,
0,

76,
0,

76,
0,

76,
0,

76,
0,

64,
76,
0,

76,
0,

63,
76,
0,

76,
0,

76,
0,

76,
0,

76,
0,

76,
0,

67,
76,
0,

76,
0,

76,
0,

76,
0,

76,
0,

54,
76,
0,

76,
0,

76,
0,

76,
0,

76,
0,

97,
0,

98,
0,

76,
0,

76,
0,

76,
0,

76,
0,

76,
0,

76,
0,

44,
76,
0,

46,
76,
0,

76,
0,

8,
76,
0,

76,
0,

55,
76,
0,

76,
0,

76,
0,

76,
0,

45,
76,
0,

76,
0,

71,
76,
0,

76,
0,

76,
0,

76,
0,

66,
76,
0,

76,
0,

76,
0,

76,
0,

76,
0,

76,
0,

76,
0,

97,
0,

6,
76,
0,

69,
76,
0,

47,
76,
0,

60,
76,
0,

76,
0,

76,
0,

76,
0,

76,
0,

76,
0,

56,
76,
0,

76,
0,

57,
76,
0,

49,
76,
0,

76,
0,

52,
76,
0,

76,
0,

72,
76,
0,

76,
0,

76,
0,

76,
0,

76,
0,

40,
76,
0,

76,
0,

61,
76,
0,

75,
76,
0,

76,
0,

76,
0,

62,
76,
0,

50,
76,
0,

9,
76,
0,

76,
0,

53,
76,
0,

70,
76,
0,

76,
0,

76,
0,

76,
0,

76,
0,

59,
76,
0,

51,
76,
0,

74,
76,
0,

73,
76,
0,

48,
76,
0,

8,
76,
0,
0};
# define YYTYPE int
struct yywork { YYTYPE verify, advance; } yycrank[] = {
0,0,	0,0,	3,13,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	3,14,	3,15,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	5,59,	14,67,	0,0,	
0,0,	0,0,	0,0,	37,95,	
0,0,	5,59,	5,60,	0,0,	
0,0,	0,0,	3,16,	3,17,	
3,18,	3,19,	3,20,	3,21,	
0,0,	0,0,	3,22,	3,23,	
3,24,	14,67,	3,25,	3,26,	
3,27,	3,28,	6,61,	0,0,	
0,0,	0,0,	5,61,	21,75,	
0,0,	3,28,	0,0,	0,0,	
3,29,	3,30,	3,31,	3,32,	
23,76,	25,80,	3,33,	3,34,	
5,59,	10,65,	3,35,	24,78,	
3,33,	70,0,	16,68,	0,0,	
5,59,	0,0,	20,74,	3,36,	
27,83,	25,81,	30,87,	23,77,	
31,88,	5,59,	32,89,	32,90,	
9,63,	24,79,	34,92,	5,59,	
36,94,	3,37,	3,38,	3,39,	
9,63,	9,64,	3,40,	3,41,	
3,42,	3,43,	3,44,	3,45,	
3,46,	35,93,	3,47,	39,96,	
6,62,	3,48,	3,49,	3,50,	
5,62,	3,51,	10,66,	3,52,	
3,53,	3,54,	76,142,	0,0,	
3,55,	49,114,	0,0,	0,0,	
93,146,	3,56,	3,57,	3,58,	
4,16,	4,17,	4,18,	4,19,	
4,20,	4,21,	9,65,	9,63,	
4,22,	4,23,	4,24,	16,69,	
4,25,	4,26,	4,27,	9,63,	
40,97,	41,98,	42,99,	44,103,	
46,108,	42,100,	43,101,	50,115,	
9,63,	51,116,	4,29,	4,30,	
4,31,	4,32,	9,63,	44,104,	
43,102,	4,34,	46,109,	45,105,	
4,35,	48,112,	47,110,	52,117,	
18,70,	54,125,	55,126,	52,118,	
45,106,	4,36,	47,111,	48,113,	
18,70,	18,0,	45,107,	9,66,	
56,127,	82,86,	92,145,	97,147,	
98,148,	99,149,	100,150,	4,37,	
4,38,	4,39,	101,152,	100,151,	
4,40,	4,41,	4,42,	4,43,	
4,44,	4,45,	4,46,	103,153,	
4,47,	104,154,	105,156,	4,48,	
4,49,	4,50,	106,157,	4,51,	
104,155,	4,52,	4,53,	4,54,	
107,158,	82,86,	4,55,	18,70,	
108,159,	53,119,	109,160,	4,56,	
4,57,	4,58,	112,163,	18,70,	
53,120,	53,121,	53,122,	113,164,	
114,165,	53,123,	115,166,	116,167,	
18,70,	53,124,	111,161,	117,168,	
118,169,	119,170,	18,70,	19,71,	
19,71,	19,71,	19,71,	19,71,	
19,71,	19,71,	19,71,	19,71,	
19,71,	120,171,	111,162,	121,173,	
122,174,	123,175,	124,176,	120,172,	
19,72,	19,72,	19,72,	19,72,	
19,72,	19,72,	19,72,	19,72,	
19,72,	19,72,	19,72,	19,72,	
19,72,	19,73,	19,72,	19,72,	
19,72,	19,72,	19,72,	19,72,	
19,72,	19,72,	19,72,	19,72,	
19,72,	19,72,	126,179,	145,182,	
147,183,	148,184,	19,72,	149,185,	
19,72,	19,72,	19,72,	19,72,	
19,72,	19,72,	19,72,	19,72,	
19,72,	19,72,	19,72,	19,72,	
19,72,	19,72,	19,72,	19,72,	
19,72,	19,72,	19,72,	19,72,	
19,72,	19,72,	19,72,	19,72,	
19,72,	19,72,	26,82,	26,82,	
26,82,	26,82,	26,82,	26,82,	
26,82,	26,82,	26,82,	26,82,	
28,84,	150,186,	28,85,	28,85,	
28,85,	28,85,	28,85,	28,85,	
28,85,	28,85,	28,85,	28,85,	
33,91,	33,91,	33,91,	33,91,	
33,91,	33,91,	33,91,	33,91,	
33,91,	33,91,	152,187,	28,86,	
153,188,	154,189,	156,190,	158,191,	
159,192,	33,91,	33,91,	33,91,	
33,91,	33,91,	33,91,	33,91,	
33,91,	33,91,	33,91,	33,91,	
33,91,	33,91,	33,91,	33,91,	
33,91,	33,91,	33,91,	33,91,	
33,91,	33,91,	33,91,	33,91,	
33,91,	33,91,	33,91,	28,86,	
160,193,	161,194,	163,195,	33,91,	
165,196,	33,91,	33,91,	33,91,	
33,91,	33,91,	33,91,	33,91,	
33,91,	33,91,	33,91,	33,91,	
33,91,	33,91,	33,91,	33,91,	
33,91,	33,91,	33,91,	33,91,	
33,91,	33,91,	33,91,	33,91,	
33,91,	33,91,	33,91,	62,128,	
166,197,	66,140,	167,198,	168,199,	
169,200,	171,201,	172,202,	62,128,	
62,0,	66,140,	66,0,	71,71,	
71,71,	71,71,	71,71,	71,71,	
71,71,	71,71,	71,71,	71,71,	
71,71,	72,72,	72,72,	72,72,	
72,72,	72,72,	72,72,	72,72,	
72,72,	72,72,	72,72,	173,203,	
62,129,	174,204,	175,205,	176,206,	
73,72,	73,72,	73,72,	73,72,	
73,72,	73,72,	73,72,	73,72,	
73,72,	73,72,	62,130,	177,207,	
66,140,	178,208,	125,177,	72,72,	
179,209,	182,211,	62,128,	183,212,	
66,140,	184,213,	73,141,	125,178,	
185,214,	186,215,	187,216,	62,128,	
190,217,	66,140,	73,72,	191,218,	
192,219,	62,128,	194,220,	66,140,	
84,84,	84,84,	84,84,	84,84,	
84,84,	84,84,	84,84,	84,84,	
84,84,	84,84,	130,180,	130,180,	
130,180,	130,180,	130,180,	130,180,	
130,180,	130,180,	62,131,	195,221,	
196,222,	84,86,	198,223,	62,132,	
62,133,	200,224,	201,225,	202,226,	
62,134,	204,227,	205,228,	206,229,	
207,230,	208,231,	209,232,	215,233,	
62,135,	216,234,	217,235,	218,236,	
62,136,	219,237,	62,137,	221,238,	
62,138,	223,239,	62,139,	224,240,	
226,241,	86,143,	228,242,	86,143,	
229,243,	84,86,	86,144,	86,144,	
86,144,	86,144,	86,144,	86,144,	
86,144,	86,144,	86,144,	86,144,	
139,181,	139,181,	139,181,	139,181,	
139,181,	139,181,	139,181,	139,181,	
139,181,	139,181,	230,244,	231,245,	
233,246,	236,247,	237,248,	241,249,	
244,250,	139,181,	139,181,	139,181,	
139,181,	139,181,	139,181,	245,251,	
246,252,	141,72,	141,72,	141,72,	
141,72,	141,72,	141,72,	141,72,	
141,72,	141,72,	141,72,	143,144,	
143,144,	143,144,	143,144,	143,144,	
143,144,	143,144,	143,144,	143,144,	
143,144,	247,253,	0,0,	0,0,	
0,0,	139,181,	139,181,	139,181,	
139,181,	139,181,	139,181,	141,72,	
180,210,	180,210,	180,210,	180,210,	
180,210,	180,210,	180,210,	180,210,	
0,0};
struct yysvf yysvec[] = {
0,	0,	0,
yycrank+0,	0,		0,	
yycrank+0,	0,		0,	
yycrank+-1,	0,		0,	
yycrank+-95,	yysvec+3,	0,	
yycrank+-20,	0,		0,	
yycrank+-16,	yysvec+5,	0,	
yycrank+0,	0,		0,	
yycrank+0,	0,		0,	
yycrank+-87,	0,		0,	
yycrank+-22,	yysvec+9,	0,	
yycrank+0,	0,		0,	
yycrank+0,	0,		0,	
yycrank+0,	0,		yyvstop+1,
yycrank+13,	0,		yyvstop+3,
yycrank+0,	0,		yyvstop+6,
yycrank+13,	0,		yyvstop+8,
yycrank+0,	0,		yyvstop+11,
yycrank+-167,	0,		yyvstop+14,
yycrank+191,	0,		yyvstop+17,
yycrank+17,	0,		yyvstop+20,
yycrank+17,	0,		yyvstop+22,
yycrank+0,	0,		yyvstop+24,
yycrank+22,	0,		yyvstop+27,
yycrank+28,	0,		yyvstop+29,
yycrank+20,	0,		yyvstop+31,
yycrank+266,	0,		yyvstop+33,
yycrank+19,	0,		yyvstop+35,
yycrank+278,	0,		yyvstop+37,
yycrank+0,	0,		yyvstop+40,
yycrank+21,	0,		yyvstop+43,
yycrank+23,	0,		yyvstop+46,
yycrank+25,	0,		yyvstop+49,
yycrank+288,	0,		yyvstop+52,
yycrank+21,	yysvec+33,	yyvstop+55,
yycrank+27,	yysvec+33,	yyvstop+58,
yycrank+22,	yysvec+33,	yyvstop+61,
yycrank+17,	0,		yyvstop+64,
yycrank+0,	0,		yyvstop+66,
yycrank+46,	0,		yyvstop+69,
yycrank+28,	yysvec+33,	yyvstop+72,
yycrank+31,	yysvec+33,	yyvstop+75,
yycrank+38,	yysvec+33,	yyvstop+78,
yycrank+49,	yysvec+33,	yyvstop+81,
yycrank+39,	yysvec+33,	yyvstop+84,
yycrank+61,	yysvec+33,	yyvstop+87,
yycrank+47,	yysvec+33,	yyvstop+90,
yycrank+64,	yysvec+33,	yyvstop+93,
yycrank+64,	yysvec+33,	yyvstop+96,
yycrank+24,	yysvec+33,	yyvstop+99,
yycrank+50,	yysvec+33,	yyvstop+102,
yycrank+39,	yysvec+33,	yyvstop+105,
yycrank+70,	yysvec+33,	yyvstop+108,
yycrank+112,	yysvec+33,	yyvstop+111,
yycrank+58,	yysvec+33,	yyvstop+114,
yycrank+66,	yysvec+33,	yyvstop+117,
yycrank+56,	0,		yyvstop+120,
yycrank+0,	0,		yyvstop+122,
yycrank+0,	0,		yyvstop+125,
yycrank+0,	0,		yyvstop+128,
yycrank+0,	0,		yyvstop+130,
yycrank+0,	0,		yyvstop+132,
yycrank+-410,	0,		yyvstop+135,
yycrank+0,	0,		yyvstop+137,
yycrank+0,	0,		yyvstop+139,
yycrank+0,	0,		yyvstop+141,
yycrank+-412,	0,		yyvstop+144,
yycrank+0,	yysvec+14,	yyvstop+146,
yycrank+0,	0,		yyvstop+148,
yycrank+0,	0,		yyvstop+150,
yycrank+-63,	yysvec+18,	yyvstop+152,
yycrank+375,	0,		yyvstop+154,
yycrank+385,	yysvec+19,	yyvstop+156,
yycrank+400,	yysvec+19,	yyvstop+158,
yycrank+0,	0,		yyvstop+160,
yycrank+0,	0,		yyvstop+162,
yycrank+57,	0,		yyvstop+164,
yycrank+0,	0,		yyvstop+166,
yycrank+0,	0,		yyvstop+168,
yycrank+0,	0,		yyvstop+170,
yycrank+0,	0,		yyvstop+172,
yycrank+0,	0,		yyvstop+174,
yycrank+112,	yysvec+26,	yyvstop+176,
yycrank+0,	0,		yyvstop+178,
yycrank+436,	0,		yyvstop+180,
yycrank+0,	yysvec+28,	yyvstop+182,
yycrank+490,	0,		0,	
yycrank+0,	0,		yyvstop+184,
yycrank+0,	0,		yyvstop+186,
yycrank+0,	0,		yyvstop+188,
yycrank+0,	0,		yyvstop+190,
yycrank+0,	yysvec+33,	yyvstop+192,
yycrank+111,	yysvec+33,	yyvstop+194,
yycrank+56,	yysvec+33,	yyvstop+196,
yycrank+0,	yysvec+33,	yyvstop+198,
yycrank+0,	0,		yyvstop+201,
yycrank+0,	0,		yyvstop+203,
yycrank+86,	yysvec+33,	yyvstop+205,
yycrank+83,	yysvec+33,	yyvstop+207,
yycrank+74,	yysvec+33,	yyvstop+209,
yycrank+76,	yysvec+33,	yyvstop+211,
yycrank+82,	yysvec+33,	yyvstop+213,
yycrank+0,	yysvec+33,	yyvstop+215,
yycrank+84,	yysvec+33,	yyvstop+218,
yycrank+96,	yysvec+33,	yyvstop+220,
yycrank+94,	yysvec+33,	yyvstop+222,
yycrank+92,	yysvec+33,	yyvstop+224,
yycrank+102,	yysvec+33,	yyvstop+226,
yycrank+100,	yysvec+33,	yyvstop+228,
yycrank+101,	yysvec+33,	yyvstop+230,
yycrank+0,	yysvec+33,	yyvstop+232,
yycrank+134,	yysvec+33,	yyvstop+235,
yycrank+112,	yysvec+33,	yyvstop+238,
yycrank+124,	yysvec+33,	yyvstop+240,
yycrank+112,	yysvec+33,	yyvstop+242,
yycrank+110,	yysvec+33,	yyvstop+244,
yycrank+126,	yysvec+33,	yyvstop+246,
yycrank+125,	yysvec+33,	yyvstop+248,
yycrank+120,	yysvec+33,	yyvstop+250,
yycrank+127,	yysvec+33,	yyvstop+252,
yycrank+141,	yysvec+33,	yyvstop+254,
yycrank+137,	yysvec+33,	yyvstop+256,
yycrank+155,	yysvec+33,	yyvstop+258,
yycrank+155,	yysvec+33,	yyvstop+260,
yycrank+139,	yysvec+33,	yyvstop+262,
yycrank+354,	yysvec+33,	yyvstop+264,
yycrank+177,	yysvec+33,	yyvstop+266,
yycrank+0,	0,		yyvstop+268,
yycrank+0,	0,		yyvstop+270,
yycrank+0,	0,		yyvstop+272,
yycrank+446,	0,		yyvstop+275,
yycrank+0,	0,		yyvstop+278,
yycrank+0,	0,		yyvstop+281,
yycrank+0,	0,		yyvstop+284,
yycrank+0,	0,		yyvstop+287,
yycrank+0,	0,		yyvstop+290,
yycrank+0,	0,		yyvstop+293,
yycrank+0,	0,		yyvstop+296,
yycrank+0,	0,		yyvstop+299,
yycrank+500,	0,		yyvstop+302,
yycrank+0,	0,		yyvstop+304,
yycrank+525,	yysvec+19,	yyvstop+306,
yycrank+0,	0,		yyvstop+309,
yycrank+535,	0,		0,	
yycrank+0,	yysvec+143,	yyvstop+311,
yycrank+210,	yysvec+33,	yyvstop+313,
yycrank+0,	yysvec+33,	yyvstop+315,
yycrank+174,	yysvec+33,	yyvstop+318,
yycrank+188,	yysvec+33,	yyvstop+320,
yycrank+172,	yysvec+33,	yyvstop+322,
yycrank+209,	yysvec+33,	yyvstop+324,
yycrank+0,	yysvec+33,	yyvstop+326,
yycrank+245,	yysvec+33,	yyvstop+329,
yycrank+247,	yysvec+33,	yyvstop+331,
yycrank+233,	yysvec+33,	yyvstop+333,
yycrank+0,	yysvec+33,	yyvstop+335,
yycrank+233,	yysvec+33,	yyvstop+338,
yycrank+0,	yysvec+33,	yyvstop+340,
yycrank+252,	yysvec+33,	yyvstop+343,
yycrank+244,	yysvec+33,	yyvstop+345,
yycrank+282,	yysvec+33,	yyvstop+347,
yycrank+280,	yysvec+33,	yyvstop+349,
yycrank+0,	yysvec+33,	yyvstop+351,
yycrank+279,	yysvec+33,	yyvstop+354,
yycrank+0,	yysvec+33,	yyvstop+356,
yycrank+285,	yysvec+33,	yyvstop+359,
yycrank+296,	yysvec+33,	yyvstop+361,
yycrank+304,	yysvec+33,	yyvstop+363,
yycrank+315,	yysvec+33,	yyvstop+365,
yycrank+299,	yysvec+33,	yyvstop+367,
yycrank+0,	yysvec+33,	yyvstop+369,
yycrank+312,	yysvec+33,	yyvstop+372,
yycrank+313,	yysvec+33,	yyvstop+374,
yycrank+327,	yysvec+33,	yyvstop+376,
yycrank+335,	yysvec+33,	yyvstop+378,
yycrank+331,	yysvec+33,	yyvstop+380,
yycrank+331,	yysvec+33,	yyvstop+383,
yycrank+348,	yysvec+33,	yyvstop+385,
yycrank+349,	yysvec+33,	yyvstop+387,
yycrank+356,	yysvec+33,	yyvstop+389,
yycrank+556,	0,		yyvstop+391,
yycrank+0,	yysvec+139,	yyvstop+393,
yycrank+387,	yysvec+33,	yyvstop+395,
yycrank+417,	yysvec+33,	yyvstop+397,
yycrank+362,	yysvec+33,	yyvstop+399,
yycrank+371,	yysvec+33,	yyvstop+401,
yycrank+368,	yysvec+33,	yyvstop+403,
yycrank+358,	yysvec+33,	yyvstop+405,
yycrank+0,	yysvec+33,	yyvstop+407,
yycrank+0,	yysvec+33,	yyvstop+410,
yycrank+361,	yysvec+33,	yyvstop+413,
yycrank+363,	yysvec+33,	yyvstop+415,
yycrank+375,	yysvec+33,	yyvstop+418,
yycrank+0,	yysvec+33,	yyvstop+420,
yycrank+362,	yysvec+33,	yyvstop+423,
yycrank+387,	yysvec+33,	yyvstop+425,
yycrank+400,	yysvec+33,	yyvstop+427,
yycrank+0,	yysvec+33,	yyvstop+429,
yycrank+390,	yysvec+33,	yyvstop+432,
yycrank+0,	yysvec+33,	yyvstop+434,
yycrank+395,	yysvec+33,	yyvstop+437,
yycrank+394,	yysvec+33,	yyvstop+439,
yycrank+401,	yysvec+33,	yyvstop+441,
yycrank+0,	yysvec+33,	yyvstop+443,
yycrank+413,	yysvec+33,	yyvstop+446,
yycrank+398,	yysvec+33,	yyvstop+448,
yycrank+414,	yysvec+33,	yyvstop+450,
yycrank+397,	yysvec+33,	yyvstop+452,
yycrank+405,	yysvec+33,	yyvstop+454,
yycrank+417,	yysvec+33,	yyvstop+456,
yycrank+0,	0,		yyvstop+458,
yycrank+0,	yysvec+33,	yyvstop+460,
yycrank+0,	yysvec+33,	yyvstop+463,
yycrank+0,	yysvec+33,	yyvstop+466,
yycrank+0,	yysvec+33,	yyvstop+469,
yycrank+409,	yysvec+33,	yyvstop+472,
yycrank+420,	yysvec+33,	yyvstop+474,
yycrank+418,	yysvec+33,	yyvstop+476,
yycrank+418,	yysvec+33,	yyvstop+478,
yycrank+415,	yysvec+33,	yyvstop+480,
yycrank+0,	yysvec+33,	yyvstop+482,
yycrank+423,	yysvec+33,	yyvstop+485,
yycrank+0,	yysvec+33,	yyvstop+487,
yycrank+427,	yysvec+33,	yyvstop+490,
yycrank+421,	yysvec+33,	yyvstop+493,
yycrank+0,	yysvec+33,	yyvstop+495,
yycrank+416,	yysvec+33,	yyvstop+498,
yycrank+0,	yysvec+33,	yyvstop+500,
yycrank+420,	yysvec+33,	yyvstop+503,
yycrank+427,	yysvec+33,	yyvstop+505,
yycrank+457,	yysvec+33,	yyvstop+507,
yycrank+458,	yysvec+33,	yyvstop+509,
yycrank+0,	yysvec+33,	yyvstop+511,
yycrank+443,	yysvec+33,	yyvstop+514,
yycrank+0,	yysvec+33,	yyvstop+516,
yycrank+0,	yysvec+33,	yyvstop+519,
yycrank+450,	yysvec+33,	yyvstop+522,
yycrank+461,	yysvec+33,	yyvstop+524,
yycrank+0,	yysvec+33,	yyvstop+526,
yycrank+0,	yysvec+33,	yyvstop+529,
yycrank+0,	yysvec+33,	yyvstop+532,
yycrank+461,	yysvec+33,	yyvstop+535,
yycrank+0,	yysvec+33,	yyvstop+537,
yycrank+0,	yysvec+33,	yyvstop+540,
yycrank+450,	yysvec+33,	yyvstop+543,
yycrank+457,	yysvec+33,	yyvstop+545,
yycrank+471,	yysvec+33,	yyvstop+547,
yycrank+483,	yysvec+33,	yyvstop+549,
yycrank+0,	yysvec+33,	yyvstop+551,
yycrank+0,	yysvec+33,	yyvstop+554,
yycrank+0,	yysvec+33,	yyvstop+557,
yycrank+0,	yysvec+33,	yyvstop+560,
yycrank+0,	yysvec+33,	yyvstop+563,
yycrank+0,	yysvec+33,	yyvstop+566,
0,	0,	0};
struct yywork *yytop = yycrank+611;
struct yysvf *yybgin = yysvec+1;
char yymatch[] = {
00  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,011 ,012 ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
011 ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,
'8' ,'8' ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'G' ,
'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,
'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,
'G' ,'G' ,'G' ,01  ,01  ,01  ,01  ,'G' ,
01  ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'G' ,
'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,
'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,
'G' ,'G' ,'G' ,01  ,01  ,01  ,01  ,01  ,
0};
char yyextra[] = {
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0};
#ifndef lint
static	char ncform_sccsid[] = "@(#)ncform 1.6 88/02/08 SMI"; /* from S5R2 1.2 */
#endif

int yylineno =1;
# define YYU(x) x
# define NLSTATE yyprevious=YYNEWLINE
char yytext[YYLMAX];
struct yysvf *yylstate [YYLMAX], **yylsp, **yyolsp;
char yysbuf[YYLMAX];
char *yysptr = yysbuf;
int *yyfnd;
/*extern struct yysvf *yyestate;*/
int yyprevious = YYNEWLINE;
int yylook(void){
	register struct yysvf *yystate, **lsp;
	register struct yywork *yyt;
	struct yysvf *yyz;
	int yych, yyfirst;
	struct yywork *yyr;
# ifdef LEXDEBUG
	int debug;
# endif
	char *yylastch;
	/* start off machines */
# ifdef LEXDEBUG
	debug = 0;
# endif
	yyfirst=1;
	if (!yymorfg)
		yylastch = yytext;
	else {
		yymorfg=0;
		yylastch = yytext+yyleng;
		}
	for(;;){
		lsp = yylstate;
		yyestate = yystate = yybgin;
		if (yyprevious==YYNEWLINE) yystate++;
		for (;;){
# ifdef LEXDEBUG
			if(debug)fprintf(yyout,"state %d\n",yystate-yysvec-1);
# endif
			yyt = yystate->yystoff;
			if(yyt == yycrank && !yyfirst){  /* may not be any transitions */
				yyz = yystate->yyother;
				if(yyz == 0)break;
				if(yyz->yystoff == yycrank)break;
				}
			*yylastch++ = yych = input();
			yyfirst=0;
		tryagain:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"char ");
				allprint(yych);
				putchar('\n');
				}
# endif
			yyr = yyt;
			if ( (int)yyt > (int)yycrank){
				yyt = yyr + yych;
				if (yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				}
# ifdef YYOPTIM
			else if((int)yyt < (int)yycrank) {		/* r < yycrank */
				yyt = yyr = yycrank+(yycrank-yyt);
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"compressed state\n");
# endif
				yyt = yyt + yych;
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				yyt = yyr + YYU(yymatch[yych]);
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"try fall back character ");
					allprint(YYU(yymatch[yych]));
					putchar('\n');
					}
# endif
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transition */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				}
			if ((yystate = yystate->yyother) && (yyt= yystate->yystoff) != yycrank){
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"fall back to state %d\n",yystate-yysvec-1);
# endif
				goto tryagain;
				}
# endif
			else
				{unput(*--yylastch);break;}
		contin:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"state %d char ",yystate-yysvec-1);
				allprint(yych);
				putchar('\n');
				}
# endif
			;
			}
# ifdef LEXDEBUG
		if(debug){
			fprintf(yyout,"stopped at %d with ",*(lsp-1)-yysvec-1);
			allprint(yych);
			putchar('\n');
			}
# endif
		while (lsp-- > yylstate){
			*yylastch-- = 0;
			if (*lsp != 0 && (yyfnd= (*lsp)->yystops) && *yyfnd > 0){
				yyolsp = lsp;
				if(yyextra[*yyfnd]){		/* must backup */
					while(yyback((*lsp)->yystops,-*yyfnd) != 1 && lsp > yylstate){
						lsp--;
						unput(*yylastch--);
						}
					}
				yyprevious = YYU(*yylastch);
				yylsp = lsp;
				yyleng = yylastch-yytext+1;
				yytext[yyleng] = 0;
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"\nmatch ");
					sprint(yytext);
					fprintf(yyout," action %d\n",*yyfnd);
					}
# endif
				return(*yyfnd++);
				}
			unput(*yylastch);
			}
		if (yytext[0] == 0  /* && feof(yyin) */)
			{
			yysptr=yysbuf;
			return(0);
			}
		yyprevious = yytext[0] = input();
		if (yyprevious>0)
			output(yyprevious);
		yylastch=yytext;
# ifdef LEXDEBUG
		if(debug)putchar('\n');
# endif
		}
	}
int yyback(int *p, int m)
{
if (p==0) return(0);
while (*p)
	{
	if (*p++ == m)
		return(1);
	}
return(0);
}
	/* the following are only used in the lex library */
int yyinput(void){
	return(input());
	}
int yyoutput(int c)
{
	output(c);
	}
int yyunput(int c)
{
	unput(c);
	}
