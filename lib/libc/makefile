# BSD UNIX system/C compatibility libraries
# This code is mostly taken from the BSD 4.4 distribution
# and is used in GNO/ME 1.1

o/scandir.a: scandir.c
  compile scandir.c keep=o/scandir

o/ttyslot.a: ttyslot.c
  compile ttyslot.c keep=o/ttyslot

o/getopt.a: getopt.c
  compile getopt.c keep=o/getopt

o/bmem.a: bmem.c
  compile bmem.c keep=o/bmem

o/dirent.a: dirent.c dirent.h
  compile dirent.c keep=o/dirent

o/libc.a: libc.c
  compile libc.c keep=o/libc

o/fts.a: fts.c
  compile fts.c keep=o/fts

o/login.tty.a: login.tty.c
  compile login.tty.c keep=o/login.tty

o/syslog.a: syslog.asm
  compile syslog.asm keep=o/syslog

o/oldlog.a: oldlog.c
  compile oldlog.c keep=o/oldlog

o/getgrent.a: getgrent.c
  compile getgrent.c keep=o/getgrent

o/getpwent.a: getpwent.c
  compile getpwent.c keep=o/getpwent

o/getttyent.a: getttyent.c
  compile getttyent.c keep=o/getttyent

o/login.a: login.c
  compile login.c keep=o/login

o/getpass.a: getpass.c
  compile getpass.c keep=o/getpass

o/crypt.a: crypt.c
  compile crypt.c keep=o/crypt

o/crypta.root: crypta.asm
  compile crypta.asm keep=o/crypta

o/l2.root: l2.asm
  compile l2.asm keep=o/l2

o/misc.a: misc.c
  compile misc.c keep=o/misc

o/parsearg.root: parsearg.asm
  compile parsearg.asm keep=o/parsearg

o/gnocmd.root: gnocmd.asm
  compile gnocmd.asm keep=o/gnocmd

lcrypt: o/crypt.a o/crypta.root
  rm -f lcrypt
  cd o
  makelib ../lcrypt +crypt.a +crypta.root +crypta.a
  cd ..

libc: o/libc.a o/dirent.a o/bmem.a o/getopt.a o/fts.a o/syslog.a \
      o/getgrent.a o/getpwent.a o/getttyent.a o/ttyslot.a o/login.a o/getpass.a \
      o/login.tty.a o/scandir.a o/std.a o/oldlog.a o/misc.a o/l2.a o/parsearg.a \
      o/gnocmd.a
  rm -f libc
  cd o
  makelib ../libc +login.tty.a
  makelib ../libc +bmem.a
  makelib ../libc +dirent.a
  makelib ../libc +getopt.a
  makelib ../libc +libc.a
  makelib ../libc +fts.a
  makelib ../libc +syslog.a
  makelib ../libc +oldlog.a
  makelib ../libc +getgrent.a
  makelib ../libc +getpwent.a
  makelib ../libc +getttyent.a
  makelib ../libc +ttyslot.a
  makelib ../libc +scandir.a
  makelib ../libc +login.a
  makelib ../libc +getpass.a
  makelib ../libc +std.a
  makelib ../libc +gnocmd.a
  makelib ../libc +l2.a
  makelib ../libc +misc.a
  makelib ../libc +parsearg.a
  cd ..
