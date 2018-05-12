# Makefile for Figlet version 2.0 (August 5, 1993).

SHELL = /bin/sh

# Define DEFAULTFONTDIR and DEFAULTFONTFILE to reflect your situation.
# DEFAULTFONTDIR is the full path name of the directory in which Figlet
#   will search first for fonts (the ".flf" files).
# DEFAULTFONTFILE is the filename of the font to be used if no other
#   is specified (standard.flf is recommended, but any other can be
#   used).  This font file should reside in the directory specified by
#   DEFAULTFONTDIR.
# OLD_DIR_STRUCTS should normally be 0.  If Figlet fails to compile
#   properly, try setting OLD_DIR_STRUCTS to 1.  This indicates that an
#   older method of accessing directories should be used.

DEFAULTFONTDIR = /usr/games/lib/figlet.dir
DEFAULTFONTFILE = standard.flf
OLD_DIR_STRUCTS = 0

all: figlet

figlet: figlet.c
	$(CC) $(CFLAGS) -DDEFAULTFONTDIR=\"$(DEFAULTFONTDIR)\"	\
	   -DDEFAULTFONTFILE=\"$(DEFAULTFONTFILE)\"	\
	   -DOLD_DIR_STRUCTS=$(OLD_DIR_STRUCTS) figlet.c -o figlet
