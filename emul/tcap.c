#include <stdio.h>
#include "tcap.h"
#include "screen.h"
#include <sys/types.h>
#ifndef __ORCAC__
#include <termio.h>
#else
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termcap.h>
#endif
#include <sys/ioctl.h>

struct tcapfoo tcap;

#ifndef __ORCAC__
char *tgetstr();
char *getenv();

char PC;
char *BC;
char *UP;
short ospeed;
#endif

tcap_init()
{
	static char tbuf[1024];
	static char sbuf[1024];
#ifdef TCGETA
        struct termio tb;
#else
	struct sgttyb tb;
#endif
        char *bufp;

	tgetent(tbuf, getenv("TERM"));

	bufp = sbuf;

	tcap.tc_AL = tgetstr("AL", &bufp);
	tcap.tc_BS = tgetstr("BS", &bufp);
	tcap.tc_CC = tgetstr("CC", &bufp);
	tcap.tc_CF = tgetstr("CF", &bufp);
	tcap.tc_CL = tgetstr("CL", &bufp);
	tcap.tc_CO = tgetstr("CO", &bufp);
	tcap.tc_CR = tgetstr("CR", &bufp);
	tcap.tc_DC = tgetstr("DC", &bufp);
	tcap.tc_DL = tgetstr("DL", &bufp);
	tcap.tc_DO = tgetstr("DO", &bufp);
	tcap.tc_G1 = tgetstr("G1", &bufp);
	tcap.tc_G2 = tgetstr("G2", &bufp);
	tcap.tc_G3 = tgetstr("G3", &bufp);
	tcap.tc_G4 = tgetstr("G4", &bufp);
	tcap.tc_GC = tgetstr("GC", &bufp);
	tcap.tc_GD = tgetstr("GD", &bufp);
	tcap.tc_GE = tgetstr("GE", &bufp);
	tcap.tc_GG = tgetnum("GG");
	tcap.tc_GH = tgetstr("GH", &bufp);
	tcap.tc_GL = tgetstr("GL", &bufp);
	tcap.tc_GR = tgetstr("GR", &bufp);
	tcap.tc_GS = tgetstr("GS", &bufp);
	tcap.tc_GU = tgetstr("GU", &bufp);
	tcap.tc_GV = tgetstr("GV", &bufp);
	tcap.tc_HP = tgetstr("HP", &bufp);
	tcap.tc_IC = tgetstr("IC", &bufp);
	tcap.tc_K0 = tgetstr("K0", &bufp);
	tcap.tc_K1 = tgetstr("K1", &bufp);
	tcap.tc_K2 = tgetstr("K2", &bufp);
	tcap.tc_K3 = tgetstr("K3", &bufp);
	tcap.tc_K4 = tgetstr("K4", &bufp);
	tcap.tc_K5 = tgetstr("K5", &bufp);
	tcap.tc_K6 = tgetstr("K6", &bufp);
	tcap.tc_K7 = tgetstr("K7", &bufp);
	tcap.tc_K8 = tgetstr("K8", &bufp);
	tcap.tc_K9 = tgetstr("K9", &bufp);
	tcap.tc_LE = tgetstr("LE", &bufp);
	tcap.tc_M0 = tgetstr("M0", &bufp);
	tcap.tc_M1 = tgetstr("M1", &bufp);
	tcap.tc_M2 = tgetstr("M2", &bufp);
	tcap.tc_M3 = tgetstr("M3", &bufp);
	tcap.tc_M4 = tgetstr("M4", &bufp);
	tcap.tc_M5 = tgetstr("M5", &bufp);
	tcap.tc_M6 = tgetstr("M6", &bufp);
	tcap.tc_M7 = tgetstr("M7", &bufp);
	tcap.tc_M8 = tgetstr("M8", &bufp);
	tcap.tc_M9 = tgetstr("M9", &bufp);
	tcap.tc_MB = tgetstr("MB", &bufp);
	tcap.tc_MC = tgetstr("MC", &bufp);
	tcap.tc_MD = tgetstr("MD", &bufp);
	tcap.tc_ME = tgetstr("ME", &bufp);
	tcap.tc_MF = tgetstr("MF", &bufp);
	tcap.tc_MG = tgetstr("MG", &bufp);
	tcap.tc_MH = tgetstr("MH", &bufp);
	tcap.tc_Ma = tgetstr("Ma", &bufp);
	tcap.tc_Mb = tgetstr("Mb", &bufp);
	tcap.tc_Mc = tgetstr("Mc", &bufp);
	tcap.tc_Md = tgetstr("Md", &bufp);
	tcap.tc_Me = tgetstr("Me", &bufp);
	tcap.tc_Mf = tgetstr("Mf", &bufp);
	tcap.tc_Mg = tgetstr("Mg", &bufp);
	tcap.tc_Mh = tgetstr("Mh", &bufp);
	tcap.tc_Mi = tgetstr("Mi", &bufp);
	tcap.tc_Mj = tgetstr("Mj", &bufp);
	tcap.tc_Mk = tgetstr("Mk", &bufp);
	tcap.tc_Ml = tgetstr("Ml", &bufp);
	tcap.tc_Mm = tgetstr("Mm", &bufp);
	tcap.tc_Mn = tgetstr("Mn", &bufp);
	tcap.tc_Mo = tgetstr("Mo", &bufp);
	tcap.tc_Mt = tgetstr("Mt", &bufp);
	tcap.tc_Mu = tgetstr("Mu", &bufp);
	tcap.tc_Mv = tgetstr("Mv", &bufp);
	tcap.tc_Mw = tgetstr("Mw", &bufp);
	tcap.tc_Mx = tgetstr("Mx", &bufp);
	tcap.tc_My = tgetstr("My", &bufp);
	tcap.tc_Mz = tgetstr("Mz", &bufp);
	tcap.tc_PD = tgetstr("PD", &bufp);
	tcap.tc_PL = tgetstr("PL", &bufp);
	tcap.tc_PR = tgetstr("PR", &bufp);
	tcap.tc_PU = tgetstr("PU", &bufp);
	tcap.tc_RI = tgetstr("RI", &bufp);
	tcap.tc_SR = tgetstr("SR", &bufp);
	tcap.tc_UP = tgetstr("UP", &bufp);
	tcap.tc_WL = tgetstr("WL", &bufp);
	tcap.tc_WR = tgetstr("WR", &bufp);
	tcap.tc_ac = tgetstr("ac", &bufp);
	tcap.tc_ae = tgetstr("ae", &bufp);
	tcap.tc_al = tgetstr("al", &bufp);
	tcap.tc_am = tgetflag("am");
	tcap.tc_as = tgetstr("as", &bufp);
	tcap.tc_bc = tgetstr("bc", &bufp);
	tcap.tc_bs = tgetflag("bs");
	tcap.tc_bt = tgetstr("bt", &bufp);
	tcap.tc_bw = tgetflag("bw");
	tcap.tc_cd = tgetstr("cd", &bufp);
	tcap.tc_ce = tgetstr("ce", &bufp);
	tcap.tc_ch = tgetstr("ch", &bufp);
	tcap.tc_cl = tgetstr("cl", &bufp);
	tcap.tc_cm = tgetstr("cm", &bufp);
	tcap.tc_co = tgetnum("co");
	tcap.tc_cr = tgetstr("cr", &bufp);
	tcap.tc_cs = tgetstr("cs", &bufp);
	tcap.tc_ct = tgetstr("ct", &bufp);
	tcap.tc_cv = tgetstr("cv", &bufp);
	tcap.tc_dC = tgetnum("dC");
	tcap.tc_dF = tgetnum("dF");
	tcap.tc_dN = tgetnum("dN");
	tcap.tc_da = tgetflag("da");
	tcap.tc_db = tgetflag("db");
	tcap.tc_dc = tgetstr("dc", &bufp);
	tcap.tc_de = tgetnum("de");
	tcap.tc_dl = tgetstr("dl", &bufp);
	tcap.tc_dm = tgetstr("dm", &bufp);
	tcap.tc_dn = tgetstr("dn", &bufp);
	tcap.tc_do = tgetstr("do", &bufp);
	tcap.tc_ed = tgetstr("ed", &bufp);
	tcap.tc_ef = tgetstr("ef", &bufp);
	tcap.tc_ei = tgetstr("ei", &bufp);
	tcap.tc_eo = tgetflag("eo");
	tcap.tc_f1 = tgetstr("f1", &bufp);
	tcap.tc_f2 = tgetstr("f2", &bufp);
	tcap.tc_f3 = tgetstr("f3", &bufp);
	tcap.tc_ff = tgetstr("ff", &bufp);
	tcap.tc_hc = tgetflag("hc");
	tcap.tc_hd = tgetstr("hd", &bufp);
	tcap.tc_ho = tgetstr("ho", &bufp);
	tcap.tc_hu = tgetstr("hu", &bufp);
	tcap.tc_hz = tgetflag("hz");
	tcap.tc_ic = tgetstr("ic", &bufp);
	tcap.tc_if = tgetstr("if", &bufp);
	tcap.tc_im = tgetstr("im", &bufp);
	tcap.tc_in = tgetflag("in");
	tcap.tc_ip = tgetstr("ip", &bufp);
	tcap.tc_is = tgetstr("is", &bufp);
	tcap.tc_k0 = tgetstr("k0", &bufp);
	tcap.tc_k1 = tgetstr("k1", &bufp);
	tcap.tc_k2 = tgetstr("k2", &bufp);
	tcap.tc_k3 = tgetstr("k3", &bufp);
	tcap.tc_k4 = tgetstr("k4", &bufp);
	tcap.tc_k5 = tgetstr("k5", &bufp);
	tcap.tc_k6 = tgetstr("k6", &bufp);
	tcap.tc_k7 = tgetstr("k7", &bufp);
	tcap.tc_k8 = tgetstr("k8", &bufp);
	tcap.tc_k9 = tgetstr("k9", &bufp);
	tcap.tc_kA = tgetstr("kA", &bufp);
	tcap.tc_kC = tgetstr("kC", &bufp);
	tcap.tc_kD = tgetstr("kD", &bufp);
	tcap.tc_kE = tgetstr("kE", &bufp);
	tcap.tc_kH = tgetstr("kH", &bufp);
	tcap.tc_kI = tgetstr("kI", &bufp);
	tcap.tc_kL = tgetstr("kL", &bufp);
	tcap.tc_kN = tgetstr("kN", &bufp);
	tcap.tc_kP = tgetstr("kP", &bufp);
	tcap.tc_kb = tgetstr("kb", &bufp);
	tcap.tc_kd = tgetstr("kd", &bufp);
	tcap.tc_ke = tgetstr("ke", &bufp);
	tcap.tc_kh = tgetstr("kh", &bufp);
	tcap.tc_kl = tgetstr("kl", &bufp);
	tcap.tc_kn = tgetnum("kn");
	tcap.tc_ko = tgetstr("ko", &bufp);
	tcap.tc_kr = tgetstr("kr", &bufp);
	tcap.tc_ks = tgetstr("ks", &bufp);
	tcap.tc_ku = tgetstr("ku", &bufp);
	tcap.tc_l0 = tgetstr("l0", &bufp);
	tcap.tc_l1 = tgetstr("l1", &bufp);
	tcap.tc_l2 = tgetstr("l2", &bufp);
	tcap.tc_l3 = tgetstr("l3", &bufp);
	tcap.tc_l4 = tgetstr("l4", &bufp);
	tcap.tc_l5 = tgetstr("l5", &bufp);
	tcap.tc_l6 = tgetstr("l6", &bufp);
	tcap.tc_l7 = tgetstr("l7", &bufp);
	tcap.tc_l8 = tgetstr("l8", &bufp);
	tcap.tc_li = tgetnum("li");
	tcap.tc_ll = tgetstr("ll", &bufp);
	tcap.tc_ma = tgetstr("ma", &bufp);
	tcap.tc_md = tgetstr("md", &bufp);
	tcap.tc_me = tgetstr("me", &bufp);
	tcap.tc_mi = tgetflag("mi");
	tcap.tc_ml = tgetstr("ml", &bufp);
	tcap.tc_mr = tgetstr("mr", &bufp);
	tcap.tc_ms = tgetflag("ms");
	tcap.tc_mu = tgetstr("mu", &bufp);
	tcap.tc_nc = tgetflag("nc");
	tcap.tc_nd = tgetstr("nd", &bufp);
	tcap.tc_nl = tgetstr("nl", &bufp);
	tcap.tc_ns = tgetflag("ns");
	tcap.tc_os = tgetflag("os");
	tcap.tc_pc = tgetstr("pc", &bufp);
	tcap.tc_pl = tgetflag("pl");
	tcap.tc_pt = tgetflag("pt");
	tcap.tc_se = tgetstr("se", &bufp);
	tcap.tc_sf = tgetstr("sf", &bufp);
	tcap.tc_sg = tgetnum("sg");
	tcap.tc_so = tgetstr("so", &bufp);
	tcap.tc_sr = tgetstr("sr", &bufp);
	tcap.tc_st = tgetstr("st", &bufp);
	tcap.tc_ta = tgetstr("ta", &bufp);
	tcap.tc_tc = tgetstr("tc", &bufp);
	tcap.tc_te = tgetstr("te", &bufp);
	tcap.tc_ti = tgetstr("ti", &bufp);
	tcap.tc_uc = tgetstr("uc", &bufp);
	tcap.tc_ue = tgetstr("ue", &bufp);
	tcap.tc_ug = tgetnum("ug");
	tcap.tc_ul = tgetflag("ul");
	tcap.tc_up = tgetstr("up", &bufp);
	tcap.tc_us = tgetstr("us", &bufp);
	tcap.tc_vb = tgetstr("vb", &bufp);
	tcap.tc_ve = tgetstr("ve", &bufp);
	tcap.tc_vs = tgetstr("vs", &bufp);
	tcap.tc_xb = tgetflag("xb");
	tcap.tc_xn = tgetflag("xn");
	tcap.tc_xo = tgetflag("xo");
	tcap.tc_xr = tgetflag("xr");
	tcap.tc_xs = tgetflag("xs");
	tcap.tc_xt = tgetflag("xt");

	PC = tcap.tc_pc ? tcap.tc_pc[0] : 0;
	BC = tcap.tc_bc;
	UP = tcap.tc_up;

#ifdef TCGETA
	ioctl(STDERR_FILENO, TCGETA, &tb);
	ospeed = tb.c_cflag & CBAUD;
#else
	ioctl(STDERR_FILENO, TIOCGETP, &tb);
        ospeed = tb.sg_ispeed;
#endif
	init_graphtab();
}

outc(c)
char c;
{
	/* putchar(c);*/
	bufr[numchars++] = c;
}

outs(s)
char *s;
{
	if(s)
		tputs(s, 0, outc);
}

tc_bs()
{
	if (tcap.tc_bs)
		outc('\b');
	else if (tcap.tc_bc)
		outs(tcap.tc_bc);
	else
		outc('\b');
}

tc_lf()
{
	if(tcap.tc_dn)
		outs(tcap.tc_dn);
	else
		outc('\n');
}

tc_ho()
{
	if(tcap.tc_ho)
		outs(tcap.tc_ho);
	else
		tc_cm(0, 0);
}

tc_cm(x, y)
{
	outs(tgoto(tcap.tc_cm, x, y));
}

tc_ll()
{
	if(tcap.tc_ll)
		outs(tcap.tc_ll);
	else
		tc_cm(0, tcap.tc_li-1);
}

tc_vb()
{
	if (tcap.tc_vb)
		outs(tcap.tc_vb);
	else
		outc(7);
}

tc_is()
{
	if(tcap.tc_is)
		outs(tcap.tc_is);
	if(tcap.tc_if) {
		FILE *fp = fopen(tcap.tc_if, "r");
		if(fp) {
			char buf[BUFSIZ];
			while(fgets(buf, BUFSIZ, fp))
				outs(buf);
			fclose(fp);
		}
	}
	if(tcap.tc_vs)
		outs(tcap.tc_vs);
}
