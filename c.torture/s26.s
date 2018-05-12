Ltext:	.stabs "s26.c",100,0,0,Ltext
.stabs "int:t1=r1;-2147483648;2147483647;",128,0,0,0
.stabs "char:t2=r2;0;127;",128,0,0,0
.stabs "void:t3=3",128,0,0,0
.stabs "long double:t4=r1;8;0;",128,0,0,0
.stabs "double:t5=r1;8;0;",128,0,0,0
.stabs "float:t6=r1;4;0;",128,0,0,0
.stabs "unsigned char:t7=r1;0;255;",128,0,0,0
.stabs "char:t2",128,0,0,0
.stabs "long unsigned int:t8=r1;0;-1;",128,0,0,0
.stabs "unsigned int:t9=r1;0;-1;",128,0,0,0
.stabs "short unsigned int:t10=r1;0;65535;",128,0,0,0
.stabs "long int:t11=r1;-2147483648;2147483647;",128,0,0,0
.stabs "short int:t12=r1;-32768;32767;",128,0,0,0
.stabs "int:t1",128,0,0,0
.data
_qs26$0:
	.asciz "s26    "
.stabs "qs26:v13=ar1;0;7;2",38,0,0,_qs26$0
.data
_s$1:
	.asciz "%3d bits in %ss.\012"
.stabs "s:v14=ar1;0;17;2",38,0,0,_s$1
.data
_s2$2:
	.asciz "%e is the least number that can be added to 1. (%s).\012"
.stabs "s2:v15=ar1;0;53;2",38,0,0,_s2$2
.globl _s26
.text
LC0:
	.asciz "char"
.text
LC1:
	.asciz "int"
.text
LC2:
	.asciz "short"
.text
LC3:
	.asciz "long"
.text
LC4:
	.asciz "unsigned"
.text
LC5:
	.asciz "float"
.text
LC6:
	.asciz "double"
.text
	.even
_s26:
	.stabs "./defs",132,0,0,Ltext
	.stabd 68,0,1
	link a6,#0
	fmovem #0xc,sp@-
	moveml #0x2030,sp@-
	movl a6@(8),a3
	.stabs "s26.c",132,0,0,Ltext
	.stabd 68,0,4
LBB2:
	.stabd 68,0,13
	lea _qs26$0,a1
	.stabd 68,0,14
	lea a3@(60),a0
	.stabd 68,0,16
L2:
	movb a1@+,d0
	movb d0,a0@+
	jne L2
	.stabd 68,0,21
	clrl a3@
	.stabd 68,0,22
	clrb d1
	.stabd 68,0,23
	movb #1,d0
	.stabd 68,0,25
L7:
	.stabd 68,0,26
	aslb #1,d0
	.stabd 68,0,27
	addql #1,a3@
	cmpb d1,d0
	jne L7
	.stabd 68,0,31
	movl a3@,d2
	asll #2,d2
	movl d2,a3@(4)
	.stabd 68,0,32
	movl a3@,d2
	asll #1,d2
	movl d2,a3@(8)
	.stabd 68,0,33
	movl a3@,d2
	asll #2,d2
	movl d2,a3@(12)
	.stabd 68,0,34
	movl a3@,d2
	asll #2,d2
	movl d2,a3@(16)
	.stabd 68,0,35
	movl a3@,d2
	asll #2,d2
	movl d2,a3@(20)
	.stabd 68,0,36
	movl a3@,d2
	asll #3,d2
	movl d2,a3@(24)
	.stabd 68,0,54
	fmoves #0r1,fp2
	.stabd 68,0,55
	fmoves fp2,d0
	.stabd 68,0,56
	.stabd 68,0,57
	fmoves #0r0,fp3
	fcmpx fp2,fp3
	fjeq L24
L12:
	.stabd 68,0,58
	fmovex fp2,fp1
	fadds d0,fp1
	.stabd 68,0,59
	fmoves d0,fp0
	fdivd #0r2,fp0
	fmoves fp0,d0
	fcmpx fp2,fp1
	fjne L12
L24:
	.stabd 68,0,61
	fmoves d0,fp0
	fmuld #0r4,fp0
	fmoves fp0,a3@(28)
	.stabd 68,0,62
	fmovecr #0x32,fp2
	.stabd 68,0,63
	movl #0x3f800000,d0
	.stabd 68,0,64
	.stabd 68,0,65
	fmovecr #0xf,fp3
	fcmpx fp2,fp3
	fjeq L23
L17:
	.stabd 68,0,66
	fmoves d0,fp0
	fmovex fp2,fp1
	faddx fp0,fp1
	.stabd 68,0,67
	fdivd #0r2,fp0
	fmoves fp0,d0
	fcmpx fp2,fp1
	fjne L17
L23:
	.stabd 68,0,69
	fmoves d0,fp0
	fmuld #0r4,fp0
	fmoves fp0,a3@(32)
	.stabd 68,0,73
	tstl a3@(40)
	jeq L22
LBB3:
	.stabd 68,0,74
	pea LC0
	movl a3@,sp@-
	pea _s$1
	lea _printf,a2
	jbsr a2@
	.stabd 68,0,75
	pea LC1
	movl a3@(4),sp@-
	pea _s$1
	jbsr a2@
	.stabd 68,0,76
	pea LC2
	movl a3@(8),sp@-
	pea _s$1
	jbsr a2@
	.stabd 68,0,77
	addw #36,sp
	pea LC3
	movl a3@(12),sp@-
	pea _s$1
	jbsr a2@
	.stabd 68,0,78
	pea LC4
	movl a3@(16),sp@-
	pea _s$1
	jbsr a2@
	.stabd 68,0,79
	pea LC5
	movl a3@(20),sp@-
	pea _s$1
	jbsr a2@
	.stabd 68,0,80
	addw #36,sp
	pea LC6
	movl a3@(24),sp@-
	pea _s$1
	jbsr a2@
	.stabd 68,0,81
	pea LC5
	fmoves a3@(28),fp3
	fmoved fp3,sp@-
	pea _s2$2
	jbsr a2@
	.stabd 68,0,82
	pea LC6
	fmoves a3@(32),fp3
	fmoved fp3,sp@-
	pea _s2$2
	jbsr a2@
LBE3:
	addw #44,sp
L22:
	.stabd 68,0,87
	clrl d0
LBE2:
	fmovem a6@(-24),#0x30
	moveml a6@(-36),#0xc04
	unlk a6
	rts
