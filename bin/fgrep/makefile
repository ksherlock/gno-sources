qfgrep.macros: qfgrep.asm
  macgen -C qfgrep.asm qfgrep.macros 13:orcainclude:m=

qfwrap.a: qfwrap.cc
  compile qfwrap.cc keep=qfwrap

qfgrep.a: qfgrep.asm qfgrep.macros
  compile qfgrep.asm keep=qfgrep

fgrep: qfgrep.a qfwrap.a
  link qfwrap qfgrep keep=fgrep
