# makefile for RegExp library
#  use ssMake 1.2 - the "official" ShellStuff make utility

# Note- the debug code only adds two routines which can print diagnostic
# information and dump compiled regexp's in a readable form. You want these
# on.

regexp.root:  regexp.c regmagic.h regexp.h
  compile -i regexp.c keep=regexp CC=(-dDEBUG)

regsub.root:  regsub.c regmagic.h regexp.h
  compile -i regsub.c keep=regsub CC=(-dDEBUG)

regerror.root:  regerror.c
  compile -i regerror.c keep=regerror

try.root:  try.c regexp.h
  compile -i try.c keep=try CC=(-dDEBUG)

lregexp: regsub.root regexp.root regerror.root
  delete lregexp
  makelib lregexp +regsub.a
  makelib lregexp +regerror.a
  makelib lregexp +regexp.a

try: try.root lregexp
  link try lregexp keep=try
