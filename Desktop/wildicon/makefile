wi.a: wi.c
  compile wi.c keep=wi

regexp.root: regexp.asm regexp.mac
  compile regexp.asm keep=regexp

wia.root: wia.asm
  compile wia.asm keep=wia

wi: wi.a regexp.root wia.root
  link wi wia regexp keep=wi
  duplicate -R wi.res wi
  chtyp -t \$BC -a 1 wi
  cp wi \*:system:finderextras
