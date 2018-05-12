#pragma databank 1
#pragma rtl

#include <gsos.h>
#include <stdlib.h>
#include <string.h>
#include <menu.h>
#include <window.h>
#include <control.h>
#include <loader.h>
#include <locator.h>
#include <memory.h>
#include <resources.h>
#include <misctool.h>

/*#pragma optimize -1*/

unsigned MYREQUESTPROC();

char fname[34],regexp[34];
word miOut[4];
char *finder = "\pApple~Finder~";
char *wildicon = "\pProcyonInc~WildIcons~";
word UID;

void main(void)
{
word curApp;

    *fname = 0;
    *regexp = 0;
    UID = userid()&0xF0FF;
    curApp = GetCurResourceApp();
    ResourceStartUp(UID);
    SetCurResourceApp(curApp);
    AcceptRequests(wildicon,UID,MYREQUESTPROC);
}                           

/*
 *	apply the regular expression rexp to the GS/OS filenames in names
 *	(stringList format) and modify the list so that only names which
 *	match the regexp are retained.
 */

word doWCSelect(char *rexp, handle names, int mode)
{
byte *p,*pi;
word count,i,j,matches = 0,leng;
GSString255Ptr gs;
extern int REGEXP(word flag, char *text, char *pattern);

    HLock(names);
    p = (byte *) *names;
    count = *((word *) p);

    p+=2;
    pi = p;
    for (i = 0; i < count; i++) {
    	gs = (GSString255Ptr) p;
        /* locate the first character of the filename */
        leng = gs->length;
        for (j = leng-1; gs->text[j] != ':'; j--);
        j++;
        memcpy(fname,&gs->text[j],(size_t) leng-j);
        fname[leng-j] = 0;

	if (REGEXP(0,fname,rexp) == mode) {	/* retain the item */
            if (pi != p)		/* don't copy if we're the same */
            	memmove(pi,p,(size_t) leng+2);
            matches++;
            pi += (leng+2);
        }
        p += (leng+2);
    }
    p = (byte *) *names;
    *((word *) p) = matches;
    HUnlock(names);
    return matches;
}

struct gwi {
    word recvCount;
    word finderResult;
    handle theHandle;
    handle GSIhandle;
};

#pragma toolparms 1
void DrawContent(void)
{
    DrawControls(GetPort()); /* Draw controls in current window */
}
#pragma toolparms 0

void doInfo(int foobar)
{
WindowPtr wp;
WmTaskRec tr;
int event;
int done = 0;

    wp = NewWindow2(NULL,NULL,(VoidProcPtr) DrawContent,NULL,2,0xFF9l,0x800E);
    tr.wmTaskMask = 0x001FFFFFL & ~tmContent & ~tmMenuKey & ~tmMenuSel
      & ~tmScroll & ~tmInfo;
    do {
        event = TaskMaster(everyEvent,&tr);
	switch (event) {
	    case wInGoAway :
	        done = 1; break;
	    default: break;
	}
    } while (!done);
    CloseWindow(wp);
}

struct ndaStruct {
    word status;
    longword openProc;
    longword closeProc;
    longword actionProc;
    longword initProc;
    word period;
    word eventMask;
    longword lastServiced;
    WindowPtr windowPtr;
    handle ndaHandle;
    word memoryID;
};

extern void CloseIt();
extern void ActionIt();

struct ndaStruct ndas =
 { 0, 0l, (longword)CloseIt, (longword)ActionIt,
   0l, 0, 0xFFFF, 0l, 0l, 0l, 0 };

struct auxWindInfo {
    word size;
    word bank;
    word DP;
    word resValue;
    handle oldHandle;
    GrafPortPtr oldPort;
    longword layer;
    word minV;
    word minH;
    struct ndaStruct *NDAPtr;
};

struct windowInfo {
    word recvCount;
    word finderResult;
    word windType;
    word windView;
    word windFST;
    longword windTitle;
    longword windPath;
    longword reserved1;
    longword reserved2;
};                     

GrafPortPtr win = NULL;

GrafPortPtr checkTopWin(void)
{
GrafPortPtr topWin;
unsigned found = 0;
static struct windowInfo wi;

    topWin = FrontWindow();
    if (_toolErr == 0x100C) topWin = NULL;
    else
    while ((!found) && (topWin != NULL)) {
	if (!GetSysWFlag(topWin)) {
            SendRequest(0x8013, 1, finder, topWin, &wi);
            if (wi.windType == 2) { found = 1; continue; }
        }
        topWin = GetNextWindow(topWin);
    }
    /* no desktop operations- they blow up right now */
    if (topWin == NULL) {
        AlertWindow(5,NULL,1l);
        return NULL;
    }
    return topWin;
}

void doControl(longword ID)
{
GrafPortPtr topWin;
word num;
handle th;
struct gwi dataOut;

    if (ID == 2l) {
    	if ((topWin = checkTopWin()) == NULL) return;
        GetLETextByID(win,1l,regexp);
        regexp[regexp[0]+1] = 0;
        SendRequest(0x8012,1,finder,topWin,&dataOut);
        if (dataOut.finderResult)
	    SysBeep2(0xb);
        else {
            th = dataOut.theHandle;
            if (doWCSelect(regexp+1,th,1))
            	SendRequest(0x8005,1,finder,th,&dataOut);
	        DisposeHandle(th);
        }
    } else if (ID == 3l) num = 1/* Find File */;
    else if (ID == 5l) {
    	if ((topWin = checkTopWin()) == NULL) return;
    	GetLETextByID(win,1l,regexp);
        regexp[regexp[0]+1] = 0;
        SendRequest(0x8004,1,finder,topWin,&dataOut);
        if (dataOut.finderResult)
            SysBeep2(0xb);
        else {
            th = dataOut.GSIhandle;

            /* tell the Finder to deselect all icons */
            num = *((word*)*th);
            *((word *)*th) = 0;
            SendRequest(0x8005,1,finder,th,&dataOut);
            *((word *)*th) = num;
            if (doWCSelect(regexp+1,th,0))
            	SendRequest(0x8005,1,finder,th,&dataOut);
	    DisposeHandle(th);
        }
    } else if (ID == 7l) doInfo(9);
}

void CActionIt(WmTaskRecPtr xy, word code)
{
word x;
static WmTaskRec tr;

    switch (code) {
	case 1: /* eventAction */
	    memcpy(&tr,xy,16l);
            tr.wmTaskMask = 0x1FFFFFl;
            x = TaskMasterDA(0,&tr);
            switch (x) {
	        case wInControl:
                    doControl((long)tr.wmTaskData4);
                    break;
            }
            break;
    }
}

void doWI(int foobar)
{
word quit = 0;
struct auxWindInfo *auxPtr;
WindowPtr oldp;

    /* Find out which Window to work with; if there is no top window,
       then we're SOL */

    if (win != NULL) {
        SelectWindow(win);
        return;
    }
    oldp = GetPort();
    win = NewWindow2(NULL,NULL,DrawContent,NULL,2,0xFFAl,0x800E);
    ndas.memoryID = userid()&0xF0FF;
    if (*regexp) SetLETextByID(win,1l,regexp);
    auxPtr = (struct auxWindInfo *) GetAuxWindInfo(win);
    auxPtr->NDAPtr = &ndas;
    SetSysWindow(win);
    DrawControls(win);

    /* we just open and draw the window; the Desk Manager handles
       everything else */
    /* while (!quit) {
    	ID = DoModalWindow(&MyEventRecord,NULL,NULL,NULL,0x8018);
    }  */
/*    CloseWindow(win);
    InitCursor(); */
    SetPort(oldp);
}

MenuItemTemplate WIitem = {0,0,0,0,0,0x00,(Ref) "\pWild Icons!"};

#pragma toolparms 1
pascal unsigned MYREQUESTPROC(unsigned request, long dataIn, struct gwi *dataOut)
{
static word rfNum;
static word menuid;
GSString255Ptr ourname;
word curApp;

    switch (request) {
	case 0x0100: /* finderSaysHello */
            SendRequest(0x800F,1,finder,&WIitem,miOut);
            menuid = miOut[2];
            ourname = (GSString255Ptr) LGetPathname2(UID,1);
            curApp = GetCurResourceApp();
            SetCurResourceApp(UID);
            rfNum = OpenResourceFile(1,NULL,ourname);
            SetCurResourceApp(curApp);
            return 0x8000;
        case 0x0108: /* finderSaysExtrasChosen */
            if ((dataIn & 0xffff) == menuid) {
            	curApp = GetCurResourceApp();
            	SetCurResourceApp(UID);
		doWI(8);
            	SetCurResourceApp(curApp);
                return 0x8000;
            }
            break;
        case 0x0101:
            curApp = GetCurResourceApp();
            SetCurResourceApp(UID);
            CloseResourceFile(rfNum);
            SetCurResourceApp(curApp);
            return 0x8000;
	case 0x0003:
    	    AcceptRequests(wildicon,UID,NULL);
	    dataOut->finderResult = UID;
	    curApp = GetCurResourceApp();
	    SetCurResourceApp(UID);
	    ResourceShutDown();
	    SetCurResourceApp(curApp);
            return 0x8000;
    }
    return 0;
}
#pragma toolparms 0
