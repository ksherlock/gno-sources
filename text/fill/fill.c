#pragma stacksize 2048
#pragma optimize -1

/* fill - Simple text formatter - 2.7 */
/* jb 1-28-93 : Made sure all funcs were properly prototyped,
                recompiled with ORCA/C 2.0 for vast speed increase */
/*
** Fill is a simple text formatter meant to be used from within
** your editor to provide the same functionality as the ^B command
** in WordStar.  This presumes your editor can pipe an object through
** a filter.  In vi, you would do something like "!}fill" to word wrap
** a paragraph.  Of course, you may, in the spirit of Unix, find other
** uses for it.  For example, fill has the side-effect of de-tabifying
** lines passed to it.
** Usage: fill [-b] [-c | -j] [-d] [-l n] [-r n] [-p n]
**    -b box the output (replying to news)
**    -c center the lines
**    -d the block is left delimited ("** comment block")
**    -j justify the right margin
**    -l n  set left margin to "n", defaults to 1
**    -r n  set right margin to "n", defaults to 72
**    -p n    sets a "paragraph" indent.
*/

/*
** Author:
**   Chad R. Larson        This program is placed in the
**   DCF, Inc.          Public Domain.  You may do with
**   14623 North 49th Place      it as you please.
**   Scottsdale, AZ 85254
*/

/* maximum length of a line (for centering only) */
#define LINE_MAX  512

/* maximum length of a word */
#define WORD_MAX  128

/* the default right margin */
#define DEF_MARGIN   72

/* strings used in boxing output */
#define BOX_MARGIN "| "
#define BOX_STRING "+---------------"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#ifndef __STDC__
#include <memory.h>
#endif

/* forward references */

int   get_word(char *);
char *append(char *, char *);
void  j_line(char *, int);   /* justify a line */
#ifndef __STDC__
void  exit();
char  *append();
char  *malloc();
#endif
#pragma lint -1

/* global flags */
#define FALSE 0
#define TRUE  1 
int   nl_flag; /* new line has been processed */
int   delimited = FALSE;   /* left delimited block */

/* identifier */
#ifndef lint
static char id[] = "@(#)fill, version 2.7";
#endif

/* main program */
int
main(int argc, char *argv[])
{
    int     c;       /* a generic character */
    int     center = FALSE;      /* center text flag */
    int     box = FALSE;      /* box text flag */
    int     justify = FALSE;  /* justify right margin flag */
    int     paragraph = 0;    /* paragraph indent value */
    int     w_length;      /* length of current word */
    int     l_length = 0;     /* length of current line */
    int     l_margin = 1;     /* left margin */
    int     r_margin = DEF_MARGIN;  /* right margin */
    int     wrap_point;    /* max chars allowed on a line */
    char *margin;    /* points to left margin string */
    char *out_line;     /* points to the output line */
    char *delimit;      /* points to the block delimiter */
    char word[WORD_MAX];      /* the current word */
    char *bp;        /* a buffer pointer */
    extern char   *optarg;    /* option argument pointer */

    /* parse the command line */
    while ((c = getopt(argc, argv, "bcdjr:l:p:")) != EOF)
   switch (c) {
   case 'b':
      box = TRUE;
      l_margin += strlen(BOX_MARGIN);
       break;
   case 'c':
      center = TRUE;
       break;
   case 'd':
      delimited = TRUE;
       break;
   case 'j':
      justify = TRUE;
       break;
   case 'r':
       r_margin = atoi(optarg);
       break;
   case 'l':
       l_margin += atoi(optarg) - 1;
       break;
   case 'p':
       paragraph = atoi(optarg);
       break;
   case '?':
       (void)fprintf(stderr,
         "Usage: %s [-b] [-c | -j] [-d] [-l n] [-r n] [-p n]\n", argv[0]);
       exit(-1);
   }

    /* validate command line inputs */
    if (justify && center) {
   (void)fputs("Center and Justify are mutually exclusive.\n", stderr);
   exit(1);
    }
    if (box && center) {
   (void)fputs("Center and Box are mutually exclusive.\n", stderr);
   exit(1);
    }
    if (l_margin >= r_margin || l_margin < 1) {
   (void)fputs("Illogical margin setting.\n", stderr);
   exit(2);
    }
    if (paragraph < 0)     /* silently adjust */
   paragraph = (-paragraph > l_margin ? -l_margin : paragraph);
    else
   paragraph = (paragraph > r_margin ? r_margin : paragraph);

    /* Center the text if requested.  Will exit without filling. */
    if (center) {
   if ( (out_line = malloc(LINE_MAX)) == NULL ) {
       (void)fputs("Unable to allocate centering buffer.\n", stderr);
       exit(3);
   }
   while ( fgets(out_line, LINE_MAX, stdin) != NULL ) {
       bp = out_line;
       while (*bp == ' ' || *bp == '\t')  /* strip leading spaces */
      bp++;
       l_length = strlen(bp) - 1;
       while (bp[l_length - 1] == ' ' || bp[l_length - 1] == '\t')
      l_length--;       /* strip trailing space */
       bp[l_length] = '\0';
       center = (r_margin - l_length) / 2;
       while (center--)
      (void)putc(' ', stdout);
       (void)puts(bp);
   }
   exit(0);
    }

    /* create the left margin string */
    if ( (margin = malloc( (unsigned)l_margin) ) == NULL ) {
   (void)fputs("Unable to allocate space for margin.\n", stderr);
   exit(4);
    }
    (void)memset(margin, ' ', l_margin - 1);
    margin[l_margin - 1] = '\0';
    if (box)
   (void)strncpy(margin, BOX_MARGIN, strlen(BOX_MARGIN));

    /* create a place for the block delimiter, if needed */
    if (delimited && (delimit = malloc(WORD_MAX)) == NULL ) {
   (void)fputs("Unable to allocate delimit buffer.\n", stderr);
   exit(5);
    }

    /* create the output line buffer */
    wrap_point = r_margin - l_margin + 1;
    if ((out_line = malloc( (unsigned)wrap_point + 3) ) == NULL) {
   (void)fputs("Unable to allocate space for line buffer.\n", stderr);
   exit(6);
    }

    /* establish the delimiter */
    if (delimited) {
   if (get_word(delimit) == 0)
       exit(0);
   else {
       (void)strcat(delimit, " ");
       wrap_point -= strlen(delimit);
   }
   /* check for delimiter only thing on line (or in file) */
   if (nl_flag)
       if (get_word(word) == 0) {
      if (box) {
          (void)puts(BOX_STRING);
          (void)fputs(BOX_MARGIN, stdout);
      }
      (void)puts(delimit);
      if (box)
          (void)puts(BOX_STRING);
      exit(0);
       }
    }

    /* move words from the input to the output */
    if (box)
   (void)puts(BOX_STRING);
    bp = out_line;
    wrap_point -= paragraph;  /* one time adjustment */
    while ( (w_length = get_word(word)) != 0 ) {
   if ( (l_length + w_length) > wrap_point ) {  /* line wrap? */
       while (out_line[l_length - 1] == ' ')   /* trailing space strip */
      l_length--;
       out_line[l_length] = '\0';
       if (justify)        /* justify the line? */
      j_line(out_line, wrap_point);
       if (paragraph) {
      wrap_point += paragraph;   /* restore */
      if (paragraph > 0) {
          (void)fputs(margin, stdout);
          while (--paragraph)
         (void)putchar(' ');
      } else {
          c = 0;
          paragraph--;
          while (++paragraph)
         (void)putchar(margin[c++]);
      }
       } else  /* not paragraph */
      (void)fputs(margin, stdout);  /* set any offset */
       if (delimited)
      (void)fputs(delimit, stdout);
       (void)puts(out_line);     /* put the line to stdout */
       *out_line = '\0';         /* reset the output line */
       l_length = 0;
       bp = out_line;
   }
   bp = append(bp, word);
   bp = append(bp, " ");
   l_length += w_length + 1;
   if ((c = word[w_length - 1]) == '.' || c == '?' || c == '!') {
       bp = append(bp, " ");  /* end-of-sentence handling */
       l_length++;
   }
   else if (word[w_length - 1] == '"' &&  /* quoted sentence */
     (c = word[w_length - 2]) == '.' || c == '?' || c == '!') {
       bp = append(bp, " ");
       l_length++;
   }
   if (delimited && nl_flag && get_word(word) == 0)
      break;   /* flush next delimiter, or quit if none */
    }

    /* clean up and exit */
    if (l_length) {     /* residual to flush */
   while (out_line[l_length - 1] == ' ')
       l_length--;
   out_line[l_length] = '\0';
   (void)fputs(margin, stdout);
   if (delimited)
       (void)fputs(delimit, stdout);
   (void)puts(out_line);
    }
    if (box)
   (void)puts(BOX_STRING);
    exit(0);
}

/*
** get_word - a routine to return the next word from the standard input.
** Copies the next word from the input stream to the location pointed to
** by its argument.  The word will be null terminated.  A word is any
** string of characters delimited by whitespace.  Returns the length
** of the word.
*/

int get_word(char *Word)
{
    register int  c; /* generic character */
    register int  i; /* a counter */

    /* first strip any leading whitespace */
    while (TRUE) {
   c = getchar();
   if (c == ' ' || c == '\t')
       continue;
   if (c == '\n' || c == '\f') {
       if (delimited) { /* the delimiter is considered whitespace */
      while ((c = getchar()) == ' ' || c == '\n' ||
        c == '\t' || c == '\f') ;      /* strip more white */
      if (c == EOF) {
          *Word = '\0';
          return 0;
      }
      while ((c = getchar()) != ' ' && c != '\n' &&
        c != '\t' && c != '\f' && c != EOF) ;
      if (c == EOF) {
          *Word = '\0';
          return 0;
      }
       }
       continue;
   }
   if (c == EOF) {
       *Word = '\0';
       return 0;
   }
   break;
    }

    /* copy the word */
    i = 0;
    do {
   *Word++ = c;
   if (++i >= WORD_MAX) {
       (void)fputs("Encountered word too large.\n", stderr);
       exit(7);
   }
   c = getchar();
    } while (c != ' ' && c != '\n' && c != '\t' && c != '\f' && c != EOF);
    *Word = '\0';
    if (c == '\n' || c == '\f')     /* did we just end an input line? */
   nl_flag = TRUE;
    else
   nl_flag = FALSE;
    return i;
}


/*
** Append - like strcpy except returns a pointer to the
** ending null.
*/

char *append(char *s1, char *s2)
{
    while(*s1++ = *s2++);
    return --s1;
}

/*
** j_line - A routine to justify a line.  Attempts to scatter
** inserted white space between calls to avoid "rivers".
*/
void j_line(char *buffer, int margin)
/*char  *buffer; /* the buffer to be justified */
/*int   margin;     /* the margin to which we must stretch */
{
    static unsigned  direction = 0; /* which end to we fill from? */
    static char      *work = NULL;  /* working storage */
    int        insert;     /* count of places to insert */
    int        spaces;     /* count of spaces to insert */
    int        multi;      /* spaces to insert each chance */
    int        extra;      /* count of extra spaces needed */
    int        count;      /* loop counter */
    int        loop;    /* loop counter */
    char    *Ibp;    /* Input buffer pointer */
    char    *Obp;    /* Output buffer pointer */

    /*
    ** Allocate a working storage large enough to hold the line.  We
    ** only do this once (and only if we are justifing).
    */
    if (work == NULL)
   if ((work = malloc( (unsigned)margin + 1 )) == NULL) {
       (void)fputs("Unable to allocate work buffer.\n", stderr);
       exit(8);
   }

    /* how many spaces do we have to insert? */
    loop = strlen(buffer);
    spaces = margin - loop;
    if (spaces == 0)
   return;

    /* find how many opportunities there are for space stuffing */
    Ibp = buffer;
    insert = 0;
    while (loop--) {
   if ( (*Ibp++ == ' ') && (*Ibp != ' ') )   /* edge triggered */
       insert++;
    }
    if (insert == 0)
   return;

    /* how many spaces do we need to stuff per chance? */
    multi = spaces / insert;     /* spaces per slot to insert */
    extra = spaces % insert;     /* extra spaces needed */

    /* copy the buffer contents, inserting spaces */
    direction = ~direction;      /* flip end to fill from */
    (void)strcpy(work, buffer);     /* make a working copy */
    if (direction) { /* spaces go at the left end of the line */
   Ibp = work;
   Obp = buffer;
   loop = strlen(buffer) + 1;
   while (loop--) {
       *Obp++ = *Ibp++;    /* move a character */
       if ((*(Ibp - 1) == ' ') && (*(Ibp - 2) != ' ')) {
      if (extra) {
          extra--;
          *Obp++ = ' ';
      }
      count = multi;
      while (count--)
          *Obp++ = ' ';
       }
   }
    } else {      /* spaces go at the right end of the line */
   loop = strlen(buffer);
   Ibp = work + loop;
   Obp = buffer + loop + spaces;
   *(Obp + 1) = '\0';
   while (loop--) {
       *Obp-- = *Ibp--;
       if ((*(Ibp + 1) == ' ') && (*(Ibp + 2) != ' ')) {
      if (extra) {
          extra--;
          *Obp-- = ' ';
      }
      for (count = multi; count; count--)
          *Obp-- = ' ';
       }
   }
    }
}
