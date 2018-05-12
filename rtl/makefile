l1.root: l1.asm
  compile l1.asm keep=l1

t1.root: t1.asm
  compile t1.asm keep=t1

# the -p is REQUIRED; otherwise, LinkIIgs will hang for some reason
tstlib1: l1.root
  linkiigs -p -t rtl -o tstlib1 l1.root

t1: t1.root tstlib1
  linkiigs -t exe -o t1 -lib tstlib1 t1.root

l.a: l.c
  compile l.c keep=l

t.root: t.c
  compile t.c keep=t

# the -p is REQUIRED; otherwise, LinkIIgs will hang for some reason
tstlib: l.a
  linkiigs -p -t rtl -o tstlib l.a

t: t.root tstlib
  linkiigs -t exe -o t -lib tstlib -lib 2/orcalib -lib 2/syslib t.root t.a
