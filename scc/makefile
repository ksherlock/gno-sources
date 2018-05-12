#t.a: t.c
#  compile t.c keep=t

port.root: port.asm equates
  compile port.asm keep=port

msccf.root: msccf.asm sccf.asm sccf.mac equates
  compile msccf.asm keep=msccf

psccf.root: psccf.asm sccf.asm sccf.mac equates
  compile psccf.asm keep=psccf

modem: port.root msccf.root
  link port msccf /src/sim/libsim keep=modem
  chtyp -t \$bb -a \$7e01 modem
  cp modem /software/dev

printer: port.root psccf.root
  link port psccf /src/sim/libsim keep=printer
  chtyp -t \$bb -a \$7e01 printer
  cp printer /software/dev

#t: t.a sccf.root
#  link t sccf keep=t
#  chtyp -a \$DC00 t
