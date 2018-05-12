/*
 *	buf.c / TelCom II
 *
 *	File Transfer Protocol buffering
 *
 */

#pragma noroot

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <gsos.h>
#include "buf.h"
#pragma optimize 9
#pragma lint -1

#define DEBUG

XFfileInfo *file_info[NUMFILES];

void *zalloc(size_t f)
{ return malloc(f); }

void zfree(void *f)
{ free(f); }

int XFgetrefnum(int ref)
{
XFfileInfo *xf = file_info[ref];
    return xf->refNum;
}

static int XFFill(int ref)
{
XFfileInfo *xf = file_info[ref];
long bytes;

    bytes = read(xf->refNum, xf->buf, XFBUFSIZE);
    if (bytes == -1) {
    	perror("read failed!");
	return -1;
    }
    if (bytes < XFBUFSIZE) xf->eof = 1;
    xf->end = bytes;
    xf->pos = 0;
    return 0;
}

int _set_mark(int refnum, unsigned long mark)
{
SetPositionRecGS sm;

    sm.pCount = 3;
    sm.refNum = refnum;
    sm.displacement = mark;
    sm.base = 0;
    SetMarkGS(&sm);
    if (_toolErr) return -1;
    else return 0;
}

unsigned long _get_eof(int refnum)
{
EOFRecGS ge;

    ge.pCount = 2;
    ge.refNum = refnum;
    GetEOFGS(&ge);
    return ge.eof;
}

int XFflush(int ref)
{
XFfileInfo *xf = file_info[ref];

    if (xf->mode == O_RDONLY) return;
    write(xf->refNum, xf->buf, xf->pos);
    xf->pos = 0;
}

int XFsetmark(int ref, unsigned long mark)
{
XFfileInfo *xf = file_info[ref];

    XFflush(ref);

    xf->pos = 0;
    xf->end = 0;
    xf->eof = 0;
    xf->mark = mark;
    return _set_mark(xf->refNum,mark);
}

int XFgetchar(int ref)
{
XFfileInfo *xf = file_info[ref];
int c;

retry:
    if (xf->pos == xf->end) {
        if (xf->eof) return -1;
	XFFill(ref);
        goto retry;
    }
    c = xf->buf[xf->pos++];
    xf->mark++;
    return c;
}

/* There are the following possible conditions when reading a block:
   o Requested data is entirely in buffer. Copy data with memcpy, and
     update variables.
   o Requested data is not entirely in buffer.
     If eof is set, return what we have.
     Copy what is in the buffer, and update variables.  repeat at step 1
*/

int XFread(int ref, char *buf, unsigned size)
{
XFfileInfo *xf = file_info[ref];
unsigned inbuf;
unsigned xfer = 0;

retry:
    inbuf = xf->end - xf->pos;
    if (size <= inbuf) {
        memcpy(buf,xf->buf+xf->pos,(size_t) size);
        xf->pos += size;
        xf->mark += size;
        xfer += size;
#ifdef DEBUG
	fprintf(stderr,"read: %d (%d)\n",ref,xfer);
#endif
        return xfer;
    }
    if (inbuf) memcpy(buf,xf->buf+xf->pos,inbuf);
    xf->pos += inbuf;
    xf->mark += inbuf;
    xfer += inbuf;
    buf += inbuf;
    size -= inbuf;
    if (xf->eof) {
#ifdef DEBUG
	fprintf(stderr,"eof read: %d (%d)\n",ref,xfer);
#endif
        if (!xfer) return -1;
        else return xfer;
    }
    XFFill(ref);
    goto retry;
}

unsigned long XFgeteof(int ref)
{
XFfileInfo *xf = file_info[ref];
#ifdef DEBUG
    fprintf(stderr,"XFgeteof: %ld\n",xf->length);
#endif
    return xf->length;
}

int XFclose(int ref)
{
XFfileInfo *xf = file_info[ref];

    XFflush(ref);
    close(xf->refNum);
    zfree(xf->buf);
    zfree(xf);
    file_info[ref] = NULL;
}

int XFopen(char *filename, unsigned mode)
{
unsigned ref;
XFfileInfo *xf;
unsigned file;

#ifdef DEBUG
    fprintf(stderr,"opening file %s\n",filename);
#endif
    for (file = 0; file < NUMFILES; file++)
        if (file_info[file] == NULL) break;
    if (file == NUMFILES) {
        /*XFerr("too many open files");*/
        return -1;
    }

    ref = open(filename,mode);
    if (ref < 0) return -1;
    xf = zalloc(sizeof(XFfileInfo));
    xf->refNum = ref;
    xf->length = _get_eof(ref);
    xf->mark = 0;
    xf->pos = 0;
    xf->end = 0;
    xf->eof = 0;
    xf->buf = zalloc(XFBUFSIZE);
    file_info[file] = xf;
#ifdef DEBUG
    fprintf(stderr,"ref: %d eof: %ld xf: %06lX buf: %06lX\n",
    	ref,xf->length,xf,xf->buf);
#endif
    return file;
}
