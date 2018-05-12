/*
 *	buf.h / TelCom II
 */


/* defined as a bit less than 32768 to avoid sign extension problems */
#define XFBUFSIZE 32000

typedef struct XFfileInfo {
    unsigned refNum;
    unsigned mode;
    unsigned long length;
    unsigned long mark;
    unsigned pos;
    unsigned end;
    unsigned eof;
    char *buf;
} XFfileInfo;

#ifdef TELCOM
int XFopenGS(GSString255Ptr filename, int mode);
int XFputchar(int ref, int c);
#endif

int XFopen(char *filename,int mode);
int XFclose(int ref);
int XFgetchar(int ref);
int XFflush(int ref);
int XFsetmark(int ref, unsigned long mark);
int XFread(int ref, char *buf, unsigned size);
unsigned long XFgeteof(int ref);
int XFgetrefnum(int ref);

#define NUMFILES 8
extern XFfileInfo *file_info[];
