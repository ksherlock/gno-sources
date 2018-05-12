#include <stdio.h>
#include "screen.h"
#include "tcap.h"
#include <ctype.h>

getx52(pane, c)
struct pane *pane;
char    c;
{
	char    col = c - ' ';

	if (col < 0)
		col = 0;
	if (col >= tcap.tc_co)
		col = tcap.tc_co - 1;
	pane->pcx = col;
	pane->state = 0;
	em_sync(pane);
}

gety52(pane, c)
struct pane *pane;
char    c;
{
	char    line = c - ' ';

	if (line < 0)
		line = 0;
	if (line >= tcap.tc_li)
		line = tcap.tc_li - 1;
	pane->pcy = line;
	em_sync(pane);
	pane->state = getx52;
}

req52(pane, c)
struct pane *pane;
char    c;
{
	pane->state = 0;
	if (c == '0')
		em_respond(pane, "\033iB0");
}

mode52(pane, c)
struct pane *pane;
char    c;
{
	pane->state = 0;
	switch (c)
	{
	    case '8':
		if (pane->param[0] == 'x')
			pane->st |= AUTO_LF;
		else
			pane->st &= ~AUTO_LF;
		break;
	    case '9':
		if (pane->param[0] == 'x')
			pane->st |= AUTO_CR;
		else
			pane->st &= ~AUTO_CR;
		break;
	    default:
		break;
	}
}

cmd52(pane, c)
struct pane *pane;
char    c;
{
	pane->state = 0;
	switch (c)
	{
	    case 'Y':
		pane->state = gety52;
		break;
	    case 'A':
		em_up(pane);
		break;
	    case 'B':
		em_dn(pane);
		break;
	    case 'C':
		em_fwd(pane);
		break;
	    case 'D':
		em_bksp(pane);
		break;
	    case 'H':
		em_home(pane);
		break;
	    case 'I':
		em_revlf(pane);
		break;
	    case 'j':
		pane->savx = pane->pcx;
		pane->savy = pane->pcy;
		break;
	    case 'k':
		em_pgoto(pane, pane->savx, pane->savy);
		break;
	    case 'n':
		em_respond(pane, "\033Y%c%c", pane->pcy + ' ', pane->pcy + ' ');
		break;
	    case '-':
		em_rtab(pane);
		break;
	    case 0x18:
		break;
	    case 'E':
		em_ff(pane, 1);
		break;
	    case 'b':
		em_cbop(pane);
		break;
	    case 'J':
		em_ceop(pane);
		break;
	    case 'l':
		em_ff(pane, 0);
	    case 'o':
		em_cbol(pane);
		break;
	    case 'K':
		em_ceol(pane);
		break;
	    case 'L':
		em_inslines(pane, 1);
		break;
	    case 'M':
		em_inslines(pane, -1);
		break;
	    case 'N':
		em_insdel(pane, -1);
		break;
	    case '@':
		em_mode(pane, INSMODE, 1);
		break;
	    case 'O':
		em_mode(pane, INSMODE, 0);
		break;
	    case '<':
		pane->emulation = ANSI;
		break;
	    case '=':
		em_mode(pane, KEYPAD, 1);
		break;
	    case '>':
		em_mode(pane, KEYPAD, 0);
		break;
	    case 'F':
	    case 'G':
	    case '.':
		break;
	    case 'p':
		em_mode(pane, INVMODE, 1);
		break;
	    case 'q':
		em_mode(pane, INVMODE, 0);
		break;
	    case 'v':
		em_mode(pane, NOWRAP, 0);
		break;
	    case 'w':
		em_mode(pane, NOWRAP, 1);
		break;
	    case 'i':
		pane->state = req52;
		break;
	    case 'Z':
		em_respond(pane, "\033/K");
		break;
	    default:
		break;
	}
}

discardc(pane, c)
struct pane *pane;
char    c;
{
	pane->state = 0;
}

/* ONLY emulate G0, G1. G1 ALWAYS DEC Special Graphic. G0 either Special
 * Graphic or ASCII.
 */
graphansi(pane, c)
struct pane *pane;
char    c;
{
	pane->state = 0;
	switch(pane->param[0]) {
		case '(':
			switch(c) {
				case 'B': em_mode(pane, GRAPHIC, 0); break;
				case '0': em_mode(pane, GRAPHIC, 1); break;
				default: break;
			}
			break;
	}
}

paramansi(pane, c)
struct pane *pane;
char    c;
{
	int     tmp;

	switch (c)
	{
	    case ';':
		if (!pane->paramptr)
			pane->paramptr++;
		pane->paramptr++;
		if (pane->paramptr >= 10)
		{
			if (!(pane->st & NOERROR))
				em_outc(pane, 127);
			pane->state = 0;
		}
		else
		{
			pane->param[pane->paramptr] = 0;
		}
		break;
	    case 0x18:
		pane->state = 0;
		return;
	    case '?':
	    case '>':
		if (pane->paramptr == 0 && pane->param[0] == 0)
		{
			pane->param[0] = c;
			pane->paramptr = 1;
		}
		else
		{
			if (!(pane->st & NOERROR))
				em_outc(pane, 127);
		}
		break;
	    case '0':
	    case '1':
	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
	    case '7':
	    case '8':
	    case '9':
		if (!pane->paramptr)
		{
			pane->paramptr++;
			pane->param[pane->paramptr] = 0;
		}
		pane->param[pane->paramptr] *= 10;
		pane->param[pane->paramptr] += c - '0';
		break;
	    default:
		switch (c)
		{
		    case 'H':
		    case 'f':
			if (pane->paramptr)
			{
				if(pane->paramptr > 1)
					pane->pcx = pane->param[2] - 1;
				else
					pane->pcx = 0;

				if (pane->pcx > tcap.tc_co-1)
					pane->pcx = tcap.tc_co-1;
				if (pane->pcx < 0)
					pane->pcx = 0;

				pane->pcy = pane->param[1] - 1;
				if(pane->st & DECOM)
					pane->pcy += pane->sl0;

				if (pane->pcy > tcap.tc_li-1)
					pane->pcy = tcap.tc_li-1;
				if (pane->pcy < 0)
					pane->pcy = 0;
				em_sync(pane);
			}
			else
			{
				em_home(pane);
			}
			break;
		    case 'J':
			if (pane->param[1] == 2)
				em_ff(pane, 0);
			else if (pane->param[1] == 1)
				em_cbop(pane);
			else
				em_ceop(pane);
			break;
		    case 'L':
			em_inslines(pane, 1);
			if (pane->param[1] > 1)
				while (--(pane->param[1]) > 0)
					em_inslines(pane, 1);
			break;
		    case 'M':
			em_inslines(pane, -1);
			if (pane->param[1] > 1)
				while (--(pane->param[1]) > 0)
					em_inslines(pane, -1);
			break;
		    case 'P':
			if (pane->param[1])
				em_insdel(pane, -pane->param[1]);
			else
				em_insdel(pane, -1);
			break;
		    case 'A':
			em_up(pane);
			if (pane->param[1] > 1)
				while (--(pane->param[1]) > 0)
					em_up(pane);
			break;
		    case 'B':
			em_dn(pane);
			if (pane->param[1] > 1)
				while (--(pane->param[1]) > 0)
					em_dn(pane);
			break;
		    case 'D':
			em_back(pane);
			if (pane->param[1] > 1)
				while (--(pane->param[1]) > 0)
					em_back(pane);
			break;
		    case 'C':
			em_fwd(pane);
			if (pane->param[1] > 1)
				while (--(pane->param[1]) > 0)
					em_fwd(pane);
			break;
		    case 'K':
			if (pane->param[1] == 2)
				em_cline(pane);
			else if (pane->param[1] == 1)
				em_cbol(pane);
			else
				em_ceol(pane);
			break;
		    case 'c':
			if (pane->param[0] == '?')
				break;
			em_respond(pane, "\033[?1;2c");
			break;
		    case 'h':
		    case 'l':
			switch (pane->param[0])
			{
			    case 0:
				{
					int     i;

					for (i = 1; i <= pane->paramptr; i++)
					{
						switch (pane->param[i])
						{
						    case 4:
							em_mode(pane, INSMODE, c == 'h');
							break;
						    default:
							break;
						}
					}
				}
				break;
			    case '?':
				{
					int     i;

					for (i = 1; i <= pane->paramptr; i++)
					{
						switch (pane->param[i])
						{
						    case 2:
							pane->emulation = VT52;
							break;
						    case 6:
							if (c == 'h')
								em_mode(pane, DECOM, 1);
							else
								em_mode(pane, DECOM, 0);
							break;
						    case 7:
							if (c == 'h')
								em_mode(pane, NOWRAP, 0);
							else
								em_mode(pane, NOWRAP, 1);
							break;
						    case 20:
							if (c == 'h')
								em_mode(pane, AUTO_CR, 1);
							else
								em_mode(pane, AUTO_CR, 0);
							break;
						    default:
							break;
						}
					}
				}
				break;
			    case '>':
				{
					int     i;

					for (i = 1; i <= pane->paramptr; i++)
					{
						switch (pane->param[i])
						{
						    case 8:
							if (c == 'h')
								em_mode(pane, AUTO_LF, 1);
							else
								em_mode(pane, AUTO_LF, 0);
							break;
						    case 9:
							if (c == 'h')
								em_mode(pane, AUTO_CR, 1);
							else
								em_mode(pane, AUTO_CR, 0);
							break;
						    default:
							break;
						}
					}
				}
			    default:
				break;
			}
			break;
		    case 'n':
			pane->state = 0;
			if (pane->param[1] == 6)
			{
				em_respond(pane, "\033[%d;%dR",
					   pane->pcy + 1,
					   pane->pcx + 1);
			}
			else if (pane->param[1] == 5)
			{
				em_respond(pane, "\0330n");
			}
			else if (!(pane->st & NOERROR))
				em_outc(pane, 127);
			break;
		    case 'm':
			{
				int     i;

				if (pane->paramptr)
				{
					for (i = 1; i <= pane->paramptr; i++)
						if (pane->param[i] == 7)
							em_mode(pane, INVMODE, 1);
						else if (!pane->param[i])
							em_mode(pane, INVMODE, 0);
				}
				else
					em_mode(pane, INVMODE, 0);
			}
			break;
		    case 'r':
			if (pane->paramptr >= 2
			 && pane->param[2] != 0)
			{
				em_set_scroll(pane, pane->param[1], pane->param[2]);
				/* ANSI bug: home when set-scroll */
				em_home(pane);
			}
			else {
				em_set_scroll(pane, 1, 24);
				em_home(pane);
			}
			break;
		    case 's':
			pane->savx = pane->pcx;
			pane->savy = pane->pcy;
			break;
		    case 'u':
			em_pgoto(pane, pane->savx, pane->savy);
			break;
		    case '@':
			if (pane->param[1])
				em_insdel(pane, pane->param[1]);
			else
				em_insdel(pane, 1);
			break;
		    default:
			if (!(pane->st & NOERROR))
				em_outc(pane, 127);
		}
		pane->state = 0;
		break;
	}
}

leadinansi(pane, c)
struct pane *pane;
char    c;
{
	pane->state = 0;
	switch (c)
	{
	    case 0x18:
		break;
	    case '(':
	    case ')':
		pane->state = graphansi;
		pane->paramptr = 1;
		pane->param[0] = c;
		break;
	    case '[':
		pane->state = paramansi;
		pane->paramptr = 0;
		for (c = 0; c < 10; c++)
			pane->param[c] = 0;
		break;
	    case '7':
		pane->savx = pane->pcx;
		pane->savy = pane->pcy;
		break;
	    case '8':
		em_pgoto(pane, pane->savx, pane->savy);
		break;
	    case '=':
		em_mode(pane, KEYPAD, 1);
		break;
	    case '>':
		em_mode(pane, KEYPAD, 0);
		break;
	    case 'D':
		em_dn(pane);
		break;
	    case 'M':
		em_revlf(pane);
		break;
	    case 'E':
		em_lf(pane);
		break;
	    default:
		if (!(pane->st & NOERROR))
			em_outc(pane, 127);
		break;
	}
}

em_write(pane, s, n)
struct pane *pane;
char   *s;
int     n;
{
	if (!pane)
		return;

	while (n--)
	{
		char    c = *s++;

		if (!c)
			continue;

		if (c & 128 && pane->st & NOGRAPH)
			c &= 127;
		if (pane->state)
			(*pane->state) (pane, c);
		else
		{
			switch (pane->emulation)
			{
			    case ANSI:
				if (c >= ' ' && c <= '~')
					em_outc(pane, c);
				else
					switch (c)
					{
					    case 007:
						em_bell(pane);
						break;
					    case '\b':
						em_bksp(pane);
						break;
					    case '\t':
						em_tab(pane);
						break;
					    case '\f':
					    case 'K' - '@':
					    case '\n':
						if (pane->st & AUTO_CR)
							em_cr(pane);
						em_lf(pane);
						break;
					    case '\r':
						if (pane->st & AUTO_LF)
							em_lf(pane);
						em_cr(pane);
						break;
					    case 'N'-'@':
						em_mode(pane, GRAPHIC, 1);
						break;
					    case 'O'-'@':
						em_mode(pane, GRAPHIC, 0);
						break;
					    case '\033':
						pane->state = leadinansi;
						break;
					    default:
						if (pane->st & DO_CTRL)
							em_outc(pane, c);
					}
				break;
			    case VT52:
				if (c >= ' ' && c <= '~')
					em_outc(pane, c);
				else
					switch (c)
					{
					    case 007:
						em_bell(pane);
						break;
					    case '\b':
						em_bksp(pane);
						break;
					    case '\t':
						em_tab(pane);
						break;
					    case '\f':
					    case 'K' - '@':
					    case '\n':
						if (pane->st & AUTO_CR)
							em_cr(pane);
						em_lf(pane);
						break;
					    case '\r':
						if (pane->st & AUTO_LF)
							em_lf(pane);
						em_cr(pane);
						break;
					    case '\033':
						pane->state = cmd52;
						break;
					    default:
						if (pane->st & DO_CTRL)
							em_outc(pane, c);
					}
				break;
			    default:
				pane->emulation = VT52;
				printf("\rUnimplemented emulator, reverting to VT52\r");
				return;
			}
		}
	}
}

em_init(pane)
struct pane *pane;
{
	tc_is();
	pane->st = 0;
	pane->state = NULL;
	pane->emulation = ANSI;
	pane->paramptr = 0;
	pane->sl0 = 0;
        pane->sl1 = tcap.tc_li - 1;
	if(pane->sl1 > 23) pane->sl1 = 23;
	pane->savx = pane->pcx = 0;
	pane->savy = pane->pcy = 0; /* pane->sl1; */
	pane->lastmode = 0;
	em_sync(pane);
	fflush(stdout);
}
