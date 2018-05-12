#include <sgtty.h>
#include <termios.h>
#include <signal.h>
#include <stdlib.h>

int cfgetospeed(const struct termios *p)
{
    return p->ospeed;
}

int cfsetospeed(struct termios *p, speed_t speed)
{
    p->ospeed = speed;
    return 0;
}

int cfgetispeed(const struct termios *p)
{
    return p->ispeed;
}

int cfsetispeed(struct termios *p, speed_t speed)
{
    p->ispeed = speed;
    return 0;
}

int sigemptyset(sigset_t *set)
{
    memset(set,0,sizeof(sigset_t));
    return 0;
}

int sigaddset(sigset_t *set, int signo)
{
    *set |= sigmask(signo);
    return 0;
}

int sigprocmask(int how, sigset_t *set, sigset_t *oset)
{
sigset_t old;

    switch (how) {
    case SIG_BLOCK:
    	old = sigblock(*set);
	if (oset != NULL) *oset = old;
	return 0;
    case SIG_UNBLOCK:
    	old = sigsetmask(0xFFFFFFFF);
	sigsetmask(old & ~(*set));
	if (oset != NULL) *oset = old;
	return 0;
    case SIG_SETMASK:
    	old = sigsetmask(*set);
	if (oset != NULL) *oset = old;
	return 0;
    }
}

#define         old_ECHO            0x00000008l     /* echo input */

int cfmakeraw(struct termios *tp)
{
    tp->c_lflag &= (~ICANON);
    return 0;
}

int tcsetattr(int filedes, unsigned long foo, struct termios *tp)
{
struct sgttyb sg;
struct tchars tc;
struct ltchars ltc;
char chs[NCCS];
unsigned flgs = 0;
int res;

    if (res = gtty(filedes,&sg)) return res;
    if (res = ioctl(filedes,TIOCGETC,&tc)) return res;
    if (res = ioctl(filedes,TIOCGLTC,&ltc)) return res;

    sg.sg_ispeed = tp->ispeed;
    sg.sg_ospeed = tp->ospeed;

    flgs = (tp->c_lflag & ICANON) ? 0 : RAW;
    flgs |= (tp->c_lflag & ISIG) ? 0 : CBREAK;
    flgs |= (tp->c_lflag & ECHO) ? ECHO : 0;

    memcpy(chs,tp->c_cc,NCCS);
    tc.t_eofc = chs[VEOF];
    tc.t_brkc = chs[VEOL];
    tc.t_intrc = chs[VINTR];
    tc.t_quitc = chs[VQUIT];
    tc.t_startc = chs[VSTART];
    tc.t_stopc = chs[VSTOP];
    ltc.t_suspc = chs[VSUSP];
    sg.sg_erase = chs[VERASE];
    sg.sg_kill = chs[VKILL];
    if (res = stty(filedes,&sg)) return res;
    if (res = ioctl(filedes,TIOCSETC,&tc)) return res;
    if (res = ioctl(filedes,TIOCSLTC,&ltc)) return res;
    return 0;
}

int tcgetattr(int filedes, struct termios *tp)
{
struct sgttyb sg;
struct tchars tc;
struct ltchars ltc;
char chs[NCCS];
unsigned cflag = 0,lflag = 0,iflag = 0,oflag = 0;
int res;

    if (res = gtty(filedes,&sg)) return res;
    if (res = ioctl(filedes,TIOCGETC,&tc)) return res;
    if (res = ioctl(filedes,TIOCGLTC,&ltc)) return res;

    tp->ispeed = sg.sg_ispeed;
    tp->ospeed = sg.sg_ospeed;

    lflag =  (sg.sg_flags & (RAW)) ? 0 : ICANON;
    lflag |= (sg.sg_flags & (CBREAK)) ? 0 : ISIG;
    lflag |= (sg.sg_flags & (old_ECHO)) ? (ECHO) : 0;

    tp->c_iflag = iflag;
    tp->c_oflag = oflag;
    tp->c_lflag = lflag;
    tp->c_cflag = cflag;

    chs[VEOF] = tc.t_eofc;
    chs[VEOL] = tc.t_brkc;
    chs[VINTR] = tc.t_intrc;
    chs[VQUIT] = tc.t_quitc;
    chs[VSTART] = tc.t_startc;
    chs[VSTOP] = tc.t_stopc;
    chs[VSUSP] = ltc.t_suspc;
    chs[VERASE] = sg.sg_erase;
    chs[VKILL] = sg.sg_kill;
    memcpy(tp->c_cc,chs,NCCS);
    return 0;
}
