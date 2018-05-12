	case	on
	mcopy	wia.mac

CloseIt	START
	phb
	phk
	plb
	lda	win
	ora	win+2
               beq	nowindow
		
               ph4	win
	_CloseWindow

	stz	win
	stz	win+2

nowindow	plb
	rtl	
	END

ActionIt	START
	pha
	phy
	phx
	jsl	CActionIt
	rtl
	END
