/****************************************************************************

  Figlet (C) 1991, 1993 Glenn Chappell and Ian Chai
  Internet: <ggc@uiuc.edu> and <spectre@uiuc.edu>
  Figlet, along with the various Figlet fonts and documentation, may be
    freely copied and distributed.
  If you use Figlet, please send an e-mail message to
    <spectre@uiuc.edu>.

****************************************************************************/

#define DATE "August 5, 1993"
#define VERSION "2.0"

/* Figlet (Frank, Ian & Glenn's Letters) */
/* by Glenn Chappell */
/* April, 1991 */
/* Automatic file addition by Ian Chai May 1991 */
/* Punctuation and numbers addition by Ian Chai Jan 1993 */
/* Full ASCII by Glenn Chappell Feb 1993 */
/* Line-breaking, general rewrite by Glenn Chappell Mar 1993 */
/* Hard blanks by Glenn Chappell Apr 1993 */
/* Release 2.0 Aug 5 1993 */

/*---------------------------------------------------------------------------
  DEFAULTFONTDIR, DEFAULTFONTFILE and OLD_DIR_STRUCTS should be defined
    in the Makefile.
  DEFAULTFONTDIR is the full path name of the directory in which Figlet
    will search first for fonts (the ".flf" files).
  DEFAULTFONTFILE is the filename of the font to be used if no other
    is specified (standard.flf is recommended, but any other can be
    used). This file should reside in the directory specified by
    DEFAULTFONTDIR.
  OLD_DIR_STRUCTS should normally be 0.  If Figlet fails to compile
    properly, try setting OLD_DIR_STRUCTS to 1.  This indicates that an
    older method of accessing directories should be used.
  In particular, OLD_DIR_STRUCTS = 0 indicates that the file <dirent.h>
    should be included, and that scandir returns a "struct dirent".
    OLD_DIR_STRUCTS = 1 indicates <sys/dir.h> and "struct direct".
---------------------------------------------------------------------------*/
#ifndef DEFAULTFONTDIR
#define DEFAULTFONTDIR "/usr/games/lib/figlet.dir"
#endif
#ifndef DEFAULTFONTFILE
#define DEFAULTFONTFILE "standard.flf"
#endif
#ifndef OLD_DIR_STRUCTS
#define OLD_DIR_STRUCTS 0
#endif

#include <stdio.h>
#ifdef __STDC__
#include <stdlib.h>
#endif
#include <string.h>
#include <ctype.h>
#include <fcntl.h>     /* Needed for get_columns */
#include <sys/ioctl.h> /* Needed for get_columns */
#include <sys/types.h> /* Needed for list_fonts */
#if (OLD_DIR_STRUCTS)
#include <sys/dir.h>   /* Needed for list_fonts */
#else
#include <dirent.h>    /* Needed for list_fonts */
#endif

#define FONTFILESUFFIX ".flf"
#define FONTFILEMAGICNUMBER "flf2"
#define SUFFIXLEN strlen(FONTFILESUFFIX)
#define DEFAULTCOLUMNS 80


/****************************************************************************

  Globals dealing with chars that are read

****************************************************************************/

char *ascline;      /* Alloc'd char ascline[asclinelengthlimit+1]; */
int asclinelength,asclinelengthlimit;


/****************************************************************************

  Globals dealing with chars that are written

****************************************************************************/

char **charlist;    /* Alloc'd char charlist[102*charheight][]; */
char **currchar;
int currcharwidth;
char **currline;    /* Alloc'd char currline[charheight][linewidthlimit+1]; */
int linewidth;


/****************************************************************************

  Globals affected by command line options

****************************************************************************/

int deutschflag,centerflag,paragraphflag;
/*---------------------------------------------------------------------------
  smushmode: (given after "-m" command line switch)
   -2: Get default value from font file (default)
   -1: Do not smush
  For explanation of non-negative values, see smushem().
---------------------------------------------------------------------------*/
int smushmode;
int linewidthlimit;
char *fontdirname,*fontname;


/****************************************************************************

  Globals read from font file

****************************************************************************/

char hardblank;
int charheight,charwidthlimit,defaultmode;


/****************************************************************************

  Name of program, used in error messages

****************************************************************************/

char *myname;


#ifdef TIOCGWINSZ
/****************************************************************************

  get_columns

  Determines the number of columns of /dev/tty.  Returns the number of
  columns, or -1 if error.  May return 0 if columns unknown.
  Requires include files <fcntl.h> and <sys/ioctl.h>.
  by Glenn Chappell & Ian Chai Apr 14 1993

****************************************************************************/

int get_columns()
{
  struct winsize ws;
  int fd,result;

  if ((fd=open("/dev/tty",O_WRONLY))<0) return -1;
  result=ioctl(fd,TIOCGWINSZ,&ws);
  close(fd);
  return result?-1:ws.ws_col;
}
#endif /* ifdef TIOCGWINSZ */


/****************************************************************************

  fontselect

  Used by list_fonts. Returns true if given string ends with
  FONTFILESUFFIX, false if not.

****************************************************************************/

int fontselect(ent)
#if (OLD_DIR_STRUCTS)
struct direct *ent;
#else
struct dirent *ent;
#endif
{
  if (strlen(ent->d_name)<SUFFIXLEN) return 0;
  return !strcmp(ent->d_name+strlen(ent->d_name)-SUFFIXLEN,FONTFILESUFFIX);
}


/****************************************************************************

  list_fonts

  Print font.
  List all files in the given directory with FONTFILESUFFIX.
  Filenames are printed without the suffix.

****************************************************************************/

void list_fonts(font,dir)
char *font,*dir;
{
#if (OLD_DIR_STRUCTS)
  struct direct **namelist;
#else
  struct dirent **namelist;
#endif

  int i,numents;
  char *name;
  extern int alphasort();

  printf("Default font: %s\n", font);
  printf("Font directory: %s\n",dir);
  numents=scandir(dir,&namelist,fontselect,alphasort);

  if (numents<0) {
    puts("Unable to open directory");
    }
  else if (numents==0) {
    puts("No figlet fonts in this directory");
    }
  else {
    puts("Figlet fonts in this directory:");
    for (i=0;i<numents;i++) {
      name=namelist[i]->d_name;
      name[strlen(name)-SUFFIXLEN]='\0';
      puts(name);
      }
    }
}


/****************************************************************************

  my_alloc

  Calls malloc. If malloc returns error, prints error message and
  quits.

****************************************************************************/

#ifdef __STDC__
char *my_alloc(size_t size)
#else
char *my_alloc(size)
int size;
#endif
{
  char *ptr;
#ifndef __STDC__
  extern void *malloc();
#endif

  if ((ptr=(char*)malloc(size))==NULL) {
    fprintf(stderr,"%s: Out of memory\n",myname);
    exit(1);
    }
  else {
    return(ptr);
    }
}


/****************************************************************************

  usageerr

  Prints "Usage: ...." line and exits.

****************************************************************************/

void usageerr()
{
  fprintf(stderr,
    "Usage: %s [ -clnptvDEF ] [ -m smushmode ] [ -w outputwidth ]\n" ,myname);
  fprintf(stderr,"               [ -d fontdirectory ] [ -f fontfile ]\n");
  exit(1);
}


/****************************************************************************

  print_version

  Prints version and copyright message.

****************************************************************************/

void print_version()
{
  printf("Figlet (C) 1991, 1993 Glenn Chappell and Ian Chai\n");
  printf("Internet: <ggc@uiuc.edu> and <spectre@uiuc.edu>\n\n");
  printf(
    "Figlet, along with the various Figlet fonts and documentation, may be\n");
  printf("freely copied and distributed.\n");
  printf("If you use Figlet, please send an e-mail message to\n");
  printf("<spectre@uiuc.edu>.\n\n");
  printf("Version: %s, date: %s\n",VERSION,DATE);
}


/****************************************************************************

  getparams

  Handles all command-line parameters. Assumes "myname" has been set
  to the name of the program. Puts all parameters within bounds except
  smushmode.

****************************************************************************/

void getparams(argc,argv)
int argc;
char *argv[];
{
  extern char *optarg;
  extern int optind;
  int c; /* "Should" be a char -- need int for "!= -1" test*/
  int columns;

  if ((myname=strrchr(argv[0],'/'))!=NULL) {
    myname++;
    }
  else {
    myname = argv[0];
    }
  fontdirname=DEFAULTFONTDIR;
  fontname=(char*)my_alloc(sizeof(char)*(strlen(DEFAULTFONTFILE)+1));
  strcpy(fontname,DEFAULTFONTFILE); /* Some systems don't have strdup() ... */
  if ((strlen(fontname)>=SUFFIXLEN)?
    !strcmp(fontname+strlen(fontname)-SUFFIXLEN,FONTFILESUFFIX):0) {
    fontname[strlen(fontname)-SUFFIXLEN]='\0';
    }
  smushmode= -2;
  deutschflag=0;
  centerflag=0;
  paragraphflag=0;
  linewidthlimit=DEFAULTCOLUMNS-1;
  while ((c=getopt(argc,argv,"DEFclpntvm:w:d:f:"))!= -1) {
    switch (c) {
      case 'D':
        deutschflag=1;
        break;
      case 'E':
        deutschflag=0;
        break;
      case 'F':
        list_fonts(fontname,fontdirname);
        exit(0);
      case 'c':
        centerflag=1;
        break;
      case 'l':
        centerflag=0;
        break;
      case 'p':
        paragraphflag=1;
        break;
      case 'n':
        paragraphflag=0;
        break;
      case 't':
#ifdef TIOCGWINSZ
        columns=get_columns();
        if (columns>0) {
          linewidthlimit=columns-1;
          }
#else /* ifdef TIOCGWINSZ */
        fprintf(stderr,
          "%s: \"-t\" is disabled, since ioctl is not fully implemented.\n",
          myname);
#endif /* ifdef TIOCGWINSZ */
        break;
      case 'v':
        print_version();
        exit(0);
      case 'm':
        smushmode=atoi(optarg);
        /* Putting smushmode within bounds is done in "initialize". */
        break;
      case 'w':
        columns=atoi(optarg);
        if (columns>0) {
          linewidthlimit=columns-1;
          }
        break;
      case 'd':
        fontdirname=optarg;
        break;
      case 'f':
        fontname=optarg;
        if ((strlen(fontname)>=SUFFIXLEN)?
          !strcmp(fontname+strlen(fontname)-SUFFIXLEN,FONTFILESUFFIX):0) {
          fontname[strlen(fontname)-SUFFIXLEN]='\0';
          }
        break;
      default:
        usageerr();
      }
    }
  if (optind!=argc) {
    usageerr();
    }
}


/****************************************************************************

  clear_line

  Clears both the input (ascline) and output (currline) storage.

****************************************************************************/

void clear_line()
{
  int i;

  for (i=0;i<charheight;i++) {
    currline[i][0]='\0';
    }
  linewidth=0;
  ascline[0]='\0';
  asclinelength=0;
}


/****************************************************************************

  initialize

  Allocates memory, initializes variables, and reads in the font.
  Called in main(), just after getparams().

****************************************************************************/

void initialize()
{
  int i,k,cmtlines;
  char *fontpath,*inputline,magicnum[4],endchar;
  FILE *fontfile;

  fontpath=(char*)my_alloc(sizeof(char)
    *(strlen(fontdirname)+strlen(fontname)+SUFFIXLEN+2));
  fontfile=NULL;
  if (!strchr(fontname,'/')) {
    strcpy(fontpath,fontdirname);
    strcat(fontpath,"/");
    strcat(fontpath,fontname);
    strcat(fontpath,FONTFILESUFFIX);
    fontfile=fopen(fontpath,"r");
    }
  if (fontfile==NULL) {
    strcpy(fontpath,fontname);
    strcat(fontpath,FONTFILESUFFIX);
    fontfile=fopen(fontpath,"r");
    if (fontfile==NULL) {
      fprintf(stderr,"%s: %s: Unable to open font file\n",myname,fontpath);
      exit(1);
      }
    }

  fscanf(fontfile,"%4s",magicnum);
  if (strcmp(magicnum,FONTFILEMAGICNUMBER)
    || 5!=fscanf(fontfile,"%*c%c %d %*d %d %d %d"
      ,&hardblank,&charheight,&charwidthlimit,&defaultmode,&cmtlines)) {
    fprintf(stderr,"%s: %s: Not a Figlet 2 font file\n",myname,fontpath);
    exit(1);
    }
  for (i=0;i<=cmtlines;i++) {
    while(k=getc(fontfile),k!='\n'&&k!=EOF) ; /* Advance to end of line */
    }
  free(fontpath);

  if (charheight<1) {
    charheight=1;
    }
  if (charwidthlimit<1) {
    charwidthlimit=1;
    }
  charwidthlimit+=100; /* Give ourselves some extra room */
  if (smushmode<-1) {
    smushmode=defaultmode;
    }
  if (smushmode<-1) {
    smushmode= -1;
    }

  inputline=(char*)my_alloc(sizeof(char)*(charwidthlimit+1));
  charlist=(char**)my_alloc(sizeof(char*)*102*charheight);
  for (i=0;i<102*charheight;i++) {
    if (fgets(inputline,charwidthlimit,fontfile)==NULL) {
      inputline[0]='\0';
      }
    k=strlen(inputline)-1;
    endchar=k<0?'\0':(k==0||inputline[k]!='\n')?inputline[k]:inputline[k-1];
    for(;k>=0?(inputline[k]=='\n' || inputline[k]==endchar):0;k--) {
      inputline[k]='\0';
      }
    charlist[i]=(char*)my_alloc(sizeof(char)*(strlen(inputline)+1));
    strcpy(charlist[i],inputline);
    }
  fclose(fontfile);
  free(inputline);

  currline=(char**)my_alloc(sizeof(char*)*charheight);
  for (i=0;i<charheight;i++) {
    currline[i]=(char*)my_alloc(sizeof(char)*(linewidthlimit+1));
    }
  asclinelengthlimit=linewidthlimit*4+100;
  ascline=(char*)my_alloc(sizeof(char)*(asclinelengthlimit+1));
  clear_line();
  }


/****************************************************************************

  getletter

  Sets currchar to point to the font entry for the given character.
  Sets currcharwidth to the width of this character.

****************************************************************************/

void getletter(c)
int c; /* Need int for Deutsch stuff */
{
  if (deutschflag) {
    if (c>='[' && c<=']') {
      c+=36;
      }
    else if (c >='{' && c <= '~') {
      c+=7;
      }
    }
  currchar=charlist+charheight*(c-' ');
  currcharwidth=strlen(currchar[0]);
}


/****************************************************************************

  smushem

  Given 2 characters, attempts to smush them into 1, according to
  smushmode. Returns smushed character or '\0' if no smushing can be
  done.  Assumes smushmode >= 0.

  smushmode values are sum of following (all values smush blanks):
    1: Smush equal chars (not hardblanks)
    2: Smush '_' with any char in hierarchy below
    4: hierarchy: "|", "/\", "[]", "{}", "()", "<>"
       Each class in hier. can be replaced by later class.
    8: [ + ] -> |, { + } -> |, ( + ) -> |
   16: / + \ -> X, < + > -> X
   32: hardblank + hardblank -> hardblank

****************************************************************************/

char smushem(ch1,ch2)
char ch1,ch2;
{
  if (ch1==' ') return ch2;
  if (ch2==' ') return ch1;

  if (smushmode & 32) {
    if (ch1==hardblank && ch2==hardblank) return ch1;
    }

  if (ch1==hardblank || ch2==hardblank) return '\0';

  if (smushmode & 1) {
    if (ch1==ch2) return ch1;
    }

  if (smushmode & 2) {
    if (ch1=='_' && strchr("|/\\[]{}()<>",ch2)) return ch2;
    if (ch2=='_' && strchr("|/\\[]{}()<>",ch1)) return ch1;
    }

  if (smushmode & 4) {
    if (ch1=='|' && strchr("/\\[]{}()<>",ch2)) return ch2;
    if (ch2=='|' && strchr("/\\[]{}()<>",ch1)) return ch1;
    if (strchr("/\\",ch1) && strchr("[]{}()<>",ch2)) return ch2;
    if (strchr("/\\",ch2) && strchr("[]{}()<>",ch1)) return ch1;
    if (strchr("[]",ch1) && strchr("{}()<>",ch2)) return ch2;
    if (strchr("[]",ch2) && strchr("{}()<>",ch1)) return ch1;
    if (strchr("{}",ch1) && strchr("()<>",ch2)) return ch2;
    if (strchr("{}",ch2) && strchr("()<>",ch1)) return ch1;
    if (strchr("()",ch1) && strchr("<>",ch2)) return ch2;
    if (strchr("()",ch2) && strchr("<>",ch1)) return ch1;
    }

  if (smushmode & 8) {
    if (ch1=='[' && ch2==']') return '|';
    if (ch2=='[' && ch1==']') return '|';
    if (ch1=='{' && ch2=='}') return '|';
    if (ch2=='{' && ch1=='}') return '|';
    if (ch1=='(' && ch2==')') return '|';
    if (ch2=='(' && ch1==')') return '|';
    }

  if (smushmode & 16) {
    if (ch1=='/' && ch2=='\\') return 'X';
    if (ch2=='/' && ch1=='\\') return 'X';
    if (ch1=='<' && ch2=='>') return 'X';
    if (ch2=='<' && ch1=='>') return 'X';
    }

  return '\0';
}


/****************************************************************************

  smushamt

  Returns the maximum amount that the current character can be smushed
  into the current line.

****************************************************************************/

int smushamt()
{
  int maxsmush,amt;
  int row,left,right;
  char ch1,ch2;

  if (smushmode== -1) {
    return 0;
    }
  maxsmush = currcharwidth;
  for (row=0;row<charheight;row++) {
    for (left=strlen(currline[row]);
      ch1=currline[row][left],(left>0&&(!ch1||ch1==' '));left--) ;
    for (right=0;ch2=currchar[row][right],ch2==' ';right++) ;
    amt = right+linewidth-1-left;
    if (!ch1||ch1==' ') {
      amt++;
      }
    else if (ch2) {
      if (smushem(ch1,ch2)!='\0') {
        amt++;
        }
      }
    if (amt<maxsmush) {
      maxsmush = amt;
      }
    }
  return maxsmush;
}


/****************************************************************************

  add_a_char

  Attempts to add the given character onto the end of the current line.
  Returns 1 if this can be done, 0 otherwise.

****************************************************************************/

int add_a_char(c)
char c;
{
  int smushamount,row,k;

  getletter(c);
  smushamount = smushamt();
  if (linewidth+currcharwidth-smushamount>linewidthlimit
      ||asclinelength+1>asclinelengthlimit) {
    return 0;
    }
  for (row=0;row<charheight;row++) {
    for (k=0;k<smushamount;k++) {
      currline[row][linewidth-smushamount+k] =
        smushem(currline[row][linewidth-smushamount+k],currchar[row][k]);
      }
    strcat(currline[row],currchar[row]+smushamount);
    }
  linewidth = strlen(currline[0]);
  ascline[asclinelength++]=c;
  ascline[asclinelength]='\0';
  return 1;
}


/****************************************************************************

  putstring

  Prints out the given null-terminated string, substituting blanks
  for hardblanks. Prints a newline at the end of the string. The
  string is centered (taking linewidthlimit+1 as the screen width) if
  centerflag is set.

****************************************************************************/

void putstring(string)
char *string;
{
  int i,len;

  len = strlen(string);
  if (centerflag) {
    for (i=1;2*i+len-1<=linewidthlimit;i++) {
      putchar(' ');
      }
    }
  for (i=0;i<len;i++) {
    putchar(string[i]==hardblank?' ':string[i]);
    }
  putchar('\n');
}


/****************************************************************************

  print_line

  Prints out the current line using putstring, then clears the current
  line.

****************************************************************************/

void print_line()
{
  int i;

  for (i=0;i<charheight;i++) {
    putstring(currline[i]);
    }
  clear_line();
}


/****************************************************************************

  split_line

  Splits ascline at the last word break (bunch of consecutive blanks).
  Makes a new line out of the first part and prints it using
  print_line.  Makes a new line out of the second part and returns.

****************************************************************************/

void split_line()
{
  int i,gotspace,lastspace,len;
  char *part1,*part2;

  part1=(char*)my_alloc(sizeof(char)*(asclinelength+1));
  part2=(char*)my_alloc(sizeof(char)*(asclinelength+1));
  gotspace=0;
  for (i=asclinelength-1;i>=0;i--) {
    if (!gotspace && ascline[i]==' ') {
      gotspace=1;
      lastspace=i;
      }
    if (gotspace && ascline[i]!=' ') {
      break;
      }
    }
  ascline[i+1]='\0';
  strcpy(part1,ascline);
  strcpy(part2,ascline+lastspace+1);
  clear_line();
  len=strlen(part1);
  for (i=0;i<len;i++) {
    add_a_char(part1[i]);
    }
  print_line();
  len=strlen(part2);
  for (i=0;i<len;i++) {
    add_a_char(part2[i]);
    }
  free(part1);
  free(part2);
}


/****************************************************************************

  main

  The main program, of course.
  Reads characters 1 by 1 from stdin, and makes lines out of them using
  add_a_char. Handles line breaking, (which accounts for most of the
  complexity in this function).

****************************************************************************/

void main(argc,argv)
int argc;
char *argv[];
{
  int c,c2;
  int i;
  int last_was_eol_flag;
/*---------------------------------------------------------------------------
  wordbreakmode:
    -1: /^$/ and blanks are to be absorbed (when line break was forced
      by a blank or character larger than linewidthlimit)
    0: /^ *$/ and blanks are not to be absorbed
    1: /[^ ]$/ no word break yet
    2: /[^ ]  *$/
    3: /[^ ]$/ had a word break
---------------------------------------------------------------------------*/
  int wordbreakmode;
  int char_not_added;

  getparams(argc,argv);
  initialize();

  wordbreakmode = 0;
  last_was_eol_flag=0;

  do {
    c=getchar();

    if (isspace(c)) {
      if (c=='\t'||c==' ') {
        c=' ';
        }
      else if (c=='\n') {
        if (paragraphflag&&!last_was_eol_flag) {
          ungetc(c2=getchar(),stdin);
          c=(isspace(c2)?'\n':' ');
          }
        }
      else {
        c='\n';
        }
      }
    last_was_eol_flag=(c=='\n');

    if (!isprint(c) && c!='\n' && c!=EOF) continue;

    do {
      char_not_added = 0;

      if (wordbreakmode== -1) {
        if (c==' ') {
          break;
          }
        else if (c=='\n') {
          wordbreakmode = 0;
          break;
          }
        wordbreakmode = 0;
        }

      if (c=='\n' || (c==EOF && linewidth!=0)) {
        print_line();
        wordbreakmode = 0;
        }

      else if (c==EOF) ;

      else if (add_a_char(c)) {
        if (c!=' ') {
          wordbreakmode = (wordbreakmode>=2)?3:1;
          }
        else {
          wordbreakmode = (wordbreakmode>0)?2:0;
          }
        }

      else if (linewidth==0) {
        for (i=0;i<charheight;i++) {
          putstring(currchar[i]);
          }
        wordbreakmode= -1;
        }

      else if (c==' ') {
        if (wordbreakmode==2) {
          split_line();
          }
        else {
          print_line();
          }
        wordbreakmode= -1;
        }

      else {
        if (wordbreakmode >= 2) {
          split_line();
          }
        else {
          print_line();
          }
        wordbreakmode=(wordbreakmode==3)?1:0;
        char_not_added = 1;
        }

      } while (char_not_added);
    } while (c!=EOF);
}
