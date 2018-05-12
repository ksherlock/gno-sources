/*

    ls.c

    a BSD-like 'ls' for the IIgs


1.3 11/30/91
    Rewrote the directory printer to display in order, but down
    columns instead of across rows.  It's MUCH more readable now.
    There is a side-effect in that the # of columns used is not
    always as much as it could be (the printing algorithm shrinks
    the array to as small a box as possible) (jb)
1.2 11/14/91
    Added error checking at major Open calls
1.1
    Removed excessive newlines (jb)
    Fixed bug preventing use with C 1.2
1.0 7/7/91
    original by Derek Taubert

*/

#include <types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <GSOS.h>
#include <shell.h>

#pragma stacksize 2048
#pragma optimize -1

#ifdef NULL
#undef NULL
#define NULL (void*)0l
#endif

/* #define BUG(__s) printf("%s\n",__s);
*/ #define BUG(__s)

struct mDirEntryRecGS {
   Word pCount;
   Word refNum;
   Word flags;
   Word base;
   Word displacement;
   ResultBuf255Ptr name;
   Word entryNum;
   Word fileType;
   Longint eof;
   LongWord blockCount;
   TimeRec createDateTime;
   TimeRec modDateTime;
   Word access;
   LongWord auxType;
/*   Word fileSysID;
   ResultBuf255Ptr optionList;
   LongWord resourceEOF;
   LongWord resourceBlocks;  */
} ;
typedef struct mDirEntryRecGS mDirEntryRecGS, *mDirEntryRecPtrGS;

#define CALCULATE -1
#define ONLYONE -2

struct listStruct {
    int entries;
    long totalSize;
    long dirSize;
    ResultBuf255Ptr parentDir;
    mDirEntryRecGS data[1];
} ;

typedef struct listStruct *list;
const char month[12][4] =
  { "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec" };

typedef struct FileTypeConv {
   word type;
   char rep[10];
} FileTypeConv;

FileTypeConv FTTable[] = {
        0xB0, "src ", /* apw source file */
        0xb1, "obj ", /* apw object file */
        0x04, "txt ", /* ascii text file */
        0xb3, "s16 ", /* gs/os program file */
        0xb5, "exe ", /* gs/os shell program file */
        0xb8, "nda ",
        0xb9, "cda ",
        0xba, "tol ",
        0x00, "non ", /* typeless file */
        0x01, "bad ", /* bad block file */
        0x06, "bin ", /* general binary file */
        0x08, "fot ", /* graphics screen file */
        0x0f, "\x1B\xFXY\xE\x18  ", /* directory file */
        0x19, "adb ", /* appleworks data base file */
        0x1a, "awp ", /* appleworks word processor file */
        0x1b, "asp ", /* appleworks spreadsheet */
        0xb2, "lib ", /* apw library file */
        0xb4, "rtl ", /* apw runtime library */
        0xef, "pas ", /* pascal partition */
        0xf0, "cmd ",
        0xfa, "int ", /* integer basic program */
        0xfb, "var ", /* int basic var file */
        0xfc, "bas ", /* applesloth basic file */
        0xfd, "var ", /* applesloth variable file */
        0xfe, "rel ", /* EDASM relocatable code */
        0xff, "sys ", /* prodos 8 system program file */
        -1, ""};


char conv[10];
char *getFileType(int type)
{
int i;

    i = 0;
    while (FTTable[i].type != -1)
    {
        if (FTTable[i].type == type)
        {
            return FTTable[i].rep;
        }
        else i++;
    }
    sprintf(conv,"$%2X ",type);
    return conv;
}

void *sortRoutine;
int less, more, columns, width;
int openDirectory, noHidden, longOutput, inK;
int dirOnly, idType, recursive;

void rexit(int status)
{
QuitRecGS q,*Q;
int err;
   q.pCount = 2;
   q.pathname = NULL;
   q.flags = restartable;
   Q = &q;
   asm {
       lda Q+2
       pha
       lda Q
       pha
       pea 0x2029
       lda status
       jsl 0xE100B0
       sta err
   }
   printf("QuitGS failed: %04X\n",err);
   exit(-999); /* should an error occur in Quit */
}
int CompareCreate(mDirEntryRecGS *entry1,mDirEntryRecGS *entry2)
{
int temp;
TimeRec *time1, *time2;
    if ((temp = entry1->name->bufString.length) > width) width = temp;
    if ((temp = entry2->name->bufString.length) > width) width = temp;
    time1 = &(entry1->createDateTime);
    time2 = &(entry2->createDateTime);
    if (time1->year < time2->year) return less;
    if (time1->year > time2->year) return more;
    if (time1->month < time2->month) return less;
    if (time1->month > time2->month) return more;
    if (time1->day < time2->day) return less;
    if (time1->day > time2->day) return more;
    if (time1->hour < time2->hour) return less;
    if (time1->hour > time2->hour) return more;
    if (time1->minute < time2->minute) return less;
    if (time1->minute > time2->minute) return more;
    if (time1->second < time2->second) return less;
    if (time1->second > time2->second) return more;
    return 0;
}

int CompareLastMod(mDirEntryRecGS *entry1,mDirEntryRecGS *entry2)
{
int temp;
TimeRec *time1, *time2;
    if ((temp = entry1->name->bufString.length) > width) width = temp;
    if ((temp = entry2->name->bufString.length) > width) width = temp;
    time1 = &(entry1->modDateTime);
    time2 = &(entry2->modDateTime);
    if (time1->year < time2->year) return less;
    if (time1->year > time2->year) return more;
    if (time1->month < time2->month) return less;
    if (time1->month > time2->month) return more;
    if (time1->day < time2->day) return less;
    if (time1->day > time2->day) return more;
    if (time1->hour < time2->hour) return less;
    if (time1->hour > time2->hour) return more;
    if (time1->minute < time2->minute) return less;
    if (time1->minute > time2->minute) return more;
    if (time1->second < time2->second) return less;
    if (time1->second > time2->second) return more;
    return 0;
}

int CompareNames(mDirEntryRecGS *entry1,mDirEntryRecGS *entry2)
{
int temp;
    if ((temp = entry1->name->bufString.length) > width) width = temp;
    if ((temp = entry2->name->bufString.length) > width) width = temp;
    if (dirOnly) return 0;
    temp = more * strcmp(entry1->name->bufString.text,
        entry2->name->bufString.text);
    return temp;
}

list addToList(list whatList, char *entryName, int mult)
{
mDirEntryRecGS *entry;
static GSString255 NameGS;
static OpenRecGS open;
static ExpandPathRecGS exp;
static mDirEntryRecGS rwind;
int e,i,err;

BUG("AddToList");
    if (whatList == NULL) {
BUG("New list");
        whatList =
          (list)malloc(sizeof(struct listStruct) - sizeof(mDirEntryRecGS));
        whatList->entries = 0;
        whatList->dirSize = 0l;
        whatList->parentDir = NULL;
        whatList->totalSize =
          (long)(sizeof(struct listStruct) - sizeof(mDirEntryRecGS));
    }

    strcpy(NameGS.text,entryName);  NameGS.length = (word)strlen(entryName);
    open.pCount = 12;
    open.pathname = &NameGS;
    open.requestAccess = 0x0001;
    OpenGS(&open);
    if (err = toolerror()) { ERROR(&err); exit(1); }
BUG("file open");
    if (mult) {
        if (dirOnly && (open.fileType != 0x0f)) goto oops;
BUG("faking the entry");
        whatList->totalSize+=sizeof(mDirEntryRecGS);
        whatList = (list)realloc(whatList,whatList->totalSize);
        entry = (mDirEntryRecGS *)&(whatList->data[whatList->entries++]);

        entry->name = (ResultBuf255Ptr)malloc(sizeof(ResultBuf255));
        entry->name->bufSize = 255;
        memcpy(&(entry->name->bufString),&NameGS,sizeof(GSString255));
        entry->access = open.access;
        entry->fileType = open.fileType;
        entry->eof = open.eof;
        entry->flags = (open.storageType == 5) ? 0x8000 : 0x0000;
        entry->auxType = open.auxType;
        memcpy(&(entry->createDateTime),&(open.createDateTime),sizeof(TimeRec));
        memcpy(&(entry->modDateTime),&(open.modDateTime),sizeof(TimeRec));
    } else {
BUG("expanding directories");
        exp.pCount = 2;
        exp.inputPath = &NameGS;
        exp.outputPath = whatList->parentDir = (ResultBuf255Ptr)malloc(
              sizeof(ResultBuf255));
        exp.outputPath->bufSize = 255;
        ExpandPath(&exp);
        whatList->parentDir->bufString.text
          [whatList->parentDir->bufString.length]=0;
BUG("rewinding dir and getting number of entries");
        rwind.pCount = 6;
        rwind.refNum = open.refNum;
        rwind.name = (ResultBuf255Ptr)malloc(sizeof(ResultBuf255));
        rwind.name->bufSize = 255;
        rwind.base = rwind.displacement = 0;
        GetDirEntryGS(&rwind);
        whatList->totalSize+=(rwind.entryNum * sizeof(mDirEntryRecGS));
        whatList = (list)realloc(whatList,whatList->totalSize);
        free(rwind.name);

        for (i=0;i<rwind.entryNum;i++) {
BUG("entry");
            entry = (mDirEntryRecGS *)&(whatList->data[whatList->entries++]);
            entry->pCount = 14;
            entry->refNum = open.refNum;
            entry->name = (ResultBuf255Ptr)malloc(sizeof(ResultBuf255));
            entry->name->bufSize = 255;
            entry->base = entry->displacement = 1;
            GetDirEntryGS(entry);
            if (e=toolerror()) {
                free(entry->name);
                whatList->entries--;
                break;
            }
            whatList->dirSize += (long)entry->eof;
            if ((entry->access & 0x4) && noHidden) {
                free(entry->name);
                whatList->entries--;
                continue;
            }
            entry->name->bufString.text[entry->name->bufString.length]=0;
        }
    }
oops: open.pCount = 1;
    CloseGS(&open);
BUG("file closed");
    return whatList;
}

void freeList(list whatList)
{
int i;
BUG("free list");
    for (i=0;i<whatList->entries;i++) {
        free(whatList->data[i].name);
    }
    if (whatList->parentDir) free(whatList->parentDir);
    free(whatList);
}

void ls(list whatList, int mult)
{
int i, j, rWidth, afile = FALSE, numColumns = 1;
mDirEntryRecGS *entry;
list newList;
static char path[128];
TimeRec *time;
int Col,Row,Height;

BUG("ls");
    width = 0;
    if (!whatList) rexit(1);
    qsort(&(whatList->data[0]),(size_t)whatList->entries,
      sizeof(mDirEntryRecGS),sortRoutine);
    if (!longOutput && columns == CALCULATE) {
        rWidth = width;
        if (inK) width+=5;
        if (idType) width++;
        if (width < 15) {
          rWidth = 15 - (width - rWidth);
          width = 15;
        } /* for jawaid :-) */
        numColumns = 80 / (++width);
    }
BUG("qsort done");
    if (whatList->parentDir)
        printf("%s\n",whatList->parentDir->bufString.text);
 /*   printf("\n");  */
    if (whatList->dirSize && longOutput)
        printf("total %ldk\n",(whatList->dirSize/1024)+1);
  Height = (whatList->entries / numColumns);
  if (whatList->entries % numColumns)  Height++;
  for (Row=0; Row < Height; Row ++) {
    for (Col = 0; Col < numColumns; Col ++) {
        i = Col * Height + Row;
        if (i >= whatList->entries) continue;

/*    for (i=0;i<whatList->entries;i++) { */
        entry = &(whatList->data[i]);
        if (mult && entry->fileType == 0x0f) continue;
        afile = TRUE;
/*        if (++currColumn > numColumns) {
            printf("\n");
            currColumn = 1;
        }      */
        if (inK) printf("%4ld ",(entry->eof/1024)+1);
        time = (sortRoutine == CompareCreate) ? &(entry->createDateTime)
          : &(entry->modDateTime);
        if (longOutput) {
            printf("%c%c%c%c%c%c%c %04lX %4s %8ld %3s %2d %02d:%02d %4d ",
              (entry->fileType == 0x0f) ? 'd' :
                ((entry->flags & 0x8000) ? 'e' : '-'),
              (entry->access & 0x01) ? 'r' : '-',
              (entry->access & 0x02) ? 'w' : '-',
              ((entry->fileType == 0xff) || (entry->fileType == 0xb5) ||
                (entry->fileType == 0xb3) ||
                ((entry->fileType == 0xb0) && (entry->auxType == 6l)))
                ? 'x' : '-',
              (entry->access & 0x20) ? 'b' : '-',
              (entry->access & 0x40) ? 'r' : '-',
              (entry->access & 0x80) ? 'd' : '-',
              entry->auxType,
              getFileType(entry->fileType),
              entry->eof,
              month[time->month],time->day+1,time->hour,time->minute,
                time->year+1900);
        }
        printf("%s",entry->name->bufString.text);
        if (idType) {
            if (entry->fileType == 0x0f) printf("/");
            else if ((entry->fileType == 0xff) || (entry->fileType == 0xb5) ||
              (entry->fileType == 0xb3) ||
              ((entry->fileType == 0xb0) && (entry->auxType == 6l))
              ) printf("*");
            else printf(" ");
        }
        if (Col+1 < numColumns) {
BUG("trailing spaces");
            for (j=0;j<(rWidth-(int)(entry->name->bufString.length));j++)
                printf(" ");
            printf(" ");
        }
    }
  putchar('\n');
  }
    if (afile) printf("\n");
BUG("checking for recursive stuff");
    if (recursive || mult) {
        printf("\n");  /* put blank lines in a recursive listing */
        for (i=0;i<whatList->entries;i++) {
            if (whatList->data[i].fileType == 0x0f) {
BUG("getting subdirectory");
                if (whatList->parentDir) {
                    sprintf(path,"%s:%s",whatList->parentDir->bufString.text,
                      whatList->data[i].name->bufString.text);
                } else {
                    strcpy(path,whatList->data[i].name->bufString.text);
                }
                newList = addToList(NULL,path,0);
BUG("printing subdirectory");
                ls(newList,0);
                freeList(newList);
            }
        }
    }
}

main(argc, argv)
        int argc;
        char **argv;
{
extern int optind, errno;
extern char *optarg;
int ch;
list output;
static char current[128];

optarg = NULL; optind = 0;
sortRoutine = CompareNames;
less = -1; more = 1; columns = CALCULATE;
openDirectory = TRUE; noHidden = TRUE; longOutput = FALSE; inK = FALSE;
dirOnly = FALSE; idType = FALSE; recursive = FALSE;

/* no g (group) i (inode) A (current & parent) L (links) u (last access) */

    while ((ch = getopt(argc, argv, "acdflqrst1CFR")) != EOF)
        switch(ch) {
            case 'r' :
                less = 1;         more = -1;
                break;
            case 't' :
                sortRoutine = CompareLastMod;
                break;
            case 'c' :
                sortRoutine = CompareCreate;
                break;
            case '1' :
                columns = ONLYONE;
                break;
            case 'C' :
                columns = CALCULATE;
                break;
            case 'a' :
                noHidden = FALSE;
                break;
            case 'l' :
                longOutput = TRUE;
                break;
            case 's' :
                inK = TRUE;
                break;
            case 'f' :
                dirOnly = TRUE;
                longOutput = FALSE;
                inK = FALSE;
                noHidden = FALSE;
                break;
            case 'q' :
                break;
            case 'F' :
                idType = TRUE;
                break;
            case 'R' :
                recursive = TRUE;
                break;
            case 'd' :
                openDirectory = FALSE;
                recursive = FALSE;
                break;
            default:
                (void)fprintf(stderr,
                  "usage: ls [-acdfilqrstu1ACLFR] [name ...]\n");
                rexit(1);
        }
    argv += optind;
BUG("done with the options...");
    if (!*argv) {
BUG("no arguments");
        getwd(current);
        ls(output=addToList(NULL,current,!openDirectory),0);
        freeList(output);
    } else {
BUG("files as args");
        output = NULL;
        for (;;) {
            output = addToList(output,*argv,1);
            if (!*++argv) break;
        }
        ls(output,(openDirectory || dirOnly));
        freeList(output);
    }
    rexit(0);
}

