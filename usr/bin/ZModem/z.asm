	mcopy	z.mac

	case	on

ZDLE	gequ  24

zsendline	START

	lda	4,s
	bit	#$60
	beq	couldbectl
	sta	lastsent
	jsr	xsendline1
	bra	goaway

couldbectl	anop
	and	#$7F
	cmp	#$0D
	beq	isCR
	cmp	#$10
	beq	isCtl
	cmp	#$11
	beq	isCtl
	cmp	#$13
	beq	isCtl
	lda	4,s
	and	#$FF
	cmp	#ZDLE
	beq	isCtl

default	tay
	lda	Zctlesc
	beq	notesc
	tya
	bit	#$60
	bne	notesc
	lda	#ZDLE
	jsr	xsendline1
	lda	4,s
	eor	#$40
	sta	lastsent
	jsr	xsendline1
	bra	goaway
notesc	tya
	sta	lastsent
	jsr	xsendline1
	bra	goaway
notcr	anop
*  We actually also handle ZDLE here too, since the code is the same!
*  Looks like what's his name is a moron
isCtl	lda	#ZDLE
	jsr	xsendline1
	lda	4,s
	eor	#$40
sendit	sta	lastsent
	jsr	xsendline1
	bra	goaway
isCR	lda	Zctlesc
               beq	notcr
	lda	lastsent
	and	#$7F
	cmp	#'@'
	bne	sendit
	lda	4,s
	bra	notcr

goaway	anop
               lda	2,s
	sta	4,s
	lda	1,s
	sta	3,s
	pla
	rtl
	END

xsendline1	START
	ldy	outbufInd
	short	m
	sta	outbuf,y
	long	m
	iny
	sty	outbufInd
	cpy	#512
	beq	flushit
	rts
flushit        jsl	flushmo
	rts
	END

* 4,5,s - data
* 1,2,3,s - rtl address

xsendline	START
	lda	4,s

* exactly the same as above
	ldy	outbufInd
	short	m
	sta	outbuf,y
	long	m
	iny
	sty	outbufInd
	cpy	#512
	beq	doflush
	lda	2,s
	sta	4,s
	lda	1,s
	sta	3,s
	pla
	rtl
doflush	jsl	flushmo
	lda	2,s
	sta	4,s
	lda	1,s
	sta	3,s
	pla
	rtl
	END
