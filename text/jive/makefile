valspeak: valspeak.l
  conv -crlf valspeak.l
  flex valspeak.l
  chtyp -l cc lex.yy.c
  mv lex.yy.c valspeak.c
  cmpl -i valspeak.c
  rm *.a *.root

jive: jive.l
  conv -crlf jive.l
  flex jive.l
  chtyp -l cc lex.yy.c 
  mv lex.yy.c jive.c
  cmpl -i jive.c
  rm *.a *.root
