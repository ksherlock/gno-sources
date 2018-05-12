inst.root: inst.c
  compile +w inst.c keep=inst

install.root: install.c
  compile +w install.c keep=install

#install: install.root
#  link install keep=install

inst: inst.root
  link inst install keep=install
  duplicate -R install.tmp install
  chtyp -t s16 install
