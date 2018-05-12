#
# Makefile for fill, a simple text formatter
#
#	Last changed 1/19/93
#

BINDIR = /usr/public/bin
MANDIR = /usr/catman/pub_man/man1
COPY = /bin/cp

DISTFILES = Makefile fill.c fill.1 test_file

fill:	fill.c
	$(CC) $(CFLAGS) -o fill fill.c

fill.man: fill.1
	nroff -man fill.1 > fill.man

$(BINDIR)/fill: fill
	$(COPY) fill $(BINDIR)/fill
	strip $(BINDIR)/fill

$(MANDIR)/fill.1.z:	fill.man
	-rm $@
	$(COPY) fill.man $(MANDIR)/fill.1
	pack $(MANDIR)/fill.1

install:	$(BINDIR)/fill $(MANDIR)/fill.1.z
	touch install

Lint:	fill.c
	lint -p fill.c >Lint

tar:
	tar cvf Fill.tar $(DISTFILES)
	compress -v Fill.tar

shar:
	shar -o shar $(DISTFILES)
