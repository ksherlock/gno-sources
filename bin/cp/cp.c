/*
	cp.c

	original version by Greg R. Thompson

	v1.2 (jb) made getting y/n responses friendlier (ReadChar &
                  actual y/n checks)
        v1.1 (jb) added file nonexistence checking to all commands

        Copyright 1992, Greg Thompson and Procyon, Inc.
*/

#pragma stacksize 1024
#pragma optimize 8

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <gsos.h>
#include <memory.h>
#include <event.h>
#include <locator.h>
#include <texttool.h>
#include <shell.h>
#include <gno/gno.h>
#include <gno/signal.h>

#define GSString(gs255ptr, strptr) gs255ptr = (GSString255Ptr)\
 malloc(sizeof(GSString255)); strcpy(gs255ptr->text, strptr);\
 gs255ptr->length = strlen(gs255ptr->text);

#define SOURCE_DATA 0
#define SOURCE_RESOURCE 1
#define DEST_DATA 2
#define DEST_RESOURCE 3

#define DIR 1
#define NON_DIR 0
#define NON_EXISTANT -1

typedef struct {
  Word pCount;
} SessionRecGS;

typedef struct dir_list_struct {
  char *dir_name;
  struct dir_list_struct *next_dir;
  struct dir_list_struct *prev_dir;
} dir_list;

#define FILE_NULL (file_list *) 0L
#define DIR_NULL (dir_list *) 0L
#define CHAR_NULL (char *) 0L

#define CP 1
#define RM 2
#define MV 3

char *progNames[] = {"","cp","rm","mv"};

int err, recurse = 0;
int interactive = 0;
int verbose = 0;
int force = 0;
int program = 0;
int gno;
Word MyID;
Word refNums[4] = {0, 0, 0, 0};  /* s_d, s_r, d_d, d_r */
RefNumRecGS dirRef = {1, 0};
char *sourcefile, *destfile, *destdir, *progname;
FileInfoRecGS FInfo;
SessionRecGS  SRec = {0};
LongWord      maxmem;
Handle        buffer;
EventRecord   *ERec;
Handle        EMHandle;

void close(void);

/* Prints a usage message to stderr & exits. */
void usage(void)
{
  if (!strcmp(progname, "cp")) {
    printf("usage:\tcp [-vi] file dest_file\n");
    printf("\tcp [-vi] file1 [ file2 ... ] dest_dir\n");
    printf("\tcp [-vir] dir1 [ dir2 ... ] dest_dir\n");
  } else if (!strcmp(progname, "rm")) {
    printf("usage\trm [-vif] file1 [ file2 ... ]\n");
    printf("\trm [-virf] dir1 [ dir2 ... ]\n");
  } else if (!strcmp(progname, "mv")) {
    printf("usage\tmv [-vi] file dest_file\n");
    printf("\tmv [-vi] file1 [ file2 dir1 ... ] dest_dir\n");
  } else {
    printf("usage\t%s -p <prog>\n");
    printf("\twhere prog is `cp', `rm', or `mv'\n");
  }
  if (!gno) {
    EMShutDown();
    DisposeHandle(EMHandle);
    free((void *) ERec);
  }
  exit(-1);
}

int getyn(void)
{
int c,r;

again:
    do {
	c = tolower(ReadChar(0) & 0x7F);
    } while ((c != 'y') && (c != 'n'));
    WriteChar(c);
    do {
	r = ReadChar(0) & 0x7F;
    } while ((r != 13) && (r != 8) && (r != 0x7F));
    if ((r == 8) || (r == 0x7F)) {
	WriteChar(8);
        goto again;
    }
    putchar('\n');
    return c;
}

/* Error handler.  Prints the error number, a message, and quits nicely */
void puke(char *msg, char *file)
{
  if (err)
    ERROR(&err);
  if (msg)
    printf("Error occurred while %s", msg);
  if (file)
    printf(" on file %s\n", file);
  else
    printf(".\n");
  close();
  if (dirRef.refNum)
    CloseGS(&dirRef);
  EndSession(&SRec);
  DisposeHandle(buffer);
  if (!gno) {
    EMShutDown();
    DisposeHandle(EMHandle);
  }
  MMShutDown();
  if (!msg && !file)
    printf("Operation aborted.\n");
  exit(-1);
}

/* Signal handler to trap ^C codes.  Allows us to clean up as we die */
void handler(int sig, int code)
{
  puke(NULL, NULL);
}

/*  Calls ExpandPathGS to expand the 'source' path.  Returns a pointer which
 * will be made invalid the next time expand is called.
 */
char *expand(char *source)
{
  ExpandPathRecGS   ERec = {3, 0l, 0l, 0};
  char             *dest;

  GSString(ERec.inputPath, source);
  ERec.outputPath = (ResultBuf255Ptr) malloc(sizeof(ResultBuf255));
  ERec.outputPath->bufSize = 0x103;
  ExpandPathGS(&ERec);
  if (err = toolerror()) {
    free((void *) ERec.inputPath);
    free((void *) ERec.outputPath);
    puke("expanding path", source);
  }
  ERec.outputPath->bufString.text[ERec.outputPath->bufString.length] = '\0';
  dest = (char *) malloc(ERec.outputPath->bufString.length + 1);
  strcpy(dest, ERec.outputPath->bufString.text);
  free((void *) ERec.inputPath);
  free((void *) ERec.outputPath);
  return dest;
}

/*  Checks to see if filename is a dir.  Returns 1 if dir, 0 if normal
 * file, -1 if nonexistant.
 */
int isadir(char *filename)
{
  FileInfoRecGS     FRec = {3, 0l, 0, 0};

  GSString(FRec.pathname, filename);
  GetFileInfoGS(&FRec);
  err = toolerror();
  free((void *) FRec.pathname);
  if (err == fileNotFound)
    return -1;
  else if (err)
    puke("checking info", filename);
  return(FRec.fileType == 0x0F);
}

/*  Returns a pointer to the tail portion of the path DIR.  NOTE: It doesn't
 * allocate any memory.  It simply returns a pointer somewhere in the middle
 * of the existing string.
 */
char *pathtail(char *dir)
{
  if (*dir == '\0')        /* If this is an empty string, return NULL */
    return NULL;
  while (*++dir != '\0');  /* point dir to the end of the path */
  while (*--dir != ':');   /* back up to the last ':' */
  return (dir);            /* point to the ':' before the name */
}

/* Adds the name of the file in sourcefile to the path in destfile. */
void fixname(void)
{
  char *tail_ptr, *temp_dest;

  tail_ptr = pathtail(sourcefile);
  temp_dest = (char *) malloc(strlen(destfile) + strlen(tail_ptr) + 1);
  strcat(strcpy(temp_dest, destfile), tail_ptr);
  free((void *) destfile);
  destfile = temp_dest;
}

/* Creates a new directory (for recursive copy) */
void mkdir(char *name, Word num)
{
  CreateRecGS CRec = {6, 0l, 0xC3, 0x0F, 0l, directoryFile, 0l};

  GSString(CRec.pathname, name);
  CRec.eof = (LongWord) num;

  CreateGS(&CRec);
  /* don't error if the directory exists, assume they want to use
     that existing directory and merge the files */
  if ((err = toolerror()) && (err != 0x47)) {
    free((void *) CRec.pathname);
    puke("creating dir", name);
  }
  free((void *) CRec.pathname);
}

void remove_it(char *file)
{
  NameRecGS          NRec = {1, 0l};
  int                response;
  char              *full;

  if (!gno) {
    if ((GetNextEvent(keyDownMask,ERec)) &&
        ((ERec->modifiers & appleKey) && (ERec->message == '.')))
      puke(NULL, NULL);
  }

  if (interactive) {
    full = expand(file);
    printf("rm: remove %s (y/n) ? ", full);
    free((void *) full);
    response = getyn();
    /* while ((response = getchar()) == '\n'); */
    if (response == 'n')
      return;
  } else if (verbose && (program == RM)) {
    full = expand(file);
    printf("rm: removing %s\n", pathtail(full)+1);
    free((void *) full);
  }
  GSString(NRec.pathname, file);
  DestroyGS(&NRec);
  if (err = toolerror()) {
    free((void *) NRec.pathname);
    puke("removing", file);
  }
  free((void *) NRec.pathname);
}

/*  Gets info on the sourcefile, creates destfile, and opens both forks
 * of both files.  returns 1 if successful, 0 if not.
 */
int open(void)
{
  OpenRecGS   ORec = {14};
  CreateRecGS CRec = {7, 0l, 0xC3, 0, 0l, 0, 0l, 0l};
  NameRecGS   DRec = {1, 0l};
  int response;

  if (verbose && (program == CP))
    printf("cp: copying %s\n", pathtail(sourcefile)+1);

  /* Get info on source file */
  GSString(FInfo.pathname, sourcefile);
  FInfo.pCount = 12;
  FInfo.optionList = 0;
  GetFileInfoGS(&FInfo);
  if (err = toolerror()) {
    free((void *) FInfo.pathname);
    puke("getting info", sourcefile);
  }

  /* Create destination file */
  GSString(CRec.pathname, destfile);
  CRec.fileType = FInfo.fileType;
  CRec.auxType = FInfo.auxType;
  CRec.storageType = FInfo.storageType;
  CRec.eof = 0;
  CRec.resourceEOF = 0;
  CreateGS(&CRec);
  err = toolerror();
  if ((err) && (err != dupPathname)) {
    free((void *) FInfo.pathname);
    free((void *) CRec.pathname);
    puke("creating destination", destfile);
  }
  if (err) {     /* delete old file */
    if (interactive) {
      printf("overwrite %s (y/n)? ", destfile);
      response = getyn();
      /* while ((response = getchar()) == '\n'); */
      if (response == 'n')
        return 0;
    }
    DRec.pathname = CRec.pathname;
    DestroyGS(&DRec);
    if (err = toolerror()) {
      free((void *) FInfo.pathname);
      free((void *) CRec.pathname);
      puke("destroying", destfile);
    }
    CreateGS(&CRec);
    if (err = toolerror()) {
      free((void *) FInfo.pathname);
      free((void *) CRec.pathname);
      puke("creating destination", destfile);
    }
  }

  /* Open both forks of source file */
  ORec.refNum = 0;
  ORec.pathname = FInfo.pathname;
  ORec.requestAccess = readEnable;
  ORec.resourceNumber = 0;
  ORec.optionList = 0l;
  OpenGS(&ORec);
  if (err = toolerror())
    puke("opening data fork", sourcefile);
  refNums[SOURCE_DATA] = ORec.refNum;

  if (FInfo.storageType == extendedFile) {
    ORec.refNum = 0;
    ORec.resourceNumber = 1;
    OpenGS(&ORec);
    if (err = toolerror())
      puke("opening resource fork", sourcefile);
    refNums[SOURCE_RESOURCE] = ORec.refNum;
  }
  else
    refNums[SOURCE_RESOURCE] = 0;

  /* Open both forks of destination if necessary. */
  ORec.refNum = 0;
  ORec.pathname = CRec.pathname;
  ORec.requestAccess = writeEnable;
  if (refNums[SOURCE_DATA]) {
    ORec.resourceNumber = 0;
    OpenGS(&ORec);
    if (err = toolerror())
      puke("opening data fork", destfile);
    refNums[DEST_DATA] = ORec.refNum;
  }

  if (refNums[SOURCE_RESOURCE]) {
    ORec.refNum = 0;
    ORec.resourceNumber = 1;
    OpenGS(&ORec);
    if (err = toolerror())
      puke("opening resource fork", destfile);
    refNums[DEST_RESOURCE] = ORec.refNum;
  }
  free((void *) CRec.pathname);
  return 1;
}

/* Closes both forks of both open files. */
void close(void)
{
  RefNumRecGS     CRec = {1, 0};
  int i;

  for (i = 0; i <= 3; ++i)
    if ((CRec.refNum = refNums[i]) != 0) {
      CloseGS(&CRec);
      refNums[i] = 0;
    }
  if (!gno) {
    if ((GetNextEvent(keyDownMask,ERec)) &&
        ((ERec->modifiers & appleKey) && (ERec->message == '.')))
      puke(NULL, NULL);
  }
}

/*  Moves sourcefile to destfile.  Returns 1 if worked, 0 if not. */
/* Prompts & removes old file. */
int move(char *from, char *to)
{
  ChangePathRecGS       CRec = {2, 0l, 0l};
  NameRecGS             DRec = {1, 0l};
  int                   response;
  
  if (verbose)
    printf("mv: moving %s\n", pathtail(from)+1);
  GSString(CRec.pathname, from);
  GSString(CRec.newPathname, to);
a: ChangePathGS(&CRec);
  if (err = toolerror()) {
    if (err == badPathSyntax) {
      free((void *) CRec.pathname);
      free((void *) CRec.newPathname);
      return 0;
    }
    else if (err == dupPathname) {
      if (interactive) {
        printf("overwrite %s (y/n)? ", to);
        response = getyn();
        /* while ((response = getchar()) == '\n'); */
        if (response == 'n') {
          free((void *) CRec.pathname);
          free((void *) CRec.newPathname);
          return -1;
        }
      }
      DRec.pathname = CRec.newPathname;
      DestroyGS(&DRec);
      if (err = toolerror()) {
        free((void *) CRec.pathname);
        free((void *) CRec.newPathname);
        puke("killing for a move", destfile);
      }
      goto a;
    }
    puke("changing path", NULL);
  }
  free((void *) CRec.pathname);
  free((void *) CRec.newPathname);
  return 1;
}
 
/*  Data is read from the source file, and written to the destination file.
 * When all the data has been copied, the destination file is flushed.
 */
void docopy(Word fromRef, Word toRef, LongWord length)
{
  IORecGS       RRec = {5, 0, 0l, 0l, 0l, 0};
  RefNumRecGS   FRec = {1, 0};
  LongWord      i, blocksize;

  RRec.dataBuffer = *buffer;
  blocksize = maxmem;
  for (i = 0; i < length; i+=blocksize) {
    if ( (i+blocksize) > length)
      blocksize = length - i;
    RRec.refNum = fromRef;
    RRec.requestCount = blocksize;
    ReadGS(&RRec);
    if (err=toolerror())
      puke("reading", sourcefile);
    RRec.refNum = toRef;
    RRec.requestCount = RRec.transferCount;
    WriteGS(&RRec);
    if (err=toolerror())
      puke("writing", destfile);
  }
  FRec.refNum = toRef;
  FlushGS(&FRec);
  if (err=toolerror())
    puke("flushing", destfile);
}

/*  Calls docopy() once for each fork that needs to be copied.  Once the copy
 * is done, the file info is set for the destination file.
 */
void copy(void)
{
  docopy(refNums[SOURCE_DATA], refNums[DEST_DATA], FInfo.eof);
  if (FInfo.storageType == extendedFile)
   docopy(refNums[SOURCE_RESOURCE], refNums[DEST_RESOURCE], FInfo.resourceEOF);
  strcpy(FInfo.pathname->text, destfile);
  FInfo.pathname->length = strlen(FInfo.pathname->text);
  SetFileInfoGS(&FInfo);
  if (err=toolerror())
    puke("setting info", destfile);
  free((void *) FInfo.pathname);
}

/*  This is one of the two options for the "free_list" function for the
 * directory traversal routine.  This will simply dispose of all memory
 * used by the traversal.
 */
void free_only(dir_list *list, dir_list *list_tail, char *dir)
{
  while (list != DIR_NULL) {
    free((void *) list->dir_name);
    list_tail = list;               /* We don't need the tail ptr, so use */
    list = list->next_dir;          /* it rather than a new variable.     */
    free((void *) list_tail);
  }
}

/*  This is the other option for the "free_list" function.  This one will
 * delete the directories as it disposes of the memory used by the
 * traversal machines.
 */
void free_n_remove(dir_list *list, dir_list *list_tail, char *dir)
{
  dir_list      *temp_dir;

  while (list_tail != DIR_NULL) {
    remove_it(list_tail->dir_name);  /* should be 22:something */
    free((void *) list_tail->dir_name);
    list = list_tail;                 /* We don't need the list ptr.... */
    list_tail = list_tail->prev_dir;
    free((void *) list);
  }
}

/*  The following three functions are the options for the "file_op" parameter
 * for the directory traversal routines.  The first delets the file, the
 * second copies the files, and then deletes it, the third just copies the
 * the file.
 */
void remove_file(char *file)
{
  strcat(strcpy(sourcefile, "8:"), file);
  remove_it(sourcefile);
}

void move_file(char *file)
{
  strcat(strcpy(sourcefile, "8:"), file);
  strcat(strcpy(destfile, "9:"), file);
  if (open()) {
    copy();
    close();
  }
  remove_it(sourcefile);
}

void copy_file(char *file)
{
  strcat(strcpy(sourcefile, "8:"), file);
  strcat(strcpy(destfile, "9:"), file);
  if (open()) {
    copy();
    close();
  }
}

/*  This is the amazing directory traversal routine.  Pass in four paramaters.
 * The first parameter is a pointer to a function that will be called after
 * the directories will be traversed.  The options are "free_only" and
 * "free_n_remove".  The second parameter is a pointer to a function that
 * will get called once for each file in the directory.  It will get passed
 * a string corresponding to the name (NOTE:  not a full pathname, just the
 * name of the file) of the file to be copied/moved/removed/whatever.  The
 * options are "copy_file", "remove_file", or "move_file".  The last two
 * parameters that get passed in are the name of the source dir, and the
 * name of the destination dir.  If the destination dir is NULL, then no
 * directories are created.
 *  This function implements a breadth-first traversal of the directories.
 * It is a non-recursive implementation which is very nice to stack space.
 * If you wanna try to make it better, go for it.
 *  The following prefixes are used:
 *         Prefix 20: The full expanded pathname of the base source dir.
 *         Prefix 21: The full expanded pathname of the base dest dir.
 *         Prefix  8: The partial path of the current source dir.
 *         Prefix  9: The partial path of the current dest dir.
 *  By partial path, I mean 22 == 20:something and 23 == 21:something.
 *  To get the path of the source file, and the dest file, just tack
 * 22: and 23: on to the front of the filename.
 */
void traverse_dir(void (*free_lists)(dir_list *, dir_list *, char *),
             void (*file_op)(char *),
             char *sourcedir, char *dstdir)
{
  int i, numEntries;
  OpenRecGS        ORec = {3, 0, 0l, readEnable};
  DirEntryRecGS    DRec = {7, 0, 0, 0, 0, 0l, 0, 0};
  PrefixRecGS      PRec = {2, 0, 0l};
  dir_list *list;      /* head of list of ALL files & dirs to be copied */
  dir_list *cur_dir;   /* pointer to current dir in master list */
  dir_list *list_tail; /* tail to master list */

  PRec.buffer.setPrefix = (GSString255Ptr) malloc(sizeof(GSString255));
  strcpy(PRec.buffer.setPrefix->text, sourcedir);
  PRec.buffer.setPrefix->length = strlen(sourcedir);
  PRec.prefixNum = 20;
  SetPrefixGS(&PRec);
  if (err = toolerror())
    puke("setting prefix #20", sourcedir);
  if (dstdir) {
    strcpy(PRec.buffer.setPrefix->text, dstdir);
    PRec.buffer.setPrefix->length = strlen(dstdir);
    PRec.prefixNum = 21;
    SetPrefixGS(&PRec);
    if (err = toolerror())
      puke("setting prefix #21", dstdir);
  }
  list = (dir_list *) malloc(sizeof(dir_list));
  list->next_dir = DIR_NULL;
  list->prev_dir = DIR_NULL;
  list->dir_name = (char *) malloc(4);
  strcpy(list->dir_name, "20:");
  cur_dir = list;
  list_tail = list;
  ORec.pathname = (GSString255Ptr) malloc(sizeof(GSString255));
  DRec.name = (ResultBuf255Ptr) malloc(sizeof(ResultBuf255));
  DRec.name->bufSize = 0x103;

  while (cur_dir != DIR_NULL) {
    strcpy(PRec.buffer.setPrefix->text, cur_dir->dir_name);
    PRec.buffer.setPrefix->length = strlen(cur_dir->dir_name);
    PRec.prefixNum = 8;
    SetPrefixGS(&PRec);
    if (err = toolerror())
      puke("setting prefix #8", cur_dir->dir_name);
    if (dstdir) {
      PRec.prefixNum = 9;
      PRec.buffer.setPrefix->text[1] = '1';
      SetPrefixGS(&PRec);
      if (err = toolerror())
        puke("setting prefix #9", cur_dir->dir_name);
    }
    strcpy(ORec.pathname->text, "8:");
    ORec.pathname->length = 2;
    OpenGS(&ORec);
    if (err = toolerror()) {
      puke("opening dir for recurse", sourcefile);
    }
    dirRef.refNum = ORec.refNum;
    DRec.refNum = ORec.refNum;
    DRec.base = DRec.displacement = 0;
    GetDirEntryGS(&DRec);
    if (err = toolerror()) {
      free((void *) DRec.name);
      puke("getting number of entries", sourcefile);
    }
    numEntries = DRec.entryNum;
    if (dstdir)
      mkdir("9:", numEntries);
    DRec.base = DRec.displacement = 1;
    for (i = 0; i < numEntries; ++i) {
      int len;

      GetDirEntry(&DRec);
      if (err=toolerror()) {
        free((void *) DRec.name);
        puke("getting directory entries", sourcefile);
      }
      len = DRec.name->bufString.length;
      DRec.name->bufString.text[len] = '\0';
      if (DRec.fileType != 0x0F) { /* if not a sub-dir */
        file_op(DRec.name->bufString.text);
      } else {
        DRec.name->bufString.text[len++] = ':';
        DRec.name->bufString.text[len] = '\0';
        list_tail->next_dir = (dir_list *) malloc(sizeof(dir_list));
        list_tail->next_dir->prev_dir = list_tail;
        list_tail = list_tail->next_dir;
        list_tail->next_dir = DIR_NULL;
        list_tail->dir_name = (char *)
          malloc(strlen(cur_dir->dir_name) + len + 1);
        strcpy(list_tail->dir_name, cur_dir->dir_name);
        strcat(list_tail->dir_name, DRec.name->bufString.text);
      }
    }
    CloseGS(&dirRef);
    dirRef.refNum = 0;
    if (err = toolerror())
      puke("closing dir", NULL);
    cur_dir = cur_dir->next_dir;
  }
  free((void *) ORec.pathname);
  free((void *) DRec.name);
  free((void *) PRec.buffer.setPrefix);
  if (program == MV) {
    i = interactive;
    interactive = 0;
  }
  free_lists(list, list_tail, sourcedir);
  if (program == MV)
    interactive = i;
}

/*  This function gets called if the program is used in 'rm' mode. */
void rm(int argc, char *argv[], int optind)
{
  int filestat;

  if (force)
    interactive = 0;

  if (optind == argc)
    usage();

  for (; optind < argc; ++optind) {
    sourcefile = expand(argv[optind]);
    filestat = isadir(sourcefile);
    if (!filestat)
      remove_it(sourcefile);
    else if ((filestat == DIR) && (recurse)) {
      traverse_dir(free_n_remove, remove_file, sourcefile, CHAR_NULL);
    }
    else if (filestat == DIR)
      printf("rm: %s directory\n", sourcefile);
    else
      printf("rm: %s does not exist\n", argv[optind]);
    free((void *) sourcefile);
  }
}

/* This gets called if the program is used in 'cp' mode. */
void cp(int argc, char *argv[], int optind)
{
  int destdirstat, srcdirstat;

  if ((argc - optind) < 2)
    usage();    /* If less than two files were given, tell 'em how it's done */

  destdir = expand(argv[argc - 1]);
  destdirstat = isadir(destdir);
  destfile = (char *) malloc(strlen(destdir));
  if ((destdirstat != DIR) && ((argc - optind) != 2))
    usage();    /* If more than two files, but dest isn't a dir... */

  CompactMem();
  maxmem = (LongWord)(MaxBlock() >> 2); /* MaxBlock div 4 */
  buffer = NewHandle(maxmem, MyID, attrLocked, NULL);
  if (err = toolerror())
    puke("getting memory for copy", NULL);

  BeginSessionGS(&SRec);
  for (; optind < argc - 1; ++optind) {
    sourcefile = expand(argv[optind]);
    strcpy(destfile, destdir);
    srcdirstat = isadir(sourcefile);
    if (destdirstat == DIR)           /* If dest is a dir,               */
      fixname();                      /* fix destfile name               */
    switch (srcdirstat) {
      case DIR:
        if (!recurse)   /* if source is a dir, but no recurse, go to next */
          break;
        if (destdirstat == NON_DIR) /* if dest is not a dir, go to next */
          break;
        traverse_dir(free_only, copy_file, sourcefile, destfile);
        break;
      case NON_DIR:
        if (open()) {   /* if source not a dir, copy the file */
          copy();
          close();
        }
        break;
      default:          /* if source doesn't exist, print a nice message */
        fprintf(stderr,"cp: %s does not exist\n", argv[optind]);
	exit(1);
    }
    free((void *) sourcefile);
  }
  EndSession(&SRec);
  DisposeHandle(buffer);
  free((void *) destfile);
  free((void *) destdir);
}

/* This function gets called if the program is run in 'mv' mode */
void mv(int argc, char *argv[], int optind)
{
  int destdirstat, srcdirstat, temp;

  force = recurse = 0;

  if ((argc - optind) < 2)
    usage();    /* If less than two files were given, tell 'em how it's done */

  destdir = expand(argv[argc - 1]);
  destdirstat = isadir(destdir);
  destfile = (char *) malloc(strlen(destdir));

  if ((destdirstat != DIR) && ((argc - optind) != 2))
    usage();    /* If more than two files, but dest isn't a dir... */

  CompactMem();
  maxmem = (LongWord)(MaxBlock() >> 2); /* MaxBlock div 4 */
  buffer = NewHandle(maxmem, MyID, attrLocked, NULL);
  if (err = toolerror())
    puke("getting memory for move", NULL);

  BeginSessionGS(&SRec);
  for (; optind < argc - 1; ++optind) {
    sourcefile = expand(argv[optind]);
    strcpy(destfile, destdir);
    srcdirstat = isadir(sourcefile);
    if (destdirstat == DIR)           /* If dest is a dir,               */
      fixname();                      /* fix destfile name               */
    switch (srcdirstat) {
      case DIR:
        if (destdirstat == NON_DIR) /* if dest is not a dir, go to next */
          break;
        if (!move(sourcefile, destfile))
          traverse_dir(free_n_remove, move_file, sourcefile, destfile);
        break;
      case NON_DIR:
        if (!move(sourcefile, destfile)) {
          if (open()) {
            copy();
            close();
	    temp = interactive;
	    interactive = 0;
            remove_it(sourcefile);
	    interactive = temp;
          }
        }
        break;
      default:          /* if source doesn't exist, print a nice message */
        fprintf(stderr,"mv: %s does not exist\n", argv[optind]);
	exit(1);
    }
    free((void *) sourcefile);
  }
  EndSession(&SRec);
  DisposeHandle(buffer);
  free((void *) destfile);
  free((void *) destdir);
}

/* Gets info from the command line, and calls the appropriate func. */
int main(int argc, char *argv[])
{
  int c;

  MyID = MMStartUp();

  progname = strrchr(argv[0],':');
  if (progname == NULL) {
  	progname = strrchr(argv[0],'/');
    if (progname == NULL) progname = argv[0];
    else progname++;
  }
  else progname++;

  while ((c = getopt(argc, argv, "virfp:")) != EOF) {
    switch (c) {
    case 'v':
      ++verbose;
      break;
    case 'i':
      ++interactive;
      break;
    case 'r':
      ++recurse;
      break;
    case 'f':
      ++force;
      break;
    case 'p':
      progname = optarg;
      break;
    default:
      usage();
      break;
    }
  }

  if (!strcmp(progname, "cp"))
    program = CP;
  else if (!strcmp(progname, "rm"))
    program = RM;
  else if (!strcmp(progname, "mv"))
    program = MV;
  else {
    printf("Invalid program type.  Try cp, rm, or mv...\n");
    exit(-1);
  }

  gno = kernStatus();
  if (err=toolerror())
    gno = 0;
    
  if (gno) {
    signal(SIGTERM, handler);
    signal(SIGINT, handler);
  }
  else {
    LoadOneTool(0x6, 0x300);
    if (err=toolerror())
      puke("loading Event Manager", NULL);
    EMHandle = NewHandle((LongWord)0x100, MyID, 0xC005, NULL);
    EMStartUp((Word)*EMHandle,(Word)0,(Word)1,(Word)2,(Word)1,(Word)2,MyID);
    if (err=toolerror())
      puke("starting up the Event Manager", NULL);
    ERec = (EventRecord *) malloc(sizeof(EventRecord));
  }
  
  switch(program) {
    case CP:
      cp(argc, argv, optind);
      break;
    case RM:
      rm(argc, argv, optind);
      break;
    case MV:
      mv(argc, argv, optind);
      break;
    default:
      break;
  }

  if (!gno) {
    EMShutDown();
    DisposeHandle(EMHandle);
    if (err=toolerror())
      puke("disposing buffer", NULL);
    free((void *) ERec);
  }
  return 0;
}
