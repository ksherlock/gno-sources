o/extern.a: extern.c robots.h
  compile extern.c keep=o/extern

o/flush.in.a: flush.in.c
  compile flush.in.c keep=o/flush.in

o/init.field.a: init.field.c robots.h
  compile init.field.c keep=o/init.field

o/main.a: main.c robots.h
  compile main.c keep=o/main

o/make.level.a: make.level.c robots.h
  compile make.level.c keep=o/make.level

o/move.a: move.c robots.h
  compile move.c keep=o/move

o/move.robs.a: move.robs.c robots.h
  compile move.robs.c keep=o/move.robs

o/play.level.a: play.level.c robots.h
  compile play.level.c keep=o/play.level

o/query.a: query.c robots.h
  compile query.c keep=o/query

o/rnd.pos.a: rnd.pos.c robots.h
  compile rnd.pos.c keep=o/rnd.pos

o/score.a: score.c robots.h pathnames.h
  compile score.c keep=o/score

robots: o/MAIN.a o/EXTERN.a o/FLUSH.IN.a o/INIT.FIELD.a \
  o/MAKE.LEVEL.a o/MOVE.a o/MOVE.ROBS.a o/PLAY.LEVEL.a o/QUERY.a \
  o/RND.POS.a o/SCORE.a
  compile linkscr keep=robots
