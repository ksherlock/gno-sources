o/DRIVER.a: DRIVER.C misc.h evaler.h prim.h parser.h lexer.h printer.h garbage.h stack.h
  compile DRIVER.C keep=o/DRIVER

o/LEXER.a: LEXER.C lexer.h evaler.h misc.h stack.h
  compile LEXER.C keep=o/LEXER

o/PARSER.a: PARSER.C evaler.h emitter.h lexer.h parser.h misc.h stack.h
  compile PARSER.C keep=o/PARSER

o/EMITTER.a: EMITTER.C emitter.h parser.h lexer.h misc.h stack.h
  compile EMITTER.C keep=o/EMITTER

o/PRINTER.a: PRINTER.C printer.h evaler.h misc.h
  compile PRINTER.C keep=o/PRINTER

o/GARBAGE.a: GARBAGE.C parser.h evaler.h misc.h stack.h lexer.h
  compile GARBAGE.C keep=o/GARBAGE

o/PRIM.a: PRIM.C misc.h prim.h evaler.h parser.h stack.h garbage.h
  compile PRIM.C keep=o/PRIM

o/MISC.a: MISC.C misc.h stack.h garbage.h evaler.h parser.h
  compile MISC.C keep=o/MISC

o/STACK.a: STACK.C
  compile STACK.C keep=o/STACK

o/EVALER.a: EVALER.C evaler.h parser.h prim.h misc.h stack.h
  compile EVALER.C keep=o/EVALER

gscheme: o/driver.a o/lexer.a o/parser.a o/emitter.a o/printer.a o/garbage.a \
	o/prim.a o/misc.a o/stack.a o/evaler.a
  compile linkscr
