#pragma nda DAOpen DAClose DAAction DAInit 0 0xffff "  GNO GSI\\H**"

/* GSI                  */
/* Derek Taubert        */

#include "pipe.h"
#include "gsi.h"
extern int needsgno(void);
#define NULL1 ((Pointer)NULL)

/* really global variables  */
boolean     myWindOpen,FMactive,TEactive;
Handle      FMZeroPageHandle,TEZeroPageHandle;
EventRecord ERec;

/* global variables         */
WindowPtr   myWindPtr;
int         gMyMemoryID;

myTEFormat teF = {0,sizeof(myTERuler),{0,0,400,0,0,0,0l,0},
                    sizeof(TEStyle),{102,0x00,8,0x0000,0xffff,0l},
                    1l,{0l,0l}};

TextEditTemplate      TEtmp = {{13,102l,{25,0,0,0},editTextControl,0x0000,
    0x7c00,0l},0x42a80000l,{0xffff,0xffff,0xffff,0xffff},(CtlRecHndl)-1l,
    0x0000,0l,0,(Ref)&teF};

SimpleButtonTemplate  Btmp  = {{7,101l,{1,4,13,64},simpleButtonControl,0x0002,
                                 0x1000,0l},(Ref)"\pDie"};

CtlRecHndl            BRecHndl,TERecHndl,PopHndl;

ParamList myWind = {sizeof(ParamList),0xC0E0,"\pGSI v0.0",0l,{0,0,0,0},
                     NULL,0,0,0,0,0,0,0,0,0,0,0l,0,NULL,NULL,NULL,
                     {40,120,151,600},(WindowPtr)(-1l),NULL};

void bzero(void *,size_t);

GrafPortPtr DAOpen(void)
{
GrafPortPtr currPort;
int oops,e;
static char dumb[256];

    if (myWindOpen)
        SelectWindow(myWindPtr);
    else {
        if (!needsgno()) {
            oops = AlertWindow(0x0000,NULL1,
              (Ref)"22/GNO GSI requires that the GNO kernel be active./Oops");
            return NULL;
        }

        LoadOneTool(0x1b,0x0204);
        if ((e = toolerror()) != 0) {
            sprintf(dumb,"$%04X",e);
            oops = AlertWindow(0x0000,(Pointer)&dumb,
              (Ref)"22/Error *0 loading Font Manager Tool/Oops");
            return NULL;
        }
        LoadOneTool(0x22,0x0100);
        if ((e = toolerror()) != 0) {
            sprintf(dumb,"$%04X",e);
            oops = AlertWindow(0x0000,(Pointer)&dumb,
              (Ref)"22/Error *0 loading TextEdit Tool/Oops");
            return NULL;
        }

        gMyMemoryID = MMStartUp();
        if (!(FMactive = FMStatus())) {
            FMZeroPageHandle = NewHandle(0x00000100l,gMyMemoryID,
                attrBank|attrPage|attrFixed|attrLocked,NULL);
            if ((e = toolerror()) != 0) {
                sprintf(dumb,"$%04X",e);
                oops = AlertWindow(0x0000,(Pointer)&dumb,
                  (Ref)"22/Error *0 allocating Font Manager Tool memory/Oops");
                MMShutDown(gMyMemoryID);
                return NULL;
            }
            FMStartUp(gMyMemoryID,(word)*FMZeroPageHandle);
        }
        if (!(TEactive = TEStatus())) {
            TEZeroPageHandle = NewHandle(0x00000100l,gMyMemoryID,
                attrBank|attrPage|attrFixed|attrLocked,NULL);
            if ((e = toolerror()) != 0) {
                sprintf(dumb,"$%04X",e);
                oops = AlertWindow(0x0000,(Pointer)&dumb,
                  (Ref)"22/Error *0 allocating TextEdit Tool memory/Oops");
                if (!FMactive) {
                    FMShutDown();
                    DisposeHandle(FMZeroPageHandle);
                }
                MMShutDown(gMyMemoryID);
                return NULL;
            }
            TEStartUp(gMyMemoryID,(word)*TEZeroPageHandle);
        }

        initPipe();
        myWindPtr = NewWindow(&myWind);
        SetSysWindow(myWindPtr);

        currPort = GetPort();
        SetPort(myWindPtr);
/* any window updates go here */
       /*SetFontFlags(2);*/
        SetPort(currPort);

        BRecHndl = NewControl2(myWindPtr,refIsPointer,(Ref)(&Btmp));
        TERecHndl = NewControl2(myWindPtr,refIsPointer,(Ref)(&TEtmp));
        SetPort(myWindPtr);
    	SetFontFlags(2);
        SetPort(currPort);
        
        myWindOpen = true;
    }
    return myWindPtr;
}

void DAClose(void)
{
    closePipe();
    CloseWindow(myWindPtr);
    myWindOpen = FALSE;
    if (!TEactive) {
        TEShutDown();
        DisposeHandle(TEZeroPageHandle);
    }
    if (!FMactive) {
        FMShutDown();
        DisposeHandle(FMZeroPageHandle);
    }
    MMShutDown(gMyMemoryID);
}

extern int globfio;

int DAAction (EventRecordPtr Param, int Code)
{
GrafPortPtr currPort;
int whocares;
static char tmp[200];

    switch (Code) {
        case eventAction : {
                switch (Param->what) {
                    case keyDownEvt :
                    case autoKeyEvt :
                        sendchar((char *)&(Param->message));
                        break;
    /* eventually, we'll take out char events to the TE, and it will be
       changed to non-editable */
                    case mouseDownEvt :
    /* why? why?    case mouseUpEvt : */
                        memcpy(&ERec,Param,(size_t)16l);
                        ERec.wmTaskMask = 0x001f7fff;
                        whocares = TaskMasterDA(0x0000,&ERec);
                        if (whocares == wInControl)
                            if (ERec.wmTaskData4 == 101)
                                /* button pressed */
                            if (ERec.wmTaskData4 == 102) {
                                /* text edit */
                            }
                        break;
                    case updateEvt : {
                           BeginUpdate(myWindPtr);
                           updateWind();
                           DrawControls(myWindPtr);
                           EndUpdate(myWindPtr);
                        } break;
                    case activateEvt : {
                           updateWind();
                           DrawControls(myWindPtr);
                        } break;
                }
            } break;
        case runAction : {
                currPort = GetPort();
                SetPort(myWindPtr);
                consume(TERecHndl);
                SetPort(currPort);
            } break;
        case cursorAction : break;
        case undoAction : {
                return 0;
            } break;
        case cutAction : {
                TECut(TERecHndl);
                return 1;
            } break;
        case copyAction : {
                TECopy(TERecHndl);
                return 1;
            } break;
        case pasteAction : {
                TEPaste(TERecHndl);
                return 1;
            } break;
        case clearAction : {
                TEClear(TERecHndl);
                return 1;
            } break;
    }
}

void DAInit(int Code)
{
    if (Code == 0) {
        if (myWindOpen) DAClose();
    }
    else {
/* initialize really global variables here */
        myWindOpen = false;
        bzero(&ERec,sizeof(EventRecord));
    }
}
