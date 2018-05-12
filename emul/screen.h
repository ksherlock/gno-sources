struct pane {
	int pcx, pcy, st;
	int (*state)();
	char emulation;
	char paramptr, param[10];
	int sl0, sl1;
	int savx, savy;
	int lastmode;
};

struct pane *newpane();
extern int numchars;
extern char bufr[1024];
void doflush(void);

#define EMULATIONS 2
#define ANSI 0
#define VT52 1

#define ALLBITS	0177777
#define GRAPHIC 0100000
#define DECOM	 040000
#define KEYPAD	 020000
#define AUTO_CR	 010000
#define AUTO_LF	  04000
#define INVMODE	  02000
#define INVIDEO	  01000
#define INSMODE	   0400
#define DO_WRAP	   0200
#define NOERROR	   0100
#define NOGRAPH	    040
#define NOWRAP	    020
#define NOSCROLL    010
#define HIDDEN	     04
#define DO_BEEP	     02
#define DO_CTRL	     01
