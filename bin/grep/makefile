regexp.root: regexp.asm
 compile regexp.asm keep=regexp

grep.root: grep.cc
 compile grep.cc keep=grep CC=(-dGREP)

egrep.root: grep.cc
 compile grep.cc keep=egrep CC=(-dEGREP)

egrep: egrep.root
 link egrep 2/lregexp 2/direct256 keep=egrep

grep: grep.root
 link grep regexp 2/direct256 keep=grep
