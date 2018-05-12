data.a: data.c config.h
  compile +O data.c

z.root: z.asm
  compile z.asm

sz.a: sz.c config.h zmodem.h proto.h
  compile sz.c

rz.a: rz.c config.h zmodem.h proto.h
  compile rz.c

zmr.a: zmr.c config.h zmodem.h proto.h
  compile zmr.c

zm.a: zm.c config.h zmodem.h proto.h
  compile zm.c

rbsb.a: rbsb.c config.h zmodem.h proto.h
  compile +w rbsb.c

crctab.a: crctab.c config.h
  compile crctab.c

sz: sz.a zmr.a zm.a rbsb.a crctab.a z.root
  link sz zmr zm rbsb crctab data z keep=sz

rz: rz.a zmr.a zm.a rbsb.a crctab.a z.root
  link rz zmr zm rbsb crctab data z keep=rz
