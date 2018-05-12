/*
 * touch.c
 *
 * v1.1 (jb) Reset stacksize
 * Original version by Derek Taubert, summer 1991
 */

#pragma stacksize 1024
#pragma optimize -1

#include <types.h>
#include <stdio.h>
#include <misctool.h>
#include <GSOS.h>
#include <getopt.h>

void bzero(void *, size_t);

int main(int argc,char **argv)
{
static GSString255 filename;
static FileInfoRecGS info;
static TimeRec current;
extern char *optarg;
extern int optind;
extern int getopt(int,char **,char *);
int c,errflg = 0;

    optarg = NULL; optind = 0;    /* this makes getopt restartable */

    while ((c = getopt(argc,argv, "")) != EOF) {
        switch (c) {
            default : errflg++;
        }
    }
    argv += optind;
    if (errflg || (optind == argc)) {
        fprintf(stderr,"usage: touch file...\n");
        exit(2);
    }
    info.pCount = 7;
    info.pathname = &filename;
    for (;optind<argc;optind++) {
        strcpy(filename.text,*argv); filename.length=strlen(*(argv++));
        GetFileInfoGS(&info);
        if (toolerror()) {
            fprintf(stderr,"error getting %s\n",filename.text);
            exit(1);
        }
        printf("%s last modified %2d-%d-%04d %d:%02d\n",filename.text,
          info.modDateTime.month+1,info.modDateTime.day+1,
          info.modDateTime.year+1900,info.modDateTime.hour,
          info.modDateTime.minute);
        info.storageType = 0;
        current = ReadTimeHex();
        if (*(unsigned long *)(((char *)&current)+1) ==
            *(unsigned long *)(((char *)&(info.modDateTime))+1)) {

/* this is a hack because prodos doesn't keep track of seconds, and if you
   touch a file in the same minute, it wouldn't do any good.  This above
   hack doesn't check month, but oh well..                                   */

            printf("    same minute, date tweaked\n");
            info.modDateTime.minute++;
        } else {
            bzero(&(info.modDateTime),sizeof(TimeRec));
        }
        SetFileInfoGS(&info);
        if (toolerror()) {
            fprintf(stderr,"error setting %s\n",filename.text);
            exit(1);
        }
    }
}
