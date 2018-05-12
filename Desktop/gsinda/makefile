pipe.a: pipe.c pipe.h
    compile pipe.c keep=pipe
    rm -f pipe.root

main.a: main.c gsi.h pipe.h
    compile main.c keep=main

gsi: main.a pipe.c gsi.h pipe.h
    link main pipe keep=gsi
    chtyp -t nda gsi
    cp gsi \*:system:desk.accs
