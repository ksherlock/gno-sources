/*

    l3.c

    ORCA/C functions that provide nice interfaces to
      GNO Kernel routines

    Major use: compatibility with Unix, new functions, and
      better interface with C.

    Source is provided so that modifications for use with APW/C can be
      made if desired.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <errno.h>
#include <gsos.h>
#include <sys/stat.h>
#include <gno/gno.h>
#include <signal.h>
#include <sgtty.h>
#include <unistd.h>

#pragma noroot
#pragma optimize 9

GSString255Ptr __C2GSMALLOC(char *s)
{
GSString255Ptr g;
int l;
    l = strlen(s);
    g = malloc(l+2);
    g->length = l;
    memcpy(g->text,s,l);
    return g;
}

char *__GS2CMALLOC(GSString255Ptr g)
{
char *s;
int l;
    l = g->length;
    s = malloc(l+1);
    memcpy(s,g->text,l);
    s[l] = 0;
    return s;
}

char *__GS2C(char *s, GSString255Ptr g)
{
int l;
    memcpy(s,g->text,g->length);
    s[g->length] = 0;
    return s;
}
/*#pragma optimize 0*/

segment "gnolib    ";

typedef struct GetDevNumberRecGS {
    int pCount;
    GSString255Ptr devName;
    int devNum;
} GetDevNumberRecGS;

/*  sleep() code from BSD 4.3-reno, modified for use with GNO  */

static int ringring;

unsigned int sleep(unsigned int seconds)
{
	register struct itimerval *itp;
	struct itimerval itv, oitv;
/*	struct sigvec vec, ovec;  */
	void (*ovec)();
	long omask;
	void sleephandler(int,int);

#ifdef NOTNEEDED
	itp = &itv;
	if (!seconds)
		return;
	timerclear(&itp->it_interval);
	timerclear(&itp->it_value);
	if (setitimer(ITIMER_REAL, itp, &oitv) < 0)
		return;
	itp->it_value.tv_sec = seconds;
	if (timerisset(&oitv.it_value)) {
		if (timercmp(&oitv.it_value, &itp->it_value, >))
			oitv.it_value.tv_sec -= itp->it_value.tv_sec;
		else {
			itp->it_value = oitv.it_value;
			/*
			 * This is a hack, but we must have time to return
			 * from the setitimer after the alarm or else it'll
			 * be restarted.  And, anyway, sleep never did
			 * anything more than this before.
			 */
			oitv.it_value.tv_sec = 1;
			oitv.it_value.tv_usec = 0;
		}
	}
#endif
/*	setvec(vec, sleephandler);  */
	ovec = signal(SIGALRM, sleephandler);
/*	(void) sigvec(SIGALRM, &vec, &ovec);  */
	omask = sigblock(sigmask(SIGALRM));
	ringring = 0;
/*	(void) setitimer(ITIMER_REAL, itp, (struct itimerval *)0); */
        alarm(seconds);
	while (!ringring)
		sigpause(omask &~ sigmask(SIGALRM));
	signal(SIGALRM, ovec);
/*	(void) sigvec(SIGALRM, &ovec, (struct sigvec *)0);  */
	(void) sigsetmask(omask);
/*	(void) setitimer(ITIMER_REAL, &oitv, (struct itimerval *)0);  */
}

#pragma databank 1
static void
sleephandler(int sig, int code)
{
	ringring = 1;
}
#pragma databank 0

int pause(void)
{
    sigpause(0l);
    return -1;
}

static char *EXcmdline, *EXfilename;

#pragma databank 1
static void exec2(void)
{
   execve(EXfilename,EXcmdline);
   perror(EXfilename);
}
#pragma databank 0

int exec(char *filename, char *cmdline)
{
   EXcmdline = cmdline; EXfilename = filename;
   return fork(exec2);
}

int needsgno(void)
{
    kernStatus();
    if (_toolErr) return 0;
    else return 1;
}

int _INITGNOSTDIO(void)
{
extern FILE *fdopen(int,char *);
struct stat sb;
extern FILE *s_stdin,*s_stdout,*s_stderr;
    	
    kernStatus();
    if (_toolErr) { return 0; }
       /*fclose(stdin);
     fclose(stdout);
     fclose(stderr);*/
    s_stdin = stdin;
    s_stdout = stdout;
    s_stderr = stderr;
    stdin = fdopen(1,"r");
    stdout = fdopen(2,"w");
    fstat(2,&sb);
    if (sb.st_mode & S_IFCHR) setvbuf(stdout,NULL,_IOLBF,256l);
    stderr = fdopen(3,"w");
    setbuf(stderr,NULL); /* stderr ALWAYS defaults to no buffering */
    return 1;
}

int _mapErr(int err)
{
    if (!err) return 0;
    switch (err) {
    	case 0x43: return EBADF;

    	case 0x44:
    	case 0x45:
    	case 0x46: return ENOENT;

    	case 0x47: return EEXIST;

    	case 0x48:
    	case 0x49: return ENOSPC;
        case 0x4A: return ENOTDIR;

    	case 0x4B:
    	case 0x4F:
    	case 0x53: return EINVAL;
    	case 0x54: return ENOMEM;
    	case 0x4E: return EACCESS;
    	case 0x58: return ENOTBLK;
    	default:   return EIO;
    }
}
    
size_t read(int filds, void *buf, size_t count)
{
IORecGS i = {4, filds, buf, (long) count, 0l};
int err;

    ReadGS(&i);
    if (_toolErr == 0x4C) return (size_t) i.transferCount;
    if (err = _mapErr(_toolErr)) { errno = err; return -1; }
    return (size_t) i.transferCount;
}
         
size_t write(int filds, void *buf, size_t count)
{
IORecGS i = {4, filds, buf, (long) count, 0l};
int err;

    WriteGS(&i);
    if (err = _mapErr(_toolErr)) { errno = err; return -1; }
    return (size_t) i.transferCount;
}

int close(int filds)
{
int cl[2] = {1, filds};
int err;

    CloseGS(cl);
    if (err = _mapErr(_toolErr)) { errno = err; return -1; }
    return 0;
}

int creat(const char *path, int mode)
{
OpenRecGS o;
CreateRecGS c;
GSString255Ptr p;
longword l;
int acc,err;

    l = strlen(path);
    p = malloc(l+2);
    p->length = l;
    memcpy(p->text,path,l);
    c.pCount = 1;
    c.pathname = p;
    DestroyGS(&c);
    if ((err = _toolErr) && (err != 0x46)) goto doerr;
    c.pCount = 4;
    acc = 0xC0; /* default rename and delete are _on_ */
    if (mode & 0x0100) acc |= 0x01;
    if (mode & 0x0080) acc |= 0x02;
    if (mode & 0x8000) acc |= 0x04;
    if (mode & 0x1000) acc |= 0x80;
    if (mode & 0x2000) acc |= 0x40;
    if (mode & 0x4000) acc |= 0x20;

    c.access = acc;
    c.fileType = 4;
    c.auxType = 0l;
    CreateGS(&c);
    if (err = _toolErr) goto doerr;

    o.pCount = 3;
    o.pathname = p;
    o.requestAccess = acc & 3;
    OpenGS(&o);
    if (!(err = _toolErr)) {
        free(p);
        return o.refNum;
    }
doerr:
    if (err = _toolErr) {
        free(p);
        errno = _mapErr(err);
        return -1;
    }
}    

/*#pragma optimize 8*/
int open(const char *path, int oflag, ...)
{
OpenRecGS o;
CreateRecGS c;
SetPositionRecGS s;
GSString255Ptr p;
int err,l,acc;
int mode;
va_list list;

    va_start(list, oflag);
    if (oflag & O_CREAT) mode = va_arg(list,int);

    l = strlen(path);
    p = malloc(l+2);
    p->length = l; memcpy(p->text,path,l);
    o.pathname = p;
    o.pCount = 12;
    o.optionList = NULL;
    o.resourceNumber = 0;

    if (oflag == 0) acc = 0;
    else if (oflag & O_RDONLY) acc = 1;
    else if (oflag & O_WRONLY) acc = 2;
    else if (oflag & O_RDWR) acc = 3;
    o.requestAccess = acc;

    OpenGS(&o);
    if ((err = _mapErr(_toolErr)) && (err == ENOENT)) {
	if (oflag & O_CREAT) {
	    c.pCount = 3;
            c.pathname = p;
            c.access = 0xC3;
            c.fileType = (oflag & O_BINARY) ? 6 : 4;
            CreateGS(&c);
            if (err = _mapErr(_toolErr)) goto errxit;
            OpenGS(&o);
            if (err = _mapErr(_toolErr)) goto errxit;
        }
        else { errno = err; va_end(list); return -1; }
    } else if (err) { errno = err; va_end(list); return -1; }
    if ((oflag & O_APPEND) && !(oflag & O_RDONLY)) {
	s.pCount = 3;
        s.refNum = o.refNum;
        s.base = 0;
        s.displacement = o.eof;
        SetMarkGS(&s);
        if (err = _mapErr(_toolErr)) goto errxit;
    }
    free(p);
    va_end(list);
    return o.refNum;
errxit: free(p);
	errno = err;
    va_end(list);
    return -1;
}

/*#pragma optimize 0*/

long lseek(int filds, long offset, int whence)
{
SetPositionRecGS s;
PositionRecGS m;
EOFRecGS e;
int err;
	
    e.pCount = m.pCount = 2;
    e.refNum = s.refNum = m.refNum = filds;
    GetEOFGS(&e);
    if (err = _mapErr(_toolErr)) { errno = err; return -1l; }
    GetMarkGS(&m);

    s.pCount = 3;
    s.base = 0;
    switch (whence) {
	case 0: s.displacement = offset; break;
        case 1: s.displacement = m.position + offset; break;
        case 2: s.displacement = e.eof + offset; break;
	default: errno = EINVAL; return -1l;
    }
    if (s.displacement < 0) { errno = EINVAL; return -1l; }
    if (s.displacement > e.eof) {
        SetEOFGS(&s);
        if (err = _mapErr(_toolErr)) { errno = err; return -1l; }
    }
    SetMarkGS(&s);
    if (err = _mapErr(_toolErr)) { errno = err; return -1l; }
    return s.displacement;
}

char *_errnoText[] = {
"unknown error",
"Domain error",
"Range error",
"Not enough memory",
"No such file or directory",
"I/O error",
"Invalid argument",
"Bad file number",
"Too many open files",
"Permission denied",
"File exists",
"File too large",
"Not owner",
"No such process",
"Interrupted system call",
"Arg list too long",
"Exec format error",
"No children",
"No more processes",
"Not a directory",
"Not a terminal",
"Broken pipe",
"Illegal seek",
"No space left on device"
"Not a socket",
"Destination address required",
"Message too long",
"Wrong protocol for socket",
"Protocol not available",
"Protocol not supported",
"Operation not supported on socket",
"Protocol family not supported",
"Address family not supported",
"Address in use",
"Can't assign requested address",
"Network is down",
"Network is unreachable",
"Network dropped connection",
"Connection aborted",
"Connection reset by peer",
"No buffer space available",
"Socket is already connected",
"Socket is not connected",
"Can't send after socket shutdown",
"Too many references: can't splice",
"Connection timed out",
"Connection refused"
};

char *strerror(int errnum)
{
    if (errnum > ECONNREFUSED) errnum = 0;
    return _errnoText[errnum];
}

void perror(char *s)
{
    fprintf(stderr,"%s: %s\n",s,_errnoText[(errno > 23) ? 0 : errno]);
}

int stty(int filedes, struct sgttyb *argp)
{
    return ioctl(filedes,TIOCSETP,argp);
}

int gtty(int filedes, struct sgttyb *argp)
{
    return ioctl(filedes,TIOCGETP,argp);
}

int isatty(int filedes)
{
struct stat sb;

    if (fstat(filedes,&sb) == -1) return 0;
    if (sb.st_mode & S_IFCHR) return 1;
    else return 0;
}

int unlink(char *fname)
{
    return remove(fname);
}

int mkdir(char *dirname)
{
GSString255Ptr dn;
CreateRecGS cr;
long int l;

    l = strlen(dirname);
    dn = malloc(l+2);
    dn->length = l;
    memcpy(dn->text,dirname,l);
    cr.pCount = 5;
    cr.pathname = dn;
    cr.access = 0xC3;
    cr.fileType = 0x0F;
    cr.auxType = 0l;
    cr.storageType = 0x0D;
    CreateGS(&cr);
    free(dn);
    if (_toolErr) { errno = _mapErr(_toolErr); return -1; }
    return 0;
}

#include <sys/vfs.h>

int statfs(char *path, struct statfs *buf)
{
GSString255Ptr inp;
ResultBuf255Ptr oup;
ExpandPathRecGS ep;
GetDevNumberRecGS gd;
VolumeRecGS vo;
int i;

    inp = malloc(strlen(path)+2);
    inp->length = strlen(path);
    memcpy(inp->text,path,inp->length);
    oup = malloc(256l);
    oup->bufSize = 252;
    ep.pCount = 2;
    ep.inputPath = inp;
    ep.outputPath = oup;
    ExpandPathGS(&ep);
    if (_toolErr) {
    	errno = _mapErr(_toolErr); free(inp); free(oup); return -1;
    }
    gd.pCount = 2;
    gd.devName = &oup->bufString;
    GetDevNumberGS(&gd);
    if (_toolErr) {
        errno = _mapErr(_toolErr); free(inp); free(oup); return -1;
    }
    memcpy(inp->text,".dx",3l);
    inp->length = 3;
    inp->text[2] = gd.devNum + '0';

    vo.pCount = 6;
    vo.devName = (GSString32Ptr) inp;
    vo.volName = NULL;
    VolumeGS(&vo);
    if (_toolErr) { errno = _mapErr(_toolErr); return -1; }
    buf->f_type = 0l;
    buf->f_bsize = vo.blockSize;
    buf->f_blocks = vo.totalBlocks;
    buf->f_bfree = buf->f_bavail = vo.freeBlocks;
    buf->f_files = buf->f_ffree = 0l;
    buf->f_fsid.val[0] = 0l;
    buf->f_fsid.val[1] = 0l;
    free(inp); free(oup);
    return 0;
}

int fsync(int fd)
{
int ff[2];

  ff[0] = 1;
  ff[1] = fd;
  FlushGS(ff);
  if (_toolErr) { errno = _mapErr(_toolErr); return -1; }
  else return 0;
}

int ftruncate(int fd, off_t length)
{
SetPositionRecGS p;

   p.pCount = 3;
   p.base = 0;
   p.refNum = fd;
   p.displacement = length;
   SetEOFGS(&p);
   if (_toolErr) { errno = _mapErr(_toolErr); return -1; }
   return 0;
}

int access(char *name, int mode)
{
FileInfoRecGS f;
long l;
int err;
extern GSString255Ptr __C2GSMALLOC(char *);

    f.pCount = 4;
    f.pathname = __C2GSMALLOC(name);
    GetFileInfoGS(&f);
    err = _toolErr;
    free(f.pathname);
    if (err) { errno =  _mapErr(err); return -1; }
    switch (mode) {

	case F_OK:	return 0;
        case X_OK:      if ((f.fileType == 0xb3) || (f.fileType == 0xb5) ||
		           (f.fileType == 0xB0 && f.auxType == 6))
                        return 0; else goto ferb;
        case W_OK:	if (f.access & 2) return 0; else goto ferb;
        case R_OK:	if (f.access & 1) return 0; else goto ferb;
    }
ferb: errno = EACCESS;
      return -1;
}
