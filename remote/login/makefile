klogin.a: klogin.c pathnames.h
    compile klogin.c keep=klogin

login.a: login.c klogin.c pathnames.h
    compile login.c keep=login

login: login.c klogin.c pathnames.h
    link login klogin keep=login
