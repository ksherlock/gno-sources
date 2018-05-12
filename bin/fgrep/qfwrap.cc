/* Quick fixed grep program v1.1. Written by Christopher Neufeld, copyright
   1992.

   This code builds successfully with ORCA/C v1.3 and ORCA/M 2.0 under
   both the ORCA and GNO/ME shells. The executable generated contains
   runtime libraries copyrighted by the Byte Works. Used with permission.

   This messy code is entirely my responsibility. It was generated using
   only the FGREP(1V) manual page shipped with GNO. Any weirdness or bugs
   here are due to my coding, not to sources grabbed from somewhere else    */

/* Available to anybody and everybody. Use as you see fit. If this program
   does anything weird, like killing your tropical fish or invoking
   demons in your refrigerator I will not take responsibility (my lawyer
   friends said I should put that in).

   Interested persons are welcome to try to port this, but be aware that
   all the real work is done in the 65816 assembly code fragments.          */
   
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>

#pragma memorymodel 1
#pragma debug 0
#pragma optimize -1

#pragma keep "qfwrap"

#define VERSIONSTR "v1.1"

#define SLEN 100
#define BUFLEN 32768u
#define ABORT -2
#define NOTFOUND -1
#define TRUE 1
#define CRET 0x0d
#define LF 0x0a

#define CNTMAT   0x80
#define NOPRNAME 0x40
#define NAMESO   0x20
#define LNUMS    0x10
#define SILENT   0x08
#define INVERT   0x04
#define EXACT    0x02
#define EXPRSN   0x01

#define CHKMEM(x) if ((x) == NULL) {\
        fprintf(stderr, "Memory allocation failure in qfgrep\n");\
        exit(2);\
}

extern int qfgrep(char stat, char *buf, unsigned int nread,
                  unsigned long *lcnt, char *str1, char *str2, char **ostr);
extern int nsqfgrep(char stat, char *buf, unsigned int nread,
                    unsigned long *lcnt, char **str1, char **str2, char **ostr);
extern void qinit(unsigned int slen);
extern void nsqinit(int fcnt, int *slen);

char s1[SLEN], s2[SLEN];        /* It's out here to save stack */


void usage(char *name)
{
  fprintf(stderr, "Quick fgrep %s by Christopher Neufeld, copyright 1992\n",
          VERSIONSTR);
  fprintf(stderr, "The executable for this program contains linked library\n");
  fprintf(stderr, "      files copyrighted by Byte Works. Used with permission\n\n");
  fprintf(stderr, "Usage: %s [-chilnsvx] [-e string ] [-f filename ]\n", name);
  fprintf(stderr, "             [ string ] [ file ... ]\n");
}

int main(int argc, char **argv)
{
  char *buf, *ch, *ostr;
  int numread;
  int retval, prnames, fallback, i, ii, j, k, rtn, fromstdin, fildes;
  FILE *ifile, *ffile;
  char **fstr1, **fstr2;
  int fcnt, fcnt2, *flens;
  char statbyte, ignorecase, fromfile;
  unsigned long lcnt, matcnt;

  statbyte = ignorecase = fromfile = 0;
  k = 1;
  while (argv[k][0] == '-' && !(statbyte & EXPRSN) && !fromfile) {
    if (argv[k][1] == 0) {
      usage(argv[0]);
      exit(2);
    }
    for (j=1;j<strlen(argv[k]);j++)
      switch (argv[k][j]) {
        case 'c': statbyte |= CNTMAT;
                  break;
        case 'h': statbyte |= NOPRNAME;
                  break;
        case 'i': ignorecase = 1;
                  break;
        case 'l': statbyte |= NAMESO;
                  break;
        case 'n': statbyte |= LNUMS;
                  break;
        case 's': statbyte |= SILENT;
                  break;
        case 'v': statbyte |= INVERT;
                  break;
        case 'x': statbyte |= EXACT;
                  break;
        case 'e': if (j == 1 && argv[k][2] == 0)
                    statbyte |= EXPRSN;
                  else {
                    usage(argv[0]);
                    exit(2);
                  }
                  break;
        case 'f': if (j == 1 && argv[k][2] == 0)
                    fromfile = 1;
                  else {
                    usage(argv[0]);
                    exit(2);
                  }
                  break;
        default: fprintf(stderr, "Unrecognized switch: -%c\n", argv[k][j]);
                 usage(argv[0]);
                 exit(2);
      }
    k++;
  }         
  if (argc < k+1 ||
      ((statbyte & CNTMAT) && (statbyte & LNUMS)) ||
      ((statbyte & NOPRNAME) && (statbyte & NAMESO)) ||
      (!(statbyte & CNTMAT) + !(statbyte & LNUMS) + !(statbyte & NAMESO) < 2) ||
      ((statbyte & EXPRSN) && fromfile)) {
    usage(argv[0]);
    return 2;
  }
  prnames = argc > k+2 ? (1 && !(statbyte & NOPRNAME)) : 0;
  fromstdin = (argc == k+1) ? 1 : 0;
  if (fromfile) {
    if ((ffile = fopen(argv[k], "r")) == NULL) {
      fprintf(stderr, "Qfgrep: Unable to open: %s\n", argv[k]);
      exit(2);
    }
    fcnt = 0;
    while (!feof(ffile))
      fcnt += (fgets(s1, SLEN-1, ffile) != NULL);
    CHKMEM(fstr1 = (char **) malloc(fcnt * sizeof(char *)))
    CHKMEM(fstr2 = (char **) malloc(fcnt * sizeof(char *)))
    for (i=0;i<fcnt;i++) {
      CHKMEM(fstr1[i] = (char *) malloc(SLEN * sizeof(char)))
      CHKMEM(fstr2[i] = (char *) malloc(SLEN * sizeof(char)))
    }
    CHKMEM(flens = (int *) malloc(fcnt * sizeof(int)))
    rewind(ffile);
    fcnt2 = fcnt;
    for (i=0;i<fcnt;i++) {
      fgets(s1, SLEN-1, ffile);
      ch = s1 + strlen(s1) - 1;
      if (*ch == CRET || *ch == LF) {
      	*ch = 0;
      }
      if (ignorecase) {
      	flens[i] = strlen(s1);
        for (j=0;j<flens[i];j++) {
          fstr1[i][j] = tolower(s1[j]);
          fstr2[i][j] = toupper(s1[j]);
        }
        fstr1[i][j] = fstr2[i][j] = 0;
      } else {
        strcpy(fstr1[i], s1);
        strcpy(fstr2[i], s1);
      }
      for (ii=0;ii<i;ii++)   /* Check for and remove duplicate strings */
        if (!strcmp(fstr1[i], fstr1[ii]) && !strcmp(fstr2[i], fstr2[ii])) {
          i--;
          fcnt--;
        }
    }
    fclose(ffile);
  } else {
    if (ignorecase) {
      for (i=0;i<strlen(argv[k]);i++) {
        if (i > SLEN-2) break;
        s1[i] = tolower(argv[k][i]);
        s2[i] = toupper(argv[k][i]);
      }
      s1[i] = s2[i] = 0;
    } else {
      strncpy(s1, argv[k], SLEN-1);
      strncpy(s2, argv[k], SLEN-1);
      s1[SLEN-1] = s2[SLEN-1] = 0;
    }
  }
  if (fromfile && fcnt == 1) {
    strcpy(s1, fstr1[0]);
    strcpy(s2, fstr2[0]);
    fromfile = 0;
    for (i=0;i<fcnt2;i++) {
      free(fstr1[i]);
      free(fstr2[i]);
    }
    free(fstr1);
    free(fstr2);
  }
  rtn = 1;
  CHKMEM(buf = (char *) malloc(BUFLEN * sizeof(char)))
  for (i=k+1;i<argc || fromstdin;i++) {
    fallback = 0;
    if (!fromstdin && (ifile = fopen(argv[i], "r")) == NULL) {
      fprintf(stderr, "Qfgrep: Unable to open: %s\n", argv[i]);
      continue;
    }
    lcnt = matcnt = 0;
    if (fromfile) nsqinit(fcnt, flens);
    else qinit(strlen(s1));
    fildes = (fromstdin) ? STDIN_FILENO : ifile->_file;
    do {
      numread = read(fildes, buf + BUFLEN / 2, BUFLEN / 2);
      do {
        if (fromfile)
          retval = nsqfgrep(statbyte, buf, BUFLEN / 2 + numread,
                            &lcnt, fstr1, fstr2, &ostr);
        else
          retval = qfgrep(statbyte, buf, BUFLEN / 2 + numread,
                          &lcnt, s1, s2, &ostr);
        if (retval == ABORT) {
          fprintf(stderr, "Fatal error processing: %s\n", argv[i]);
          fallback = 1;
          break;
        }
        if (retval == NOTFOUND) break;
        rtn = 0;
        if (statbyte & SILENT) continue;
        if (statbyte & NAMESO) {
          fallback = 1;
          printf("%s\n", argv[i]);
          break;
        }
        if (statbyte & CNTMAT) {
          matcnt++;
          continue;
	}
        if (prnames) {
          if (statbyte & LNUMS) 
            printf("%lu:%s: %s\n", lcnt, argv[i], ostr);
          else
            printf("%s: %s\n", argv[i], ostr);
        } else {
          if (statbyte & LNUMS)
            printf("%lu: %s\n", lcnt, ostr);
          else
            printf("%s\n", ostr);
        }
      } while (TRUE);
      if (fallback) break;
    } while (numread == BUFLEN / 2);
    if ((statbyte & CNTMAT) && !(statbyte & SILENT)) {
      if (prnames) {
        printf("%s: %lu\n", argv[i], matcnt);
      } else {
        printf("%lu\n", matcnt);
      }
    }
    if (!fromstdin) {
      fclose(ifile);
    } else break;
  }
  free(buf);
  if (fromfile) {
    for (i=0;i<fcnt2;i++) {
      free(fstr1[i]);
      free(fstr2[i]);
    }
    free(fstr1);
    free(fstr2);
  }
  return rtn;
}
