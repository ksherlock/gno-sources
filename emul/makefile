o/emulate.a: emulate.c screen.h tcap.h
  compile emulate.c keep=o/emulate

o/main.a: main.c screen.h
  compile main.c keep=o/main

o/screen.a: screen.c screen.h tcap.h
  compile screen.c keep=o/screen

o/tcap.a: tcap.c tcap.h
  compile tcap.c keep=o/tcap

evt: o/EMULATE.A o/MAIN.A o/SCREEN.A o/TCAP.A
  link o/main o/emulate o/screen o/tcap 2/direct256 keep=evt
