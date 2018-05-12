#include <stdio.h>
#include "screen.h"
#include "tcap.h"
#include <ctype.h>

em_bell(pane)
struct pane *pane;
{
	if (pane->st & DO_BEEP)
		tc_vb();
}

em_ff(pane, ch)
struct pane *pane;
{
	outs(tcap.tc_cl);
	if (ch)
	{
		pane->pcx = 0;
		pane->pcy = 0;
	}
	else
		em_sync(pane);
}

em_up(pane)
struct pane *pane;
{
	if (pane->pcy > 0)
	{
		outs(tcap.tc_up);
		pane->pcy--;
	}
}

em_back(pane)
struct pane *pane;
{
	if (pane->pcx > 0)
	{
		tc_bs();
		pane->pcx--;
	}
}

em_dn(pane)
struct pane *pane;
{
	if (pane->pcy < tcap.tc_li && pane->pcy != pane->sl1)
	{
		tc_lf();
		pane->pcy++;
	}
}

em_fwd(pane)
struct pane *pane;
{
	if (pane->pcx < tcap.tc_co)
	{
		outs(tcap.tc_nd);
		pane->pcx++;
	}
}

em_bksp(pane)
struct pane *pane;
{
	if (pane->pcx > 0)
	{
		tc_bs();
		pane->pcx--;
	}
	else if (pane->st & DO_WRAP)
	{
		if (pane->pcy > 0)
			pane->pcy--;
		pane->pcx = tcap.tc_co - 1;
		em_sync(pane);
	}
}

em_ndsp(pane)
struct pane *pane;
{
	if (pane->pcx >= tcap.tc_co - 1)
	{
		if (!(pane->st & NOWRAP))
		{
			em_lf(pane);
			em_cr(pane);
		}
	}
	else
	{
		outs(tcap.tc_nd);
		pane->pcx++;
	}
}

                  /* `abcdefghijklmnopqrstuvwxyz{|}~ */
char graphtab[]   = "*@????'+??+++++-----++++|<>\"=$.";
init_graphtab()
{
	char *s;
	if(tcap.tc_ac == 0)
		return;
	for(s = graphtab; *s; s++)
		*s = ' ';
	for(s = tcap.tc_ac; s[0] && s[1]; s+=2) {
		if(s[0] >= '`' && s[0] <= '~')
			graphtab[s[0]-'`'] = s[1];
	}
}

em_outc(pane, c)
struct pane *pane;
char    c;
{
	if( (pane->lastmode&GRAPHIC) != (pane->st&GRAPHIC) && tcap.tc_ac) {
		if(pane->st&GRAPHIC)
			outs(tcap.tc_as);
		else
			outs(tcap.tc_ae);
		pane->lastmode &= ~GRAPHIC;
		pane->lastmode |= pane->st&GRAPHIC;
	}
	if((pane->st&GRAPHIC)) {
		if(c >='`' && c <= '~') {
			c = graphtab[c-'`'];
		} else {
			pane->lastmode &= ~GRAPHIC;
			outs(tcap.tc_ae);
		}
	}
	/* putchar(c);*/
        /*putc(c,stdscr);*/
	bufr[numchars++] = c;

        if (pane->pcx >= tcap.tc_co - 1)
	{
		if (!(pane->st & NOWRAP))
		{
/* The following case is a kludge to handle a weird bit of behaviour in
   the DEC vt100, where a character printed in the last column is displayed
   but the cursor is not physically advanced until the *next* character
   is displayed, so you can write the last character in a scrolling region
   without scrolling it. This is not a general solution, and in fact does
   not work properly if that following character is written, but I don't
   want to bother dealing with implied scrolls and the like. I hope this
   will work well enough to satisfy EDT. If not, I'll have to set an implied
   scroll flag here and clear it on any cursor motion, so that the NEXT time
   I output a character it will do the em_lf first. Growl.

   Anyone else want to target Maynard, Mass with a SCUD?
*/
			if (
			    (
			     pane->emulation == ANSI ||
			     pane->emulation == VT52
			    ) &&
			    pane->sl1 == pane->pcy &&
			    pane->sl1 != tcap.tc_li
			   )
			{
				pane->pcx = 0;
				em_sync(pane);
			}
			else if (tcap.tc_am == 0)
			{
				em_lf(pane);
				em_cr(pane);
			}
			else
			{
				pane->pcx = 0;
				if (pane->pcy != pane->sl1)
					pane->pcy++;
				else if (pane->sl1 != tcap.tc_li - 1)
					em_scroll(pane, 1);
			}
		}
		else if (tcap.tc_am != 0)
			em_sync(pane);
	}
	else
	{
		pane->pcx++;
	}
}

em_tab(pane)
struct pane *pane;
{
	int     cnt;

	cnt = 8 - (pane->pcx % 8);
	while (cnt--)
		em_ndsp(pane);
}

em_rtab(pane)
struct pane *pane;
{
	int     cnt;

	cnt = (pane->pcx % 8);
	if (cnt == 0 && pane->pcx > 0)
		cnt = 8;
	while (cnt--)
		em_back(pane);
}

em_revlf(pane)
struct pane *pane;
{
	if (pane->pcy == pane->sl0)
	{
		if (!(pane->st & NOSCROLL))
			em_scroll(pane, -1);
	}
	else if (pane->pcy > 0)
		em_up(pane);
}

em_lf(pane)
struct pane *pane;
{
	if (pane->pcy == pane->sl1)
	{
		if (!(pane->st & NOSCROLL))
			em_scroll(pane, 1);
	}
	else if (pane->pcy < tcap.tc_li - 1)
		em_dn(pane);
}

em_home(pane)
struct pane *pane;
{
	pane->pcx = 0;

	if(pane->st & DECOM) {
		pane->pcy = pane->sl0;
		if(pane->sl0 != 0) {
			tc_cm(0, pane->sl0);
			return;
		}
	}
	else
		pane->pcy = 0;

	tc_ho();
}

em_cr(pane)
struct pane *pane;
{
	pane->pcx = 0;
	outc('\r');
}

em_sync(pane)
struct pane *pane;
{
	tc_cm(pane->pcx, pane->pcy);
}

em_pgoto(pane, x, y)
struct pane *pane;
{
	if (pane->pcx != x && pane->pcy != y)
	{
		pane->pcx = x;
		pane->pcy = y;
		em_sync(pane);
	}
}

em_cline(pane)
struct pane *pane;
{
	int i;
	outc('\r');
	outs(tcap.tc_ce);
	em_sync(pane);
}

em_cbol(pane)
struct pane *pane;
{
	int i;
	outc('\r');
	for(i = 0; i < pane->pcx; i++)
		outc(' ');
}

em_cbop(pane)
struct pane *pane;
{
	int i;
	if(pane->pcy > 0) {
		tc_ho();
		for(i = 0; i < pane->pcy; i++) {
			outs(tcap.tc_ce);
			tc_lf();
		}
		em_cbol(pane);
	}
}

em_ceol(pane)
{
	outs(tcap.tc_ce);
}

em_ceop(pane)
{
	outs(tcap.tc_cd);
}

em_insdel(pane, chars)
{
	char *s = tcap.tc_dc;
	if(chars < 0)
		chars = -chars;
	else
		s = tcap.tc_ic;
	while(chars-->0)
		outs(s);
}

em_inslines(pane, lines)
{
	char *s = tcap.tc_dl;
	if(lines < 0)
		lines = -lines;
	else
		s = tcap.tc_al;
	while(lines-->0)
		outs(s);
}

em_mode(pane, flag, on)
struct pane *pane;
int flag;
int on;
{
	int old = pane->st&flag;
	if(on) pane->st |= flag;
	else pane->st &= ~flag;
	if(old != pane->st) {
		switch(flag) {
			case DECOM: break;
			case KEYPAD:
				if(pane->st&flag)
					outs(tcap.tc_ks);
				else
					outs(tcap.tc_ke);
				break;
			case AUTO_CR: break;
			case AUTO_LF: break;
			case INVMODE:
				if(pane->st&flag)
					outs(tcap.tc_so);
				else
					outs(tcap.tc_se);
				break;
			case INVIDEO: break;
			case INSMODE:
				if(pane->st&flag)
					outs(tcap.tc_is);
				else
					outs(tcap.tc_ei);
				break;
			case DO_WRAP: break;
			case NOERROR: break;
			case NOGRAPH: break;
			case NOWRAP: break;
			case NOSCROLL: break;
			case HIDDEN: break;
			case GRAPHIC: break;
		}
	}
}

#ifndef XVT
em_respond(p, s, a1, a2, a3)
struct pane *p;
char *s, *a1, *a2, *a3;
{
	char buffer[128];
	int i;
	sprintf(buffer, s, a1, a2, a3);

	tc_cm(0, tcap.tc_li-1);
	for(i = 0; buffer[i]; i++) {
		if(buffer[i] < ' ') {
			outc('^');
			outc(buffer[i]+'@');
		} else
			outc(buffer[i]);
	}
	em_sync(p);
}
#endif

em_set_scroll(p, l0, l1)
struct pane *p;
int l0, l1;
{
	if(l0<1) l0=1;
	if(l0>tcap.tc_li) l0 = tcap.tc_li;
	if(l1<1) l1=1;
	if(l1>tcap.tc_li) l0 = tcap.tc_li;
	if(l1<l0) l1=l0;
	p->sl0 = l0-1;
	p->sl1 = l1-1;
}

em_scroll(pane, delta)
struct pane *pane;
int delta;
{
	char *s;
	int moved;

	moved = 0;
	if(pane->sl0 > 0 || pane->sl1 < tcap.tc_li-1) {
		if(tcap.tc_cs == 0 ||
		   (delta>0 && tcap.tc_sf==0) ||
		   (delta<0 && tcap.tc_sr==0))
			return em_sim_scroll(pane, delta);
	}
	if(delta>0) {
		if(pane->pcy != pane->sl1) {
			tc_ll();
			moved = 1;
		}
		if(tcap.tc_sf)
			s = tcap.tc_sf;
		else
			s = "\n";
	}
	else {
		if(tcap.tc_sr)
			s = tcap.tc_sr;
		else
			return em_sim_scroll(pane, delta);
		delta = -delta;
		if(pane->pcy != pane->sl0) {
			tc_ho();
			moved = 1;
		}
	}
	while(delta>0) {
		outs(s);
		delta--;
	}
	if(moved)
		em_sync(pane);
}

em_sim_scroll(pane, delta)
struct pane *pane;
int delta;
{
	int iline;	/* Line to insert lines at */
	int dline;	/* Line to delete lines at */
	int i;

	if(delta < 0) {
		iline = pane->sl0;
		dline = pane->sl1;
		delta = -delta;
	} else {
		iline = pane->sl1;
		dline = pane->sl0;
	}
	tc_cm(0, dline);
	for(i = 0; i < delta; i++)
		outs(tcap.tc_dl);
	tc_cm(0, iline);
	for(i = 0; i < delta; i++)
		outs(tcap.tc_al);
	em_sync(pane);
}
