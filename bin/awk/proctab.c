#include <stdio.h>
#include "awk.h"
#include "y.tab.h"

static uchar *printname[92] = {
	(uchar *) "FIRSTTOKEN",	/* 257 */
	(uchar *) "PROGRAM",	/* 258 */
	(uchar *) "PASTAT",	/* 259 */
	(uchar *) "PASTAT2",	/* 260 */
	(uchar *) "XBEGIN",	/* 261 */
	(uchar *) "XEND",	/* 262 */
	(uchar *) "NL",	/* 263 */
	(uchar *) "ARRAY",	/* 264 */
	(uchar *) "MATCH",	/* 265 */
	(uchar *) "NOTMATCH",	/* 266 */
	(uchar *) "MATCHOP",	/* 267 */
	(uchar *) "FINAL",	/* 268 */
	(uchar *) "DOT",	/* 269 */
	(uchar *) "ALL",	/* 270 */
	(uchar *) "CCL",	/* 271 */
	(uchar *) "NCCL",	/* 272 */
	(uchar *) "CHAR",	/* 273 */
	(uchar *) "OR",	/* 274 */
	(uchar *) "STAR",	/* 275 */
	(uchar *) "QUEST",	/* 276 */
	(uchar *) "PLUS",	/* 277 */
	(uchar *) "AND",	/* 278 */
	(uchar *) "BOR",	/* 279 */
	(uchar *) "APPEND",	/* 280 */
	(uchar *) "EQ",	/* 281 */
	(uchar *) "GE",	/* 282 */
	(uchar *) "GT",	/* 283 */
	(uchar *) "LE",	/* 284 */
	(uchar *) "LT",	/* 285 */
	(uchar *) "NE",	/* 286 */
	(uchar *) "IN",	/* 287 */
	(uchar *) "ARG",	/* 288 */
	(uchar *) "BLTIN",	/* 289 */
	(uchar *) "BREAK",	/* 290 */
	(uchar *) "CLOSE",	/* 291 */
	(uchar *) "CONTINUE",	/* 292 */
	(uchar *) "DELETE",	/* 293 */
	(uchar *) "DO",	/* 294 */
	(uchar *) "EXIT",	/* 295 */
	(uchar *) "FOR",	/* 296 */
	(uchar *) "FUNC",	/* 297 */
	(uchar *) "SUB",	/* 298 */
	(uchar *) "GSUB",	/* 299 */
	(uchar *) "IF",	/* 300 */
	(uchar *) "INDEX",	/* 301 */
	(uchar *) "LSUBSTR",	/* 302 */
	(uchar *) "MATCHFCN",	/* 303 */
	(uchar *) "NEXT",	/* 304 */
	(uchar *) "ADD",	/* 305 */
	(uchar *) "MINUS",	/* 306 */
	(uchar *) "MULT",	/* 307 */
	(uchar *) "DIVIDE",	/* 308 */
	(uchar *) "MOD",	/* 309 */
	(uchar *) "ASSIGN",	/* 310 */
	(uchar *) "ASGNOP",	/* 311 */
	(uchar *) "ADDEQ",	/* 312 */
	(uchar *) "SUBEQ",	/* 313 */
	(uchar *) "MULTEQ",	/* 314 */
	(uchar *) "DIVEQ",	/* 315 */
	(uchar *) "MODEQ",	/* 316 */
	(uchar *) "POWEQ",	/* 317 */
	(uchar *) "PRINT",	/* 318 */
	(uchar *) "PRINTF",	/* 319 */
	(uchar *) "SPRINTF",	/* 320 */
	(uchar *) "ELSE",	/* 321 */
	(uchar *) "INTEST",	/* 322 */
	(uchar *) "CONDEXPR",	/* 323 */
	(uchar *) "POSTINCR",	/* 324 */
	(uchar *) "PREINCR",	/* 325 */
	(uchar *) "POSTDECR",	/* 326 */
	(uchar *) "PREDECR",	/* 327 */
	(uchar *) "VAR",	/* 328 */
	(uchar *) "IVAR",	/* 329 */
	(uchar *) "VARNF",	/* 330 */
	(uchar *) "CALL",	/* 331 */
	(uchar *) "NUMBER",	/* 332 */
	(uchar *) "STRING",	/* 333 */
	(uchar *) "FIELD",	/* 334 */
	(uchar *) "REGEXPR",	/* 335 */
	(uchar *) "GETLINE",	/* 336 */
	(uchar *) "RETURN",	/* 337 */
	(uchar *) "SPLIT",	/* 338 */
	(uchar *) "SUBSTR",	/* 339 */
	(uchar *) "WHILE",	/* 340 */
	(uchar *) "CAT",	/* 341 */
	(uchar *) "NOT",	/* 342 */
	(uchar *) "UMINUS",	/* 343 */
	(uchar *) "POWER",	/* 344 */
	(uchar *) "DECR",	/* 345 */
	(uchar *) "INCR",	/* 346 */
	(uchar *) "INDIRECT",	/* 347 */
	(uchar *) "LASTTOKEN",	/* 348 */
};


Cell *(*proctab[92])(Node **, int) = {
	nullproc,	/* FIRSTTOKEN */
	program,	/* PROGRAM */
	pastat,	/* PASTAT */
	dopa2,	/* PASTAT2 */
	nullproc,	/* XBEGIN */
	nullproc,	/* XEND */
	nullproc,	/* NL */
	array,	/* ARRAY */
	matchop,	/* MATCH */
	matchop,	/* NOTMATCH */
	nullproc,	/* MATCHOP */
	nullproc,	/* FINAL */
	nullproc,	/* DOT */
	nullproc,	/* ALL */
	nullproc,	/* CCL */
	nullproc,	/* NCCL */
	nullproc,	/* CHAR */
	nullproc,	/* OR */
	nullproc,	/* STAR */
	nullproc,	/* QUEST */
	nullproc,	/* PLUS */
	boolop,	/* AND */
	boolop,	/* BOR */
	nullproc,	/* APPEND */
	relop,	/* EQ */
	relop,	/* GE */
	relop,	/* GT */
	relop,	/* LE */
	relop,	/* LT */
	relop,	/* NE */
	instat,	/* IN */
	arg,	/* ARG */
	bltin,	/* BLTIN */
	jump,	/* BREAK */
	closefile,	/* CLOSE */
	jump,	/* CONTINUE */
	adelete,	/* DELETE */
	dostat,	/* DO */
	jump,	/* EXIT */
	forstat,	/* FOR */
	nullproc,	/* FUNC */
	sub,	/* SUB */
	gsub,	/* GSUB */
	ifstat,	/* IF */
	sindex,	/* INDEX */
	nullproc,	/* LSUBSTR */
	matchop,	/* MATCHFCN */
	jump,	/* NEXT */
	arith,	/* ADD */
	arith,	/* MINUS */
	arith,	/* MULT */
	arith,	/* DIVIDE */
	arith,	/* MOD */
	assign,	/* ASSIGN */
	nullproc,	/* ASGNOP */
	assign,	/* ADDEQ */
	assign,	/* SUBEQ */
	assign,	/* MULTEQ */
	assign,	/* DIVEQ */
	assign,	/* MODEQ */
	assign,	/* POWEQ */
	printstat,	/* PRINT */
	aprintf,	/* PRINTF */
	asprintf,	/* SPRINTF */
	nullproc,	/* ELSE */
	intest,	/* INTEST */
	condexpr,	/* CONDEXPR */
	incrdecr,	/* POSTINCR */
	incrdecr,	/* PREINCR */
	incrdecr,	/* POSTDECR */
	incrdecr,	/* PREDECR */
	nullproc,	/* VAR */
	nullproc,	/* IVAR */
	getnf,	/* VARNF */
	call,	/* CALL */
	nullproc,	/* NUMBER */
	nullproc,	/* STRING */
	nullproc,	/* FIELD */
	nullproc,	/* REGEXPR */
	getline,	/* GETLINE */
	jump,	/* RETURN */
	split,	/* SPLIT */
	substr,	/* SUBSTR */
	whilestat,	/* WHILE */
	cat,	/* CAT */
	boolop,	/* NOT */
	arith,	/* UMINUS */
	arith,	/* POWER */
	nullproc,	/* DECR */
	nullproc,	/* INCR */
	indirect,	/* INDIRECT */
	nullproc,	/* LASTTOKEN */
};

uchar *tokname(int n)
{
	static uchar buf[100];

	if (n < FIRSTTOKEN || n > LASTTOKEN) {
		sprintf(buf, "token %d", n);
		return buf;
	}
	return printname[n-FIRSTTOKEN];
}
