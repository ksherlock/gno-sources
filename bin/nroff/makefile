nroff.h: version.h
 touch nroff.h

o/main.a: main.c nroff.h
 compile main.c keep=o/main

o/command.a: command.c nroff.h
 compile command.c keep=o/command

o/text.a: text.c nroff.h
 compile text.c keep=o/text

o/io.a: io.c nroff.h
 compile io.c keep=o/io

o/macros.a: macros.c nroff.h
 compile macros.c keep=o/macros

o/strings.a: strings.c nroff.h
 compile strings.c keep=o/strings

o/escape.a: escape.c nroff.h
 compile escape.c keep=o/escape

o/low.a: low.c nroff.h
 compile low.c keep=o/low

nroff: o/main.a o/command.a o/text.a o/io.a o/macros.a o/strings.a o/escape.a o/low.a
 purge
 link o/main o/command o/text o/io o/macros o/strings o/escape o/low keep=nroff
