*
*        l2.asm
*        Toolbox Interface library for
*
*        GNO Kernel
*        v2.0.5
*        Copyright 1991-4, Procyon Inc.
*
               case  on
               mcopy m/l2.mac

udispatch      gequ  $E10008

dummy	START		ends up in .root file
	END

* int   getpid(void) inline (0x0903, udispatch);
getpid         START	gnolib
retval         equ   1
               sub	(0:foo),2
               pha                      ; push result space
               ldx   #$0903
               jsl   udispatch
               pla
               sta   retval
               ret	2:retval
               END

* int   getppid(void) inline (0x4003, udispatch);
getppid        START	gnolib
retval         equ   1
               sub	(0:foo),2
               pha                      ; push result space
	ph4	#errno
               ldx   #$4003
               jsl   udispatch
               pla
               sta   retval
               ret	2:retval
               END

getuid         START	gnolib
retval         equ   1
               sub	(0:foo),2
               pha                      ; push result space
	ph4	#errno
               ldx   #$2A03
               jsl   udispatch
               pla
               sta   retval
               ret	2:retval
               END

getgid         START gnolib
retval         equ   1
               sub	(0:foo),2
               pha                      ; push result space
	ph4	#errno
               ldx   #$2B03
               jsl   udispatch
               pla
               sta   retval
               ret	2:retval
               END

geteuid	START gnolib
retval         equ   1
               sub	(0:foo),2
               pha                      ; push result space
	ph4	#errno
               ldx   #$2C03
               jsl   udispatch
               pla
               sta   retval
               ret	2:retval
               END

getegid        START gnolib
retval         equ   1
               sub	(0:foo),2
               pha                      ; push result space
	ph4	#errno
               ldx   #$2D03
               jsl   udispatch
               pla
               sta   retval
               ret	2:retval
               END

setuid         START gnolib
retval         equ   1
               sub (2:uid),2
               pha                      ; push result space
               ph2	uid
	ph4	#errno
	ldx   #$2E03
               jsl   udispatch
               pla
               sta   retval
               ret 2:retval
               END

setgid         START gnolib
retval         equ   1
               sub (2:gid),2
               pha                      ; push result space
               ph2	gid   
	ph4	#errno
	ldx	#$2F03
               jsl   udispatch
               pla
               sta   retval
               ret 2:retval
               END

* int   kill(int pid, int sig) inline (0x0A03, udispatch);
kill           START gnolib
retval         equ   1
               sub (2:sig,2:pid),2
               pha
               ph2   pid
               ph2   sig
               ph4   #errno             ; tell the library routine where it is
               ldx   #$0A03
               jsl   udispatch
               pla
               sta   retval
               ret 2:retval
               END

* int   fork(void *subr) inline(0x0B03, udispatch);
fork           START gnolib
retval         equ   1
               sub (4:subr),2
               pha
               ph4   subr
               ph4   #errno
               ldx   #$0B03
               jsl   udispatch
               pla
               sta   retval
               ret 2:retval
               END

* int fork2(void *subr, int stack, int prio, char *name, word argc, ...)
fork2	START	gnolib
subr	equ	7
stack	equ	subr+4
prio	equ	stack+2
name	equ	prio+2
argc	equ	name+4

* set up the variable frame
	phb
	phk
	plb
	phd
	tsc
	tcd

	pha		; temp space for result
	pei	(subr+2)
	pei	(subr)
               pei	(stack)
	pei	(prio)
	pei	(name+2)
	pei	(name)
	pea	0
	tdc
	clc
	adc	#argc
	pha
	ph4	#errno

	ldx	#$3F03
	jsl	udispatch

	pla
	tay		; temp store result in Y
	lda	argc
	asl	a
	clc
	adc	#argc
	tax
	dex
	lda	5
	sta	1,x
	lda	4
	sta	0,x
	pld
	plb
	dex
	dex
	phx
	tsc
	clc
	adc	1,s
	tcs

	tya
	rtl
	END

* int   exec(char *filename,char *cmdline) inline(0x0C03, udispatch);
*exec           START gnolib
*retval         equ   1
*               sub (4:cmdline,4:filename),2
*
*               pha
*               ph4   filename
*               ph4   cmdline
*               ph4   #errno
*               ldx   #$0C03
*               jsl   udispatch
*               pla
*               sta   retval
*               ret 2:retval
*               END

* int   swait(int sem) inline(0x0D03, udispatch);
swait          START gnolib
retval         equ   1
               sub (2:sem),2
               pha
               ph2   sem
               ph4   #errno
               ldx   #$0D03
               jsl   udispatch
               pla
               sta   retval
               ret 2:retval
               END

* int   ssignal(int sem) inline(0x0E03, udispatch);
ssignal        START gnolib
retval         equ   1
               sub (2:sem),2
               pha
               ph2   sem
               ph4   #errno
               ldx   #$0E03
               jsl   udispatch
               pla
               sta   retval
               ret 2:retval
               END

* int   screate(int count) inline(0x0F03, udispatch);
screate        START gnolib
retval         equ   1
               sub (2:sem),2
               pha
               ph2   sem
               ph4   #errno
               ldx   #$0F03
               jsl   udispatch
               pla
               sta   retval
               ret 2:retval
               END

* int   sdelete(int sem) inline(0x1003, udispatch);
sdelete        START gnolib
retval         equ   1
               sub (2:sem),2
               pha
               ph2   sem
               ph4   #errno
               ldx   #$1003
               jsl   udispatch
               pla
               sta   retval
               ret 2:retval
               END

* void  *signal(int sig, void (*func)()) inline(0x1603, udispatch);
signal         START gnolib
retval         equ   1
               sub (4:func,2:sig),4
               pha
               pha
               ph2   sig
               ph4   func
               ph4   #errno
               ldx   #$1603
               jsl   udispatch
               pla
               sta   retval  
               pla
               sta   retval+2
               ret 4:retval
               END

* int   wait(union wait *status) inline(0x1703, udispatch);
wait           START gnolib
retval         equ   1
               sub (4:status),2
               pha
               ph4   status
               ph4   #errno
               ldx   #$1703
               jsl   udispatch
               pla
               sta   retval
               ret 2:retval
               END

* int   tcnewpgrp(int fdtty) inline(0x1803, udispatch);
tcnewpgrp      START gnolib
retval         equ   1
               sub (2:fdtty),2
               pha
               ph2   fdtty
               ph4   #errno
               ldx   #$1803
               jsl   udispatch
               pla
               sta   retval
               ret 2:retval
               END

* int   settpgrp(int fdtty) inline(0x1903, udispatch);
settpgrp       START gnolib
retval         equ   1
               sub (2:fdtty),2
               pha
               ph2   fdtty
               ph4   #errno
               ldx   #$1903
               jsl   udispatch
               pla
               sta   retval
               ret 2:retval
               END

* int   tctpgrp(int fdtty, int pid) inline(0x1A03, udispatch);
tctpgrp        START gnolib
retval         equ   1
               sub (2:pid,2:fdtty),2
               pha
               ph2   fdtty
               ph2   pid
               ph4   #errno
               ldx   #$1A03
               jsl   udispatch
               pla
               sta   retval
               ret 2:retval
               END

* longword sigsetmask(longword mask) inline(0x1B03, udispatch);
sigsetmask     START gnolib
retval         equ   1
               sub (4:mask),4
               pha
               pha
               ph4   mask
               ph4   #errno
               ldx   #$1B03
               jsl   udispatch
               pla
               sta   retval  
               pla
               sta   retval+2
               ret 4:retval
               END

* longword sigblock(longword mask) inline(0x1C03, udispatch);
sigblock       START gnolib
retval         equ   1
               sub (4:mask),4
               pha
               pha
               ph4   mask
               ph4   #errno
               ldx   #$1C03
               jsl   udispatch
               pla
               sta   retval  
               pla
               sta   retval+2
               ret 4:retval
               END

* int   execve(char *filename,char *cmdline) inline(0x1D03,udispatch);
execve         START gnolib
retval         equ   1
               sub (4:cmdline,4:filename),2
               pha
               ph4   filename
               ph4   cmdline
               ph4   #errno
               ldx   #$1D03
               jsl   udispatch
               pla
               sta   retval
               ret 2:retval
               END

* longword alarm(longword seconds) inline(0x1E03,udispatch);
alarm          START gnolib
retval         equ   1
               sub (4:seconds),4
               pha
               pha
               ph4   seconds
               ph4   #errno
               ldx   #$1E03
               jsl   udispatch
               pla
               sta   retval  
               pla
               sta   retval+2
               ret 4:retval
               END

* longword alarm10(longword seconds) inline(0x4203,udispatch);
alarm10	START gnolib
retval         equ   1
               sub (4:seconds),4
               pha
               pha
               ph4   seconds
               ph4   #errno
               ldx   #$4203
               jsl   udispatch
               pla
               sta   retval  
               pla
               sta   retval+2
               ret 4:retval
               END

* int   setdebug(int code) inline(0x1F03,udispatch);
setdebug       START gnolib
retval         equ   1
               sub (2:code),2
               pha
               ph2   code
               ldx   #$1F03
               jsl   udispatch
               pla
               sta   retval
               ret 2:retval
               END

* void *setsystemvector(void *vect) inline(0x2003,udispatch);
setsystemvector START gnolib
retval         equ   1
               sub (4:vect),4
               pha
               pha
               ph4   vect
               ldx   #$2003
               jsl   udispatch
               pla
               sta   retval  
               pla
               sta   retval+2
               ret 4:retval
               END

* int   sigpause(longword mask) inline(0x2103,udispatch);
sigpause       START gnolib
retval         equ   1
               sub (4:mask),2
               pha
               ph4   mask
               ph4   #errno
               ldx   #$2103
               jsl   udispatch
               pla
               sta   retval
               ret 2:retval
               END

* kvmt *kvm_open(void) inline(0x1103, udispatch);
kvm_open       START gnolib
retval         equ   1
               sub (0:foo),4
               pha
               pha
               ph4   #errno
               ldx   #$1103
               jsl   udispatch
               pla
               sta   retval
               pla
               sta   retval+2
               ret 4:retval
               END
* int kvm_close(kvmt *k) inline(0x1203, udispatch);
kvm_close      START gnolib
retval         equ   1
               sub (4:k),2
               pha
               ph4   k
               ph4   #errno
               ldx   #$1203
               jsl   udispatch
               pla
               sta   retval
               ret 2:retval
               END

* struct pentry *kvmgetproc(kvmt *kd, int pid) inline(0x1303, udispatch);
kvmgetproc     START gnolib
retval         equ   1
               sub (2:pid,4:kd),4
               pha
               pha
               ph4   kd
               ph2   pid
               ph4   #errno
               ldx   #$1303
               jsl   udispatch
               pla
               sta   retval
               pla
               sta   retval+2
               ret 4:retval
               END

* struct pentry *kvmnextproc(kvmt *kd) inline(0x1403, udispatch);
kvmnextproc    START gnolib
retval         equ   1
               sub (4:kd),4
               pha
               pha
               ph4   kd
               ph4   #errno
               ldx   #$1403
               jsl   udispatch
               pla
               sta   retval
               pla
               sta   retval+2
               ret 4:retval
               END

* int kvmsetproc(kvmt *kd) inline(0x1503, udispatch);
kvmsetproc     START gnolib
retval         equ   1
               sub (4:kd),4
               pha
               ph4   kd
               ph4   #errno
               ldx   #$1503
               jsl   udispatch
               pla
               sta   retval
               ret 2:retval
               END

dup            START gnolib
retval         equ   1
               sub (2:fd),2
               pha
               ph2   fd
               ph4   #errno
               ldx   #$2203
               jsl   udispatch
               pla
               sta   retval
               ret 2:retval
               END

dup2           START gnolib
retval         equ   1
               sub (2:fd2,2:fd1),2
               pha
               ph2   fd1
               ph2   fd2
               ph4   #errno
               ldx   #$2303
               jsl   udispatch
               pla
               sta   retval
               ret 2:retval
               END

pipe           START gnolib
retval         equ   1
               sub (4:intptr),2
               pha
               ph4   intptr
               ph4   #errno
               ldx   #$2403
               jsl   udispatch
               pla
               sta   retval
               ret 2:retval
               END

* int   getpgrp(int pid) inline(0x2503, udispatch);
getpgrp        START gnolib
retval         equ   1
               sub (2:pid),2
               pha
               ph2   pid
               ph4   #errno
               ldx   #$2503
               jsl   udispatch
               pla
               sta   retval
               ret 2:retval
               END

* int   setpgrp(int pid) inline(0x2503, udispatch);
setpgrp        START gnolib
retval         equ   1
               sub (2:pgrp,2:pid),2
               pha
               ph2   pid
	ph2	pgrp
               ph4   #errno
               ldx   #$3403
               jsl   udispatch
               pla
               sta   retval
               ret 2:retval
               END

ioctl          START gnolib
retval         equ   1
               sub (4:argp,4:request,2:d),2
               pha
               ph2   d
               ph4   request
               ph4   argp
               ph4   #errno
               ldx   #$2603
               jsl   udispatch
               pla
               sta   retval
               ret 2:retval
               END

stat	START gnolib
retval	equ	1
	sub (4:buf,4:name),2
	pha
	ph4	name
	ph4	buf
	ph4	#errno	
	ldx	#$2703
	jsl	udispatch
	pla
	sta	retval
	ret 2:retval
	END

fstat	START gnolib
retval	equ	1
	sub (4:buf,2:fd),2
	pha
	ph2	fd
	ph4	buf
	ph4	#errno	
	ldx	#$2803
	jsl	udispatch
	pla
	sta	retval
	ret 2:retval
	END

lstat	START gnolib
retval	equ	1
	sub (4:buf,4:name),2
	pha
	ph4	name
	ph4	buf
	ph4	#errno	
	ldx	#$2903
	jsl	udispatch
	pla
	sta	retval
	ret 2:retval
	END

procsend	START gnolib
retval	equ	1
	sub (4:msg,2:pid),2
	pha
               ph2	pid
	ph4	msg
               ph4	#errno
               ldx	#$3003
               jsl	udispatch
               pla
               sta	retval
               ret 2:retval
               END

procreceive	START gnolib
retval	equ	1
	sub (0:foo),4
               pha
               pha
               ph4	#errno
               ldx	#$3103
               jsl	udispatch
               pla
	sta	retval
               pla
               sta	retval+2
               ret 4:retval
               END

procrecvclr	START gnolib
retval	equ	1
	sub (0:foo),4
               pha
               pha
	ph4	#errno
               ldx	#$3203
               jsl	udispatch
               pla
	sta	retval
               pla
               sta	retval+2
               ret 4:retval
               END
               
procrecvtim	START gnolib
retval	equ	1
	sub (2:timeout),4
	pha
	pha
	ph2	timeout
	ph4	#errno
	ldx	#$3303
	jsl	udispatch
	pla
	sta	retval
	pla
	sta	retval+2
	ret 4:retval
	END

times          START gnolib
retval         equ   1
               sub (4:buffer),2
               pha
               ph4   buffer
               ph4   #errno
               ldx   #$3503
               jsl   udispatch
               pla
               sta   retval
               ret 2:retval
               END

* int pcreate(int count);
pcreate	START	gnolib
	sub (2:count),0
	pha
	pei	(count)
	ph4	#errno
	ldx	#$3603
	jsl	udispatch
	ply
	return2
	END

* int psend(int portid, long int msg);
psend	START	gnolib
	sub (4:msg,2:portid),0
	pha
	pei	(portid)
	pei	(msg+2)
	pei	(msg)
	ph4	#errno
	ldx	#$3703
	jsl	udispatch
	ply
	return2
	END

* long int preceive(int portid);
preceive	START	gnolib
	sub (2:portid),0
	pha
	pha
	pei	(portid)
	ph4	#errno
	ldx	#$3803
	jsl	udispatch
	ply
	plx
	return2
	END

* int pdelete(int portid, int (*dispose)());
pdelete	START	gnolib
	sub (4:dispose,2:portid),0
	pha
	pei	(portid)
	pei	(dispose+2)
	pei	(dispose)
	ph4	#errno
	ldx	#$3903
	jsl	udispatch
	ply
	return2
	END

* int preset(int portid, int (*dispose)());
preset	START	gnolib
	sub (4:dispose,2:portid),0
	pha
	pei	(portid)
	pei	(dispose+2)
	pei	(dispose)
	ph4	#errno
	ldx	#$3A03
	jsl	udispatch
	ply
	return2
	END

* int pbind(int portid, char *name);
pbind	START	gnolib
	sub (4:name,2:portid),0
	pha
	pei	(portid)
	pei	(name+2)
	pei	(name)
	ph4	#errno
	ldx	#$3B03
	jsl	udispatch
	ply
	return2
	END

* int pgetport(char *name);
pgetport	START	gnolib
	sub (4:name),0
               pha
               pei	(name+2)
               pei	(name)
               ph4	#errno
               ldx	#$3C03
               jsl	udispatch
               ply
               return2
               END

* int pgetport(char *name);
pgetcount	START	gnolib
	sub (2:portnum),0
               pha
               pei	(portnum)
               ph4	#errno
               ldx	#$3D03
               jsl	udispatch
               ply
               return2
               END
                  
* int pgetport(char *name);
scount	START	gnolib
	sub (2:sem),0
               pha
               pei	(sem)
               ph4	#errno
               ldx	#$3E03
               jsl	udispatch
               ply
               return2
               END

* void SetGNOQuitRec(word pCount,GSString255Ptr pathname, word flags)
SetGNOQuitRec	START gnolib
retval         equ   1
               sub (2:flags,4:pathname,2:pCount),0
               pha
	ph2	pCount
	ph4	pathname
	ph2	flags
               ph4   #errno             ; tell the library routine where it is
               ldx   #$4103
               jsl   udispatch
	pla
               ret
               END

select	START gnolib
res	equ	1
	sub (4:toptr,4:exc,4:wr,4:rd,2:nfd),2

	pha
	pei	(nfd)
	pei	(rd+2)
	pei	(rd)
	pei	(wr+2)
	pei	(wr)
	pei	(exc+2)
	pei	(exc)
	pei	(toptr+2)
	pei	(toptr)
	ph4	#errno
	ldx	#$4303
	jsl	udispatch
	pla
	sta	res
	ret 2:res
	END

InstallNetDriver START gnolib
res            equ   1
               sub (4:netcore),2
               pha
	pei   (netcore+2)
               pei   (netcore)
               ph4   #errno
               ldx   #$4403
               jsl   udispatch
               pla
	sta	res
	ret 2:res
	END

socket	START	gnolib
res	equ	1
	sub (2:protocol,2:type,2:domain),2
	pha
	pei	(domain)
	pei	(type)
	pei	(protocol)
	ph4	#errno
	ldx	#$4503
	jsl	udispatch
	pla
	sta	res
	ret 2:res
	END

bind	START	gnolib
res	equ	1
	sub (2:addrlen,4:myaddr,2:fd),2
	pha
	pei	(fd)
	pei	(myaddr+2)
	pei	(myaddr)
	pei	(addrlen)
	ph4	#errno
	ldx	#$4603
	jsl	udispatch
	pla
	sta	res
	ret 2:res
	END

connect	START	gnolib
res	equ	1
	sub (2:addrlen,4:servaddr,2:fd),2
	pha
	pei	(fd)
	pei	(servaddr+2)
	pei	(servaddr)
	pei	(addrlen)
	ph4	#errno
	ldx	#$4703
	jsl	udispatch
	pla
	sta	res
	ret 2:res
	END

listen	START	gnolib
res	equ	1
	sub (2:backlog,2:fd),2
	pha
	pei	(fd)
	pei	(backlog)
	ph4	#errno
	ldx	#$4803
	jsl	udispatch
	pla
	sta	res
	ret 2:res
	END

accept	START gnolib
res	equ	1
	sub (4:addrlen,4:remaddr,2:fd),2
	pha
	pei	(fd)
	pei	(remaddr+2)
	pei	(remaddr)
	pei	(addrlen+2)
	pei	(addrlen)
	ph4	#errno
	ldx	#$4903
	jsl	udispatch
	pla
	sta	res
	ret 2:res
	END

recvfrom	START	gnolib
res	equ	1
	sub (4:addrlen,4:remaddr,2:flags,4:len,4:buf,2:fd),2

	pha
	pei	(fd)
	pei	(buf+2)
	pei	(buf)
	pei	(len+2)
	pei	(len)
	pei	(flags)
	pei	(remaddr+2)
	pei	(remaddr)
	pei	(addrlen+2)
	pei	(addrlen)
	ph4	#errno
	ldx	#$4A03
	jsl	udispatch
	pla
	sta	res
	ret 2:res
	END

sendto	START	gnolib
res	equ	1
	sub (2:addrlen,4:remaddr,2:flags,4:len,4:buf,2:fd),2

	pha
	pei	(fd)
	pei	(buf+2)
	pei	(buf)
	pei	(len+2)
	pei	(len)
	pei	(flags)
	pei	(remaddr+2)
	pei	(remaddr)
	pei	(addrlen)
	ph4	#errno
	ldx	#$4B03
	jsl	udispatch
	pla
	sta	res
	ret 2:res
	END

recv	START	gnolib
res	equ	1
	sub (2:flags,4:len,4:buf,2:fd),2

	pha
	pei	(fd)
	pei	(buf+2)
	pei	(buf)
	pei	(len+2)
	pei	(len)
	pei	(flags)
	ph4	#errno
	ldx	#$4C03
	jsl	udispatch
	pla
	sta	res
	ret 2:res
	END

send	START	gnolib
res	equ	1
	sub (2:flags,4:len,4:buf,2:fd),2

	pha
	pei	(fd)
	pei	(buf+2)
	pei	(buf)
	pei	(len+2)
	pei	(len)
	pei	(flags)
	ph4	#errno
	ldx	#$4D03
	jsl	udispatch
	pla
	sta	res
	ret 2:res
	END
