o/b.a: b.c awk.h proto.h y.tab.h
  compile b.c keep=o/b

o/main.a: main.c awk.h proto.h y.tab.h
  compile main.c keep=o/main

o/maketab.a: maketab.c awk.h proto.h y.tab.h
  compile maketab.c keep=o/maketab

o/parse.a: parse.c awk.h proto.h y.tab.h
  compile parse.c keep=o/parse

o/lib.a: lib.c awk.h proto.h y.tab.h
  compile lib.c keep=o/lib

o/run.a: run.c awk.h proto.h y.tab.h
  compile run.c keep=o/run

o/tran.a: tran.c awk.h proto.h y.tab.h
  compile tran.c keep=o/tran

o/y.tab.a: y.tab.c awk.h proto.h
  compile y.tab.c keep=o/y.tab

o/lex.yy.a: lex.yy.c awk.h proto.h
  compile lex.yy.c keep=o/lex.yy

o/proctab.a: proctab.c awk.h proto.h
  compile proctab.c keep=o/proctab

awk: o/main.root o/b.a o/parse.a o/lib.a o/run.a o/tran.a o/y.tab.a o/lex.yy.a o/proctab.a
  link o/main o/b o/parse o/lib o/run o/tran o/y.tab o/lex.yy o/proctab keep=awk
