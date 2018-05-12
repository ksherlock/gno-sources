#pragma optimize -1

#include <Font.h>
#include <TextEdit.h>
#include <gno/gno.h>

extern void initPipe(void);
extern void closePipe(void);
extern void consume(CtlRecHndl);
extern void sendchar(char *);
extern void updateWind(void);

typedef struct myTERuler {
   Word leftMargin;
   Word leftIndent;
   Word rightMargin;
   Word just;
   Word extraLS;
   Word flags;
   LongWord userData;
   Word tabType;
/*   TabItem theTabs[1];
   Word tabTerminator;   */
} myTERuler, *myTERulerPtr, **myTERulerHndl;

typedef struct myTEFormat {
   Word version;
   LongWord rulerListLength;
   myTERuler theRulerList[1];
   LongWord styleListLength;
   TEStyle theStyleList[1];
   LongWord numberOfStyles;
   StyleItem theStyles[1];
} myTEFormat, *myTEFormatPtr, **myTEFormatHndl;


