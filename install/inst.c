#include <control.h>
#include <desk.h>
#include <event.h>
#include <gsos.h>
#include <lineedit.h>
#include <locator.h>
#include <menu.h>
#include <misctool.h>
#include <quickdraw.h>
#include <intmath.h>
#include <qdaux.h>
#include <resources.h>
#include <scrap.h>
#include <window.h>
#include <font.h>
#include <orca.h>
#include <stdfile.h>
#include <texttool.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "inst.h"

#pragma optimize 9


/************************************************************************
 Standard global data
************************************************************************/

long startStopAddr ; /* Space for StartStopRec from Tool Locator */
int QuitFlag ; /* Set to true when time to quit program */

/* Flag telling if the system window is the top window
   or not. This is set to the output of a GetWKind call. */
int WindType ;

WindowPtr WindowPointer; /* space for window pointer */
Handle TargetCtl ; /* Handle of current target control */

/************************************************************************
 Resource locking flag.  If you want locked memory handles on resources
   you load this flag should be 1. If 0, your handles may move around.
************************************************************************/

#define LOCKRESHANDLES 1

/************************************************************************
 LoadRes used to load resources and optionally lock the handle based on
  value of LOCKRESHANDLES.  The Handle is returned.
************************************************************************/

Handle LastResHNDL ; /* For LoadRes macro */

#if LOCKRESHANDLES  /* Setup our LoadRes macro */
#define LoadRes(type,num) (LastResHNDL=LoadResource((word)type,(long)num), \
   HLock(LastResHNDL), LastResHNDL)
#else
#define LoadRes(type,num) (LastResHNDL=LoadResource((word)type,(long)num)
#endif

/*************************************************************************
Most resources in your resource fork are called by Tool calls. If you want to
   access some directly, call this routine. Handle to the resource is returned.
**************************************************************************/

/************************** Control Lists ********************************
   Prototypes and descriptions of defined macros:

       CtlRecHndl DoControlList( long cList, WindowPtr wPtr )
           A control list is usually tied to a window.  However, you may
           wish to create a control list and call the NewControl2 call
           yourself.  This routine allows you to pass a window pointer and
           the control list resource number and returns the control handle
           (if a single control created) or 0.

**************************************************************************/
#define DoControlList(cList,wPtr)\
        NewControl2((WindowPtr) wPtr, (word) 9, (long) cList)

/************************** Control Templates ****************************
   Prototypes and descriptions of defined macros:

       CtlRecHndl DoRNewControl( long ctrl, WindowPtr wPtr )
           Controls are usually in a list tied to a window.  However,
           you may wish to create a control and call the NewControl2 call
           yourself.  This routine allows you to pass a window pointer and
           the control resource number and returns the control handle.

**************************************************************************/

#define DoRNewControl(ctrl,wPtr)\
   NewControl2((WindowPtr) wPtr, (word) 2, (long) ctrl)

/**************************** Pascal Strings *****************************
   Prototypes and descriptions of defined macros:

       char *GetRPString(long yourString)
           This macro returns a pointer to a PString in a resource.

**************************************************************************/

#define GetRPString(yourString) ((char *)(*LoadRes(rPString,yourString)))

#define FIRSTMENUBAR 1 /* First menu bar defined in resource fork */

/****************************** Menu Bars ********************************
   Prototypes and descriptions of defined macros:

       MenuBarRecHndl DoRMenuBar(long menuBarRef, WindowPtr wPtr)
           This routine gives your your choice of creating your program's
           menu bar or a menu bar in a window.  Just pass a NULL for your
           program's menu bar, or a window pointer. Returns menu bar handle.

**************************************************************************/

MenuBarRecHndl DoRMenuBar(long menuBarRef, WindowPtr wPtr)
{
   MenuBarRecHndl MyMenuBarRecHndl ;
   MyMenuBarRecHndl = NewMenuBar2((word) 2, menuBarRef, wPtr);
   if (wPtr == NULL) {   /* create program menu bar */
      SetSysBar(MyMenuBarRecHndl);
      SetMenuBar(NULL);
      RefreshDesktop(0L);   /* Refresh the whole desktop */
      FixAppleMenu(1);      /* change if apple menu not 1 */
      FixMenuBar();
   }
   DrawMenuBar();
   return MyMenuBarRecHndl ; /* Return handle to menu bar record */
}

/********************************* Menus *********************************
   Prototypes and descriptions of defined macros:

       Handle DoRMenu(long menuRef)
           This routine allocates a single menu from a resource and
           returns a handle to the menu created.  Note that the menu is
           created in memory only; if you want it to appear in some menu
           bar, call the InsertMenu toolbox call.

**************************************************************************/

#define DoRMenu(menuRef) NewMenu2((word) 2, (long) menuRef)

/****************************** Menu Items *******************************
   Prototypes and descriptions of defined macros:

       void InsertRMItem( long menuItemRef,int insertAfter,int menuNum)
           Insert a menu item from a resource into an existing menu
           using the InsertMItem2 tool call.  "insertAfter" is 0 for top of
           menu, -1 for the bottom). "menuNum" which menu item goes into

**************************************************************************/

#define InsertRMItem(menuItemRef,insertAfter,menuNum) \
   InsertMItem2(2, (long)menuItemRef, (int)insertAfter, (int)menuNum)

#define FIRSTWINDOW 4087 /* First window defined in resources */

/************************* DrawContent Routine ***************************
   Prototype and description of defined routine:

       void DrawContent(void)
           This is a generic TaskMaster drawing procedure that can be used
           by your NewWindow2 calls. Modify this routine as needed, or copy
           it for specialized cases for window drawing. This routine
           is called by TaskMaster automatically when you specify it as the
           window's draw content procedure.  Whenever your window needs to be
           redrawn by TaskMaster, the window manager will call it.

**************************************************************************/

#pragma databank 1
#pragma toolparms 1
void DrawContent(void)
{
    DrawControls(GetPort()); /* Draw controls in current window */
}
#pragma toolparms 0
#pragma databank 0

/************************ DoNewWindow Window Routine *********************
   This is a generic function that will allow you to pass the resource ID
   of your window, and will open your window, as well as set it's grafport
   for you.  It returns the window pointer of the window opened.
**************************************************************************/

WindowPtr DoNewWindow(long windowID )
{
WindowPtr wPtr;        /* Pointer to window's GrafPort */
Handle wHandle ;       /* Handle to window's resource template */
Handle colorsH ;       /* Handle to window's color resource template */

struct wTemplate {     /* Kludgy way to get at the data we need */
   char stuff1[0x14];
   long p1Color ;
   char stuff2[0x32];
   long ControlList ;
   word p1InVerb ;
} *wTemplatePtr;
   /* Apply fix for system software window color table bug */
   wHandle = LoadRes(rWindParam1, windowID); /* Get handle to resource */
   HLock(wHandle);                           /* Lock it, no matter what! */
   DetachResource(0x800E, windowID);         /* now its ours */

   wTemplatePtr = (void *) *wHandle ;        /* ref by pointer */

   /* load in color table */
   colorsH = LoadResource(0x8010, wTemplatePtr->p1Color);

   /* If no color table or error, clear ref to color resource and color ref */
   if (toolerror()) {
      if (wTemplatePtr->p1InVerb & 0x0800) {
         wTemplatePtr->p1InVerb != 0x0800;
      }
      wTemplatePtr->p1Color = 0L;
   }

   /* Color table loaded ok, so lock the handle and set refs to handle */
   else {
      DetachResource(0x8010, wTemplatePtr->p1Color);
      HLock(colorsH);
      if (wTemplatePtr->p1InVerb & 0x0800) {
         wTemplatePtr->p1InVerb ^= 0x0800;
      }
      wTemplatePtr->p1InVerb |= 0x0400;
      wTemplatePtr->p1Color = (long) colorsH;
   };

   /* Call NewWindow2 with patched color table  */
   wPtr = NewWindow2(NULL,                   /* desired window title  */
                      0x00000000L,           /* refcon for the window */
                      (VoidProcPtr)DrawContent, /* routine to draw it */
                      (LongProcPtr) NULL,    /* info draw routine     */
                      0x0001,                /* use the handle        */
                      (Ref) wHandle,        /* modified handle       */
                      rWindParam1);          /* standard type window  */

   /* Set the port */
   SetPort(wPtr);

   return wPtr;

}

#define TOOLSTARTRES 1 /* First Tool startup defined in resources */

/**************************** Tool Startup *******************************
   Prototype and description of defined routine:

      int StartUpMyTools(void)
           This routine starts up your tools.  NOTE that although
           you may have more than one tool startup table defined
           with Genesys, this routine uses the first one it encounters
           (you should only have one tool startup table defined anyway).
           If tools started, 0 returned, else -1 for error.
           returned for an error.

**************************************************************************/

#define Tool_Startup_Table 1
int StartUpMyTools(void)
{
int returncode ;
   startStopAddr = StartUpTools(userid(), 2, (long) Tool_Startup_Table);
   if (toolerror()) {
      returncode = -1; 
   } else { returncode = 0; }
   return returncode ;
}

/*************************************************************************
   Prototype and description of defined routine:

       void ShutDownMyTools(void)
           Shut down the tools started by the procedure StartUpMyTools.
**************************************************************************/

void ShutDownMyTools(void)
{
  ShutDownTools(1, startStopAddr);
    /* Note that Orca/C will call MMShutDown and TLShutDown on exit */
}

#define FIRSTALERT 1 /* First alert defined in resources */

/******************************* Alerts **********************************
   Prototypes and descriptions of defined routines:

       int DoSimpleAlert(long alertID)
           Call an alert from a resource and return the number of button
           pressed (0 for leftmost, 1 for next, 2 for last.
       int DoAlertC(long alertID, char *S0, char *S1, ..., char *S9, NULL)
           Call an alert and pass up to ten C substitution strings.
           The string list must end with a NULL.
       int DoAlertP(long alertID, char *S0, char *S1, ..., char *S9)
           Call an alert and pass up to ten Pascal sub strings.
           The string list must end with a NULL.

               NOTE: The alertID passed to DoAlertC and DoAlertP
                   MUST BE A LONG!!!  Orca C stack fixup is turned off
                   for these routines so that a variable number of
                   arguments may be passed, so you must behave
                   yourself when using these calls!

**************************************************************************/

#define DoSimpleAlert(alertID) (AlertWindow(5, 0L, (long) alertID))

#if 0
#pragma optimize 8
#include <stdarg.h>
int DoAlertC(long alertID, ...)
{
   long str;           /* Pointer to input C strings */
   long MsgArray[10] ; /* Array of pointers to C strings */
   int i;              /* Array index */
   va_list ap;         /* List of elements passed to function */

   i=0;                /* Init index */

   va_start(ap, alertID);      /* Init vararg processing */
   while ( (str = va_arg(ap,long)) && (i<10) ) {  /* List ends w/NULL */
       MsgArray[i++] = str ;  /* Point to string */
   }
   va_end(ap);         /* Got all the strings */

   /* Ref is resource w/C substitution strings */
   return AlertWindow(4, (long) MsgArray, (long) alertID) ;

}

int DoAlertP(long alertID, ...)
{
   long str;           /* Pointer to input P strings */
   long MsgArray[10] ; /* Array of pointers to P strings */
   int i;              /* Array index */
   va_list ap;         /* List of elements passed to function */

   i=0;                /* Init index */

   va_start(ap, alertID);      /* Init vararg processing */
   while ( (str = va_arg(ap,long)) && (i<10) ) {  /* List ends w/NULL */
       MsgArray[i++] = str ;  /* Point to string */
   }
   va_end(ap);         /* Got all the strings */

   /* Ref is resource w/P substitution strings */
   return AlertWindow(5, (long) MsgArray, (long) alertID) ;

}
#endif

/********************************* Text **********************************
   Prototype and description of defined macro:

       Handle GetRTextHandle(long textID)
           Get a handle to a text resource.

**************************************************************************/

#define GetRText(textID) LoadRes(rText, textID)

/**************************************************************************
   The following is generated by Genesys in case you forgot to add a tool
   table as a resource. Once added, delete eveything from #ifndef TOOLSTARTRES   to the #endif.
**************************************************************************/

#ifndef TOOLSTARTRES

typedef struct  MyToolSpec {
    word    toolNumber;
    word    minVersion;
};

typedef struct ToolStartupRec {
    word    flags;
    word    videoMode;
    word    resFileID;
    long    dPageHandle;
    word    numTools;
    struct MyToolSpec toolArray[];
} ;

struct ToolStartupRec TSTART = {
 0x0000,    /* flags */
 0xC080,    /* videoMode */
 0, 0L,    /* resFileID & dPageHandle are set by the StartUpTools call */
 0x0014,    /* numTools */
 1, 0x0300,    /* Tool Locator */
 2, 0x0300,    /* Memory Manager */
 3, 0x0300,    /* Miscellaneous Tools */
 4, 0x0301,    /* QuickDraw II */
 5, 0x0302,    /* Desk Manager */
 6, 0x0300,    /* Event Manager */
 11, 0x0200,    /* Integer Math */
 14, 0x0301,    /* Window Manager */
 15, 0x0301,    /* Menu Manager */
 16, 0x0301,    /* Control Manager */
 18, 0x0301,    /* QuickDraw II Aux. */
 19, 0x0300,    /* Print Manager */
 20, 0x0301,    /* LineEdit Tools */
 21, 0x0301,    /* Dialog Manager */
 22, 0x0300,    /* Scrap Manager */
 23, 0x0301,    /* Standard File Tools */
 27, 0x0301,    /* Font Manager */
 28, 0x0301,    /* List Manager */
 30, 0x0100,    /* Resource Manager */
 34, 0x0101     /* TextEdit Manager */
};

int StartUpMyTools(void)
{
int returncode ;
   startStopAddr = StartUpTools(userid(), 0, (long) &TSTART);
   if (toolerror()) {
      returncode = -1; 
   } else { returncode = 0; }
   return returncode ;
}

void ShutDownMyTools(void)
{
  ShutDownTools(0, startStopAddr);
}

#endif


#if 0
/************************************************************************
* Determines if the current target control is a LineEdit control or not.
* Returns a handle to the LE data if the current target is an editLine
*  control, NULL if not.
************************************************************************/

Handle IsItLineEdit(void)
{
CtlRecHndl LECtlRecHandle ; /* Handle to LE control record */

    LECtlRecHandle = (void *) FindTargetCtl(); /* Get handle of target ctrl*/
    if (toolerror()) return NULL ; /* No control, so return */

    /* if bit 12 of control's MoreFlags is set, then it may be a LE control*/
    if ( GetCtlMoreFlags(LECtlRecHandle) & 0x1000 ) {

         /* Check for Control proc pointer = $83000000 == LE control */
         if ( (long) (*LECtlRecHandle)->ctlProc == editLineControl ) {
              /* Got LE control! */
              return (Handle) (*LECtlRecHandle)->ctlData ;
                         /* So return handle to data */

         }
    }

    /* If we make it here, then we don't have a LE control */
    return NULL ;
}

#endif

/************************************************************************
* Generic error handling routine. Pass the error number.
************************************************************************/

void FatalError(void)
{
    GrafOff();   /* Turn off graphics display */
    puts("A fatal error has occurred. Press any key to exit.");
    getchar();  /* Wait for a keystroke */

    ShutDownMyTools(); /* Close any tools started */

    exit(1); /* Exit with an error */

}

/************************************************************************
* Load and init the tools, handle any errors, create the menu bar.
************************************************************************/

void InitTools(void)
{

   /* Startup all tools from resource. */
   if (StartUpMyTools()) FatalError();
}

/************************************************************************
*
* Perform any application specific initialization.
* All we do is initialize QuitFlag to zero and create our window.
*
************************************************************************/

void InitApp(void)
{
    QuitFlag = 0 ;      /* Zero QuitFlag */
    WindType = 0x8000 ; /* and set WindType for system window */

    /*SetMenuBar(0L);  /* Set system menu bar */
    InitCursor();

}

/************************************************************************
* Close down things. Usually undoes what was done in InitApp. Kill memory.
************************************************************************/

void CloseApp(void)
{
/* We don't close our window since Orca/C does it for us. */
}

/************************************************************************
*
* Routines for Menu selection
*
************************************************************************/

/************************************************************************
*
* Quit routine. Set the QuitFlag to TRUE for the EventLoop.
*
************************************************************************/

void doQuit()

{
    QuitFlag = -1 ; /* Set flag to exit event loop */
}

/************************************************************************
*
* Bring up an Alert Dailog box with our name in it.
*
************************************************************************/

void doAbout()
{
    DoSimpleAlert(4l);
}
/************************************************************************
*
* Called by the MenuSelect routine to handle the
*  "cut" standard menu item.
*
************************************************************************/

void doCut(void)
{
#if 0
Handle leDataHandle ; /* Handle to Line Edit data */

WindowPtr CurrentGrafPortPtr ; /* Current GrafPort pointer */

    /* Check to see if we were in an LE control when the menu was selected */

    leDataHandle = IsItLineEdit() ;
    if (leDataHandle == NULL )  return ; /* Exit if not LE control */

    CurrentGrafPortPtr = GetPort() ;  /* Save current GrafPort pointer */
    StartDrawing(FrontWindow()) ; /* Start drawing in our window */

    LECut(leDataHandle); /* Cut date to LE scrap */

    SetOrigin(0,0);  /* Set port origin to 0,0 */
    SetPort(CurrentGrafPortPtr) ; /* Switch back to original GrafPort */

    ZeroScrap() ; /* Zero out any old desk scrap */

    if ( LEGetScrapLen() != 0 ) /* If there is LE scrap ... */
         LEToScrap() ;          /*  copy it to the desk scrap */

#endif
}

/************************************************************************
*
* Called by the MenuSelect routine to handle the "copy" standard menu item.
*
************************************************************************/

void doCopy(void)
{
#if 0
Handle leDataHandle ; /* Handle to Line Edit data */

WindowPtr CurrentGrafPortPtr ; /* Current GrafPort pointer */

    /* Check to see if we were in an LE control when the menu was selected */

    leDataHandle = IsItLineEdit() ;
    if (leDataHandle == NULL )  return ; /* Exit if not LE control */

    CurrentGrafPortPtr = GetPort() ;  /* Save current GrafPort pointer */
    StartDrawing(FrontWindow()) ; /* Start drawing in our window */

    LECopy(leDataHandle); /* Cut date to LE scrap */

    SetOrigin(0,0);  /* Set port origin to 0,0 */
    SetPort(CurrentGrafPortPtr) ; /* Switch back to original GrafPort */

    ZeroScrap() ; /* Zero out any old desk scrap */

    if ( LEGetScrapLen() != 0 ) /* If there is LE scrap ... */
         LEToScrap() ;          /*  copy it to the desk scrap */

#endif
}

/************************************************************************
*
* Called by the MenuSelect routine to handle the "paste" standard menu item.
*
************************************************************************/

void doPaste()
{
#if 0
Handle leDataHandle ; /* Handle to Line Edit data */

WindowPtr CurrentGrafPortPtr ; /* Current GrafPort pointer */

    /* Check to see if we were in an LE control when the menu was selected */

    leDataHandle = IsItLineEdit() ;
    if (leDataHandle == NULL )  return ; /* Exit if not LE control */

    LEFromScrap() ;  /* Get LE scrap from desk scrap */

    CurrentGrafPortPtr = GetPort() ;  /* Save current GrafPort pointer */
    StartDrawing(FrontWindow()) ; /* Start drawing in our window */

    LEPaste(leDataHandle); /* Paste from LE scrap */

    SetOrigin(0,0);  /* Set port origin to 0,0 */
    SetPort(CurrentGrafPortPtr) ; /* Switch back to original GrafPort */

#endif
}

/************************************************************************
*
* Called by the MenuSelect routine to handle the "clear" standard menu item.
*
************************************************************************/

void doClear(void)
{
#if 0
Handle leDataHandle ; /* Handle to Line Edit data */

WindowPtr CurrentGrafPortPtr ; /* Current GrafPort pointer */

    /* Check to see if we were in an LE control when the menu was selected */

    leDataHandle = IsItLineEdit() ;
    if (leDataHandle == NULL )  return ; /* Exit if not LE control */

    CurrentGrafPortPtr = GetPort() ;  /* Save current GrafPort pointer */
    StartDrawing(FrontWindow()) ; /* Start drawing in our window */

    LEDelete(); /* Delete from LE window */

    SetOrigin(0,0);  /* Set port origin to 0,0 */
    SetPort(CurrentGrafPortPtr) ; /* Switch back to original GrafPort */

#endif
}

/************************************************************************
*
* Main event loop
*
************************************************************************/


/************************************************************************
*
* Main Event Loop.  Handle things until user selects Quit.
*  This represents a typical event loop.  Just place the routine calls you
*  will be using here depending on the action that occurs.
*
************************************************************************/

void doInfoWindow(long winid)
{
EventRecord MyEventRecord ;  /* Setup an event record for my use */
int Event ;  /* Event number returned by TaskMaster */
WindowPtr win = DoNewWindow(winid);
int done = 0;

    MyEventRecord.wmTaskMask = 0x001FFFFFL ;   /* Handle all events possible*/
    do {
         Event = TaskMaster(everyEvent,&MyEventRecord );

         switch (Event) {
              case wInGoAway : /* In window's close box */
                   CloseWindow(win);
                   done = 1;
                   break;
              default: break;
         }
    } while (!done);  /* Keep going until quitflag is set */
}


int greetLoop(void)
{
EventRecord MyEventRecord ;  /* Setup an event record for my use */
int Event ;  /* Event number returned by TaskMaster */
int done = 0, result = 0;
WindowPtr win = DoNewWindow(0xff7l);

    ShowWindow(win);
    MyEventRecord.wmTaskMask = 0x001FFFFFL ;   /* Handle all events possible*/
    do {
         Event = TaskMaster(everyEvent , &MyEventRecord ) ;

         switch (Event) {
              case wInContent:  /* In the content region */
                   break ;

              case wInControl : /* In a control */
                   if (MyEventRecord.wmTaskData4 == 6) {
                   	done = 1;
	                result = 1;
	           }
                   else if (MyEventRecord.wmTaskData4 == 5) {
                   	done = 1;
	                CloseWindow(win);
                   }
                   break ;

              case wInGoAway : /* In window's close box */
                   CloseWindow(win) ;
                   break ;
         }
    } while (!done);  /* Keep going until quitflag is set */
    return result;
}

int inst_orca = 0,inst_orcalib = 0,inst_mu = 0;
char *orca_dir;

void findORCA(void)
{
SFReplyRec2 rr = {0, 0, 0l, 3, 0l, 3, 0l};
FileInfoRecGS fi;
int typeList[] = {1,0x8000,0xB3,0,0};
GSString255Ptr path;
char *pathc;
int st,i;
char *tmpbuf = malloc(256);

    /* save and restore prefix 0 (directory to install GNO in) because
       stdfile changes it */
    getwdn(tmpbuf,0);
    SFGetFile2(100,30,0,"\pLocate ORCA.SYS16",NULL,typeList,&rr);
    setpfx(tmpbuf,0);
    free(tmpbuf);
    if (!rr.good) return;
    /* we now check for deskness/nondeskness in launchApp */
    DisposeHandle(rr.nameRef);
    path = &(((ResultBuf255Ptr) *((handle) rr.pathRef))->bufString);
    pathc = malloc(path->length+2);
    memcpy(pathc, path->text, (size_t)path->length);
    pathc[path->length] = 0;
    for (i = path->length-2; i > 0; i--) {
    	if (pathc[i] == ':') break;
    }
    pathc[i] = 0; /* truncate the filename portion of the path */
    orca_dir = pathc;
}

int doInstall(void)
{
int res = 0;
extern int setpfx(char *pfx, int pf);

    if (greetLoop()) return 0; /* they want to quit, boo hoo! */
    do {
    	inst_orca = DoSimpleAlert(1);
    	switch (inst_orca) {
	    case 0: /* Info */
		doInfoWindow(0xff9l);
		break;
	    case 1: /* No */
	    case 2: /* Yes */
		break;
	}
    } while (inst_orca == 0);
    inst_orca--;

    if (inst_orca) {
        findORCA();
        if (setpfx(orca_dir,3)) {
           GrafOff();
	   printf("Couldn't set prefix 3: %s\n",strerror(errno));
           ReadChar(0);
	   GrafOn();
	}

        do {
    	    inst_orcalib = DoSimpleAlert(3);
    	    switch (inst_orcalib) {
		case 0: /* Info */
		    doInfoWindow(0xff8l);
		    break;
		case 1: /* No */
		case 2: /* Yes */
		    break;
	    }
	} while (inst_orcalib == 0);
    	inst_orcalib--;
    }
    do {
	inst_mu = DoSimpleAlert(2);
	switch (inst_mu) {
	    case 0: /* Info */
	        doInfoWindow(0xffal);
	        break;
	    case 1: /* No */
	    case 2: /* Yes */
	    	break;
        }
    } while (inst_mu == 0);
    inst_mu--;
    return 1;
}

/************************************************************************
*
* This is the main routine. It calls all routines needed to run
*  the application.
*
************************************************************************/

void main(void)
{
char s[2];

    if (!needsgno()) {
        WriteCString("Please run 'Kern' from the hard drive copy of GNO.Disk1 to start\n\r"
               "the installation process\n\r"
               "(press return to exit to your program launcher)\n\r");
        ReadChar(0);
        exit(1);
    }
    InitTools();        /* Initialize tools */
    InitApp();          /* Init Application specific info */
    ShowCursor();       /* Display cursor */
    if (doInstall()) {
    	ShutDownMyTools();
        GrafOff();
        mainInstall();
    }
    else ShutDownMyTools();  /* Shutdown tools */
}
