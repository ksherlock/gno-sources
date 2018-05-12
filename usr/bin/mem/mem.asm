* mem.asm - simple memory utility for GNO

		keep	mem
		mcopy	mem.mac
		case	on

G_Bank0		gequ	0
G_Count		gequ	2
G_Optind	gequ	4
G_Total		gequ	4
G_Place		gequ	8
G_MemID		gequ	10
G_argv		gequ	12
G_RBank0	gequ	14
G_Hand		gequ	16
G_kvmhand	gequ	20
G_perp		gequ	24
G_kvmptr	gequ	26
G_kvmlen	gequ	30
G_kvmt		gequ	34
G_pentry	gequ	38
GG_pid		gequ	42
GG_mmid		gequ	44
GG_cline	gequ	46

main		start
		using	string
		using	error_msg

		jsl	GNO_Parse
		lda	>argv
		sta	G_argv
		lda	>argv+2
		sta	G_argv+2
		lda	>memid
		sta	G_MemID
		stz	G_Optind
		stz	G_perp
optloop		anop
lpp15		lda	>argc
		dec	a
		sta	>argc
		beq	nomoreargs
		inc	G_Optind
		inc	G_Optind
		ldy	G_Optind
		lda	[G_argv],y
		tay
		lda	[G_argv],y
		cmp	#2
		bcc	usage
		tax
		dex
		iny
		iny
		lda	[G_argv],y
		and	#$ff
		cmp	#'-'
		bne	usage
lpp60		cpx	#0
		beq	optloop
		iny
		lda	[G_argv],y
		dex
		and	#$ff
		cmp	#'p'
		bne	checknext
		lda	#1
		sta	G_perp
		bra	lpp60
checknext	cmp	#'V'
		bne	usage
		WriteCString #versstr
		brl	getout
usage		ErrWriteCString #usgstr
		brl	getout
nomoreargs	anop
		phk
		plb
		lda	G_perp
		beq	normal
		brl	process
normal		tsc
		sec
		sbc	#12		; 4 space freemem, 8 space LDivide
		tcs
		~TotalMem
		lda	1,s
		sta	G_Total
		lda	3,s
		sta	G_Total+2
		ph4	#1024
		~LongDivide *,*
		ph4	#total
		ph2	#5
		ph2	#0
		~Long2Dec *,*,*,*
		tsc
		sec
		sbc	#8		; 8 space for LDivide, 4 already there
		tcs
		~RealFreeMem
		lda	#realp
		jsr	percent
		ph4	#1024
		~LongDivide *,*
		ph4	#realfree
		ph2	#5
		ph2	#0
		~Long2Dec *,*,*,*
		tsc
		sec
		sbc	#8
		tcs
		~MaxBlock
		ph4	#1024
		~LongDivide *,*
		ph4	#maxblock
		ph2	#5
		ph2	#0
		~Long2Dec *,*,*,*
		tsc
		sec
		sbc	#8
		tcs
		~FreeMem
		lda	#freep
		jsr	percent
		ph4	#1024
		~LongDivide *,*
		ph4	#free
		ph2	#5
		ph2	#0
		~Long2Dec *,*,*,*	; 4 byte remainder still on stk after
		pla
		pla
*		pha
*		pha
*		pha			; now 6 bytes on stack
*		ph2	#$36
*		~ReadBParam *
*		ph2	#32
*		~Multiply *,*
*		ph4	#ramdisk
*		ph2	#5
*		ph2	#0
*		~Long2Dec *,*,*,*
		stz	G_Bank0
		stz	G_Count
		stz	G_RBank0
		pha
		pha
loop		pea	0		; bank byte of address
		lda	G_Count
		xba
		pha			; page number, multiply by 256
		~FindHandle *
		lda	1,s
		ora	3,s
		bne	used
		inc	G_Bank0
used3		inc	G_RBank0
		bra	used2
used		anop
		tsc
		phd
		tcd
		ldy	#4
		lda	[1],y
		pld
		tax
		and	#%1000000000000000
		bne	used2
		txa
		and	#%0000001100000000
		bne	used3
used2		inc	G_Count
		lda	G_Count
		cmp	#$100
		bcc	loop
		pla
		pla
		pea	0
		pei	G_Bank0		; pages free
		lda	#$100		; pages total
		stz	G_Total+2
		sta	G_Total
		lda	#bank0p
		jsr	percent
		pla
		plx
		lsr	a
		lsr	a		; make into a number of K
		pha
		ph4	#bank0
		ph2	#5
		ph2	#0
		~Int2Dec *,*,*,*
		pea	0
		pei	G_RBank0
		lda	#rbank0p
		jsr	percent
		pla
		plx
		lsr	a
		lsr	a		; make into a number of K
		pha
		ph4	#rbank0
		ph2	#5
		ph2	#0
		~Int2Dec *,*,*,*

		WriteGS	wrrec
		GSError	#writmsg
		lda	#0
		bra	getout
badout		entry
		lda	#1
getout		entry
		pha
		lda	G_kvmhand
		ora	G_kvmhand+2
		beq	nodisp
		~DisposeHandle <G_kvmhand
nodisp		~DisposeHandle >argvhand
		pla
		QuitGS	qtrec
		end

process		start
		using	error_msg
		using	string2
		pha
		pha
		pea	0
		phd
		~FindHandle *
		pla
		sta	G_Hand
		pla
		sta	G_Hand+2
* Handle:
*  0 = pointer
*  4 = attr
*  6 = owner
*  8 = size
*  C = previous
*  10= next
lpp271		ldy	#$c
		lda	[G_Hand],y
		tax
		ldy	#$e
		ora	[G_Hand],y
		beq	beginning
		lda	[G_Hand],y
		stx	G_Hand
		sta	G_Hand+2
		bra	lpp271
beginning	anop
		pha
		pha
		pea	0
		pea	2048		; 16 * 128
		pei	G_MemID
		pea	$8000
		pha
		pha
		~NewHandle *,*,*,*
		plx
		stx	G_kvmhand
		plx
		stx	G_kvmhand+2
		GSError	#procread
		ldy	#2
		lda	[G_kvmhand],y
		sta	G_kvmptr+2
		lda	[G_kvmhand]
		sta	G_kvmptr
		lda	#5
		sta	G_kvmlen
		ldy	#0
		ldx	#$0000
		jsr	fill
		ldx	#$ffff
		jsr	fill
		ldx	#$4000
		jsr	fill
		ldx	#$3000
		jsr	fill
		ldx	#$5000
		jsr	fill

		pha
		pha
		pea	errno|-16
		pea	errno
		ldx	#$1103		; kvm_open
		jsl	$e10008
		pla
		plx
		GNOError #procread,,4
		sta	G_kvmt
		stx	G_kvmt+2

lpp284		pha
		pha
		pei	G_kvmt+2
		pei	G_kvmt
		pea	errno|-16
		pea	errno
		ldx	#$1403		; kvm_nextproc
		jsl	$e10008
		pla
		plx
		GNOError #procread,,4
		sta	G_pentry
		stx	G_pentry+2
		ora	G_pentry+2
		beq	fullnull
		ldy	#4
		lda	[G_pentry],y
		sta	GG_mmid
		ldy	#2
		lda	[G_kvmt],y
		sta	GG_pid
		ldy	#34
		lda	[G_pentry],y
		sta	GG_cline
		ldy	#36
		lda	[G_pentry],y
		sta	GG_cline+2
		lda	G_kvmlen
		asl	a
		asl	a
		asl	a
		asl	a
		tay
* 0 = pid
* 2 = mid
* 4 = b0mem
* 8 = rmem
* 12= cline
		lda	GG_pid
		sta	[G_kvmptr],y
		iny
		iny
		lda	GG_mmid
		sta	[G_kvmptr],y
		iny
		iny
		lda	#0
		sta	[G_kvmptr],y
		iny
		iny
		sta	[G_kvmptr],y
		iny
		iny
		sta	[G_kvmptr],y
		iny
		iny
		sta	[G_kvmptr],y
		iny
		iny
		lda	GG_cline
		sta	[G_kvmptr],y
		iny
		iny
		lda	GG_cline+2
		sta	[G_kvmptr],y
		inc	G_kvmlen
		lda	G_kvmlen
		cmp	#128
		bcs	fullnull
		brl	lpp284

fullnull	pha
		pei	G_kvmt+2
		pei	G_kvmt
		pea	errno|-16
		pea	errno
		ldx	#$1203
		jsl	$e10008
		pla
		GNOError #procread,,2

handloop	anop
		lda	[G_Hand]
		cmp	G_Hand
		bne	notfree
		ldy	#2
		lda	[G_Hand]
		cmp	G_Hand+2
		beq	free
notfree		ldy	#2
		lda	[G_Hand],y
		sta	G_Bank0
		ora	[G_Hand]
		beq	free		; purged handle
		ldy	#8
		lda	[G_Hand],y
		sta	G_kvmt
		ldy	#10
		lda	[G_Hand],y
		sta	G_kvmt+2
		ldy	#4
		lda	[G_Hand],y
		and	#%1000000000000000
		bne	notpurg		; locked - not purgeable
		lda	[G_Hand],y
		and	#%0000001100000000
		beq	notpurg
		ldy	#$14		; purgeable
		bra	bymin
notpurg		ldy	#6
		lda	[G_Hand],y
		beq	free
		jsr	bymemid
		iny
		iny
bymin		lda	G_Bank0
		beq	inbank0
		iny
		iny
		iny
		iny
inbank0		anop
		lda	[G_kvmptr],y
		clc
		adc	G_kvmt		; handle size
		sta	[G_kvmptr],y
		iny
		iny
		lda	[G_kvmptr],y
		adc	G_kvmt+2
		sta	[G_kvmptr],y

free		jsr	nexthand
		bcc	handloop		

		WriteGString #headline

		ldx	#4
finalloop	inx
		cpx	G_kvmlen
		beq	allfin
		phx
		txa
		asl	a
		asl	a
		asl	a
		asl	a
		tay
		lda	[G_kvmptr],y
		phy
		pha
		pea	PP_pid|-16
		pea	PP_pid
		pea	9
		pea	0
		~Int2Dec *,*,*,*
		ply
		ldx	#PP_b0mem
		jsr	kbytes
		ldx	#PP_nmem
		jsr	kbytes
		iny
		iny
		iny
		iny			; to cline
		phy
		WriteGString #format
		ply
		lda	[G_kvmptr],y
		clc
		adc	#8
		tax
		iny
		iny
		lda	[G_kvmptr],y
		adc	#0
		tay
		lda	#2
		jsl	WriteCString
		WriteGString #nlonly
		plx
		bra	finalloop
allfin		anop
		ldx	#5
floop2		dex
		bmi	allfinagain
		phx
		txa
		asl	a
		asl	a
		asl	a
		tax
		asl	a
		tay
		txa
		clc
		adc	1,s
		adc	#9
		tax
		phy
		ldy	#7
namlp		lda	lablab-2,x
		sta	PP_pid,y
		dex
		dey
		bpl	namlp
		ply
		ldx	#PP_b0mem
		jsr	kbytes
		ldx	#PP_nmem
		jsr	kbytes
		WriteGString #format
		WriteGString #nlonly
		plx
		bra	floop2
allfinagain	brl	getout
		end

kbytes		start
		using	string2
		iny
		iny
		iny
		iny
		phy
		lda	[G_kvmptr],y
		sta	G_kvmt
		iny
		iny
		lda	[G_kvmptr],y
		sta	G_kvmt+2
		bne	inkk
		lda	G_kvmt
		cmp	#2048
		bcs	inkk
		phx
		inx
		inx
		inx
		inx
		inx
		lda	#$2020
		sta	|0,x
		bra	kk2
inkk		anop
		phx
		inx
		inx
		inx
		inx
		inx
		lda	#$206b
		sta	|0,x
		lda	G_kvmt
		ldx	#10
shifter		lsr	G_kvmt+2
		ror	a
		dex
		bne	shifter
		sta	G_kvmt
kk2		anop
		plx
		pei	G_kvmt
		pea	PP_pid|-16
		phx
		pea	5
		pea	0
		~Int2Dec *,*,*,*
		ply
		rts
		end

nexthand	start
		ldy	#$10
		lda	[G_Hand],y
		tax
		iny
		iny
		ora	[G_Hand],y
		beq	scrap
		lda	[G_Hand],y
		sta	G_Hand+2
		stx	G_Hand
		clc
		rts
scrap		sec
		rts
		end

bymemid		start
* Call this, inc twice to get b0mem location
		and	#$f0ff
		pha			; memid
		ldx	G_kvmlen
byloop		dex
		beq	catother
		cpx	#4
		beq	notnorm
byin		txa
		asl	a
		asl	a
		asl	a
		asl	a
		tay
		iny
		iny
		lda	[G_kvmptr],y
		and	#$f0ff
		cmp	1,s
		bne	byloop
		pla
		rts
notnorm		pla
		and	#$f000
		pha
		bra	byin	
catother	ldy	#2
		pla
		rts
		end

fill		start
		lda	#$ffff
		sta	[G_kvmptr],y
		iny
		iny
		txa
		sta	[G_kvmptr],y
		lda	#0
		ldx	#6
		iny
		iny
ffill		dex
		bmi	dfill
		sta	[G_kvmptr],y
		iny
		iny
		bra	ffill
dfill		rts
		end

string2		data
headline	dc	i'm12end-m12beg'
m12beg		dc	c' ProcessID    Bank 0    Other     Command Line'
		dc	h'0d'
		dc	c' ---------    ------    ------    ------------'
		dc	h'0d'
m12end		anop

nlonly		dc	h'01 00 0d'

format		dc	i'm13end-m13beg'
m13beg		dc	c' '
PP_pid		dc	c'xxxxxxxxx    '
PP_b0mem	dc	c'xxxxx'
PP_kloc1	dc	c'k    '
PP_nmem		dc	c'xxxxx'
PP_kloc2	dc	c'k    '
m13end		anop

lablab		dc	c'    Other'
		dc	c'Purgeable'
		dc	c'    Tools'
		dc	c'    GS/OS'
		dc	c'Desk.Accs'
		end

error_msg	data
writmsg		dc	i'm1end-m1beg'
m1beg		dc	c' while writing to standard out.'
		dc	h'0d'
m1end		anop

procread	dc	i'm2end-m2beg'
m2beg		dc	c' while reading process table.'
		dc	h'0d'
m2end		anop
		end

error_hook	start
		brl	badout
		end

percent		start
		sta	G_Place
		tsc
		sec
		sbc	#12
		tcs
* Stack high(15:longvalue, 13:rts, 1:space(12))low
		lda	17,s		; caller's stack
		pha
		lda	17,s
		pha
		ph4	#100
		~LongMul *,*
* Stack high(15:longvalue, 13:rts, 9:space(4), 5:garbage(4), 1:result(4))low
		pei	G_Total+2
		pei	G_Total
		~LongDivide *,*
* Stack high(11:longvalue, 9:rts, 5:grabage(4), 1:result)low
		lda	#100
		sec
		sbc	1,s
		sta	1,s
		pea	string|-16
		pei	G_Place
		ph2	#2
		ph2	#0
		~Long2Dec *,*,*,*
* Stack high(7:longvalue, 5:rts, 1:garbage(4))low
		pla
		pla
		rts
		end

string		data

		dc	c'        Free mem: '
free		dc	c'xxxxxk ('
freep		dc	c'%%% used)          Max block: '
maxblock	dc	c'xxxxxk',h'0d'
		dc	c'   Real free mem: '
realfree	dc	c'xxxxxk ('
realp		dc	c'%%% used)          Total mem: '
total		dc	c'xxxxxk',h'0d'
		dc	c' Free bank 0 mem: '
bank0		dc	c'xxxxxk ('
bank0p		dc	c'%%% used)   Real free bank 0: '
rbank0		dc	c'xxxxxk ('
rbank0p		dc	c'%%% used)',h'0d'
strend		anop

wrrec		dc	i'4'
		dc	i'2'
		dc	a4'string'
		dc	i4'strend-string'
		ds	4
qtrec		dc	i'2'
		dc	a4'0'
		dc	i'%0100000000000000'

versstr		dc	c'Mem version 1.2 by Phillip Vandry'
		dc	h'0d'

usgstr		dc	c'Usage: mem [-pV]'
		dc	h'0d 00'
		end

dpstk		data	~Direct
		kind	$12
		ds	512
		end
