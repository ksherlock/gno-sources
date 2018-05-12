/* CONV:
 *   Originally by Jawaid Bazyar (bazyar@cs.uiuc.edu).
 *   Re-written from scratch by Greg Thompson (gt0t+@andrew.cmu.edu).
 *   Completed April 8, 1991.
 *
 *   v1.2  now prints an error if a specified file does not exist
 *         changed all error messages to use stderr
 *   v1.1  fixed senseless "Parameter out of Range" errors (jb)
 *
 *   Differences between this version and Jawaid's:
 *     -0001 added: converts all 0x00 to 0x01 for sound files
 *     -crlf and -lfcr no longer create temp files, and are considerably
 *                     faster (~10.5K per second).
 *     -detab now creates temp file in same directory as file to be
 *            converted.  Resulting file has same filetype, auxtype, and
 *            access as original file.  Minor speed increase.
 */
#pragma keep "conv"
#pragma stacksize 1024
#pragma optimize -1

#include <types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gsos.h>
#include <memory.h>
#include <shell.h>
#include <ctype.h>

extern pascal void DebugStr() inline(0x09FF,dispatcher);
/* for GSBug.  Call with pointer to Str255 */

Word MyID;
int err, tabColumns;

void usage(char *name)
{
  fprintf(stderr,"usage: %s -<convspec> datafile.\n", name);
  fprintf(stderr,"  <convspec> is one of the following:\n");
  fprintf(stderr,"    crlf  - convert CR to LF\n");
  fprintf(stderr,"    lfcr  - convert LF to CR\n");
  fprintf(stderr,"    0001  - convert 0x00 to 0x01 (useful for using Mac sound\
 files)\n");
  fprintf(stderr,"    lower - change the filename to lowercase\n");
  fprintf(stderr,"    detab <col> - convert tabs to spaces (tab stop every COL\
 columns)\n");
  exit(-1);
}

void puke(char *code)
{
  fprintf(stderr,"conv: error 0x%X at %s", err, code);
  MMShutDown(MyID);
  exit(-1);
}

void close(Word refNum)
{
  OpenRecGS *ORec;

  ORec = (OpenRecGS *) malloc(sizeof *ORec);
  ORec->pCount = 1;
  ORec->refNum = refNum;
  CloseGS(ORec);
  if (err = toolerror())
    puke("CloseGS");
  free((void *) ORec);
}

LongWord openfile(char *file, Word *number)
{
  OpenRecGS *ORec;
  LongWord length;

  ORec = (OpenRecGS *) malloc (sizeof *ORec);
  ORec->pathname = (GSString255 *) malloc (sizeof(GSString255));
  ORec->optionList = NULL;
  ORec->pCount = 12;
  strcpy(ORec->pathname->text, file);
  ORec->pathname->length = strlen(ORec->pathname->text);
  ORec->requestAccess = readWriteEnable;
  ORec->resourceNumber = 0;
  OpenGS(ORec);
  if (err=toolerror())
    puke("OpenGS");
  *number = ORec->refNum;
  length = ORec->eof;
  free((void *) ORec->pathname);
  free((void *) ORec);
  return length;
}
  
void readfile(Word refNum, Handle filebuf, LongWord length)
{
  IORecGS  *RRec;

  RRec = (IORecGS *) malloc(sizeof *RRec);
  RRec->pCount = 5;
  RRec->refNum = refNum;
  RRec->dataBuffer = *filebuf;
  RRec->requestCount = length;
  RRec->cachePriority = cacheOff;
  ReadGS(RRec);
  if (err=toolerror())
    puke("ReadGS");
  if (RRec->transferCount != length)
    printf("Bytes requested: %ld.  Bytes transferred: %ld.\n", length,
            RRec->requestCount);
  free((void *) RRec);
}

void swap(Pointer data, LongWord length, char from, char to)
{
  LongWord a;

  for (a=length; a > 0; a--, data++)
    if (*data == from)
      *data = to;
}

void write(Word refNum, Handle data, LongWord length)
{
  IORecGS  *WRec;
  SetPositionRecGS  MRec = {3, 0, markMinus, 0l};

  MRec.refNum = refNum;
  MRec.displacement = length;
  SetMarkGS(&MRec);
  if (err=toolerror())
    puke("SetMarkGS");
  WRec = (IORecGS *) malloc(sizeof *WRec);
  WRec->pCount = 5;
  WRec->refNum = refNum;
  WRec->dataBuffer = *data;
  WRec->requestCount = length;
  WRec->cachePriority = cacheOff;
  WriteGS(WRec);
  if (err=toolerror())
    puke("WriteGS");
  if (WRec->transferCount != length)
    printf("Bytes requested: %ld.  Bytes transferred: %ld.\n", length,
            WRec->transferCount);
  free((void *) WRec);
}

void swapfunc(Word refNum, FILE *out, Handle databuf, LongWord maxmem,
              char from, char to)
{
  readfile(refNum, databuf, maxmem);
  swap(*databuf, maxmem, from, to);
  write(refNum, databuf, maxmem);
}

void tabfunc(Word refNum, FILE *out, Handle databuf, LongWord maxmem,
             char from, char to)
{
  Pointer data;
  LongWord a;
  int i, curcolumn;

  readfile(refNum, databuf, maxmem);
  data = *databuf;

  for (a=maxmem; a > 0; a--, data++)
    switch (*data) {
    case '\t':
      for (i = (curcolumn % tabColumns); i < tabColumns; i++) {
        fputc(' ', out);
        curcolumn++;
      }
      break;
    case '\n':
    case '\r':
      curcolumn = -1;
    default:
      curcolumn++;
      fputc(*data, out);
    }
}  

void convert(Word refNum, FILE *out, LongWord length, char from, char to,
             void (*doit)(Word, FILE *, Handle, LongWord, char, char))
{
  Handle databuf;
  LongWord maxmem, i;
  int SessionPB = 0;

  maxmem = length;
  databuf = NewHandle(length, MyID, attrLocked, NULL);
  if (err=toolerror()) {
    if (err != 0x0201)
      puke("NewHandle problem");
    CompactMem();
    maxmem=(LongWord)(MaxBlock() & 0xFFFF00);
    databuf = NewHandle(maxmem, MyID, attrLocked, NULL);
    if (err=toolerror())
      puke("NewHandle");
  }
  for (i = 0; i < length; i+=maxmem) {
    if ( (i+maxmem) > length)
      maxmem = length - i;
    doit(refNum, out, databuf, maxmem, from, to);
  }
  DisposeHandle(databuf);
  EndSessionGS(&SessionPB);
  close(refNum);
}

void detab(Word refNum, LongWord length)
{
  FILE *outfile;
  RefInfoRecGS RRec;
  FileInfoRecGS IRec;
  CreateRecGS CRec;
  ChangePathRecGS NRec;
  GSString255 name;
  LongWord trash;

  RRec.pCount = 3;
  RRec.refNum = refNum;
  RRec.pathname = (ResultBuf255 *) malloc(sizeof(ResultBuf255));
  GetRefInfoGS(&RRec);
  IRec.pCount = 7;
  IRec.pathname = &RRec.pathname->bufString;
  GetFileInfoGS(&IRec);
  if (err=toolerror())
    puke("GetFileInfoGS");
  RRec.pathname->bufString.text[RRec.pathname->bufString.length+1] = '\0';
  name.length = RRec.pathname->bufString.length;
  strcpy(name.text, RRec.pathname->bufString.text);
  CRec.pathname = &(RRec.pathname->bufString);
  while (CRec.pathname->text[CRec.pathname->length--] != ':');
  strcpy(&CRec.pathname->text[CRec.pathname->length+2], "TEMP00");
  CRec.pathname->length+=7;
  CRec.pathname->text[++CRec.pathname->length] = '\0';
  CRec.pCount = 4;
  CRec.access = IRec.access;
  CRec.fileType = IRec.fileType;
  CRec.auxType = IRec.auxType;
  CreateGS(&CRec);
  outfile = fopen(CRec.pathname->text, "wb");
  convert(refNum, outfile, length, 0x00, 0x00, tabfunc);
  fclose(outfile);
  NRec.pCount = 1;
  NRec.pathname = &name;
  DestroyGS(&NRec);
  NRec.pCount = 2;
  NRec.pathname = CRec.pathname;
  NRec.newPathname = &name;
  ChangePathGS(&NRec);
  free((void *) RRec.pathname);
}  

int main(int argc, char *argv[])
{
  int x, filecount, SessionPB = 0;
  Word refNum;
  LongWord length;
  char expanded[65];
  Init_WildcardPB iwpb;
  Next_WildcardPB nwpb;

  filecount = 2;
  if (argc < 3)
    usage(argv[0]);
  x = 0;
  while (argv[1][x] = tolower(argv[1][x++]));
  if (!strcmp(argv[1],"-crlf"))
    x = 1;
  else if (!strcmp(argv[1],"-lfcr"))
    x = 2;
  else if (!strcmp(argv[1],"-0001"))
    x = 3;
  else if (!strcmp(argv[1],"-lower"))
    x = 4;
  else if (!strcmp(argv[1],"-detab")) {
    x = 5;
    filecount = 3;
    sscanf(argv[2],"%d",&tabColumns);
  }
  else {
    fprintf(stderr,"conv: Illegal conversion parameter %s\n",argv[1]);
    usage(argv[0]);
  }

  while (filecount < argc) {
    strcpy(expanded+1,argv[filecount]);
    expanded[0] = strlen(argv[filecount]);
    iwpb.w_file = expanded;
    iwpb.flags = 0x8000;
    INIT_WILDCARD(&iwpb);
    nwpb.nextfile = expanded;
    NEXT_WILDCARD(&nwpb);
    expanded[expanded[0]+1] = 0;
    if (strlen(expanded) == 0) {
        fprintf(stderr,"conv: '%s' does not exist.\n",argv[filecount]);
        exit(1);
    }
    while (strlen(expanded) != 0) {
      printf("Processing: %s\n", expanded+1);
      if (x == 4) {
        char *p, *r;
        p = (char *) malloc(sizeof(char) * strlen(expanded+1));
        strcpy(p, expanded+1);
        r = p;
        while (*r != '\0') { if (isupper(*r)) *r = tolower(*r); r++; }
        rename(expanded+1, p);
        free((void *) p);
      }
      else {
        MyID = MMStartUp();
        BeginSessionGS(&SessionPB);
        length = openfile(expanded+1, &refNum);
        if (err=toolerror() == 0) {
          switch (x) {
          case 1:
            convert(refNum, NULL, length, 0x0D, 0x0A, swapfunc);
            break;
          case 2:
            convert(refNum, NULL, length, 0x0A, 0x0D, swapfunc);
            break;
          case 3:
            convert(refNum, NULL, length, 0x00, 0x01, swapfunc);
            break;
          case 5:
            detab(refNum, length);
            break;
          }
        }
      }
      NEXT_WILDCARD(&nwpb);
      expanded[expanded[0]+1] = 0;
    }
    filecount++;
  }
  MMShutDown(MyID);
  return 0;
}

