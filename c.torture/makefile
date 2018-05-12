OBJS = crm.a s243.a s26.a s626.a s715.a s7813.a s85.a s9.a \
  s22.a s244.a s4.a s71.a s72.a s81.a s86.a setev.a  \
  s241.a s25.a s61.a s714.a s757.a s84.a s88.a

CC = cc
#CFLAGS = -g -B./gcc- -O -ffloat-store
CFLAGS =

doit: test
	test

test: ${OBJS}
	${CC} -o test *.a -g
