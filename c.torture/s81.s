#NO_APP
Ltext:	.stabs "s81.c",100,0,0,Ltext
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
_s81er$0:
	.ascii "s81,er%d\12\0"
.stabs "s81er:v13=ar1;0;9;2",38,0,0,_s81er$0
.data
_qs81$1:
	.ascii "s81    \0"
.stabs "qs81:v14=ar1;0;7;2",38,0,0,_qs81$1
.data
_badtest$2:
	.ascii "Register count for %s is unreliable.\12\0"
.stabs "badtest:v15=ar1;0;37;2",38,0,0,_badtest$2
.data
_goodtest$3:
	.ascii "%d registers assigned to %s variables.\12\0"
.stabs "goodtest:v16=ar1;0;39;2",38,0,0,_goodtest$3
.globl _s81
.text
LC0:
	.ascii "char\0"
.text
LC1:
	.ascii "pointer\0"
.text
LC2:
	.ascii "int\0"
.text
	.even
_s81:
	.stabs "./defs",132,0,0,Ltext
	.stabd 68,0,1
	link a6,#-42
	moveml #0x3e00,sp@-
	.stabs "s81.c",132,0,0,Ltext
	.stabd 68,0,4
LBB2:
	.stabd 68,0,18
	clrl a6@(-16)
	.stabd 68,0,19
	clrl a6@(-24)
	.stabd 68,0,20
	clrl a6@(-28)
	.stabd 68,0,21
	clrl a6@(-32)
	.stabd 68,0,22
	movel #_qs81$1,a6@(-4)
	.stabd 68,0,23
	movel a6@(8),d0
	moveq #60,d6
	addl d0,d6
	movel d6,a6@(-8)
	.stabd 68,0,25
L2:
	movel a6@(-8),a0
	movel a6@(-4),a1
	moveb a1@,a0@
	addql #1,a6@(-4)
	addql #1,a6@(-8)
	tstb a0@
	jeq L3
L4:
	jra L2
L3:
L5:
	.stabd 68,0,43
	moveq #1,d6
	movel d6,a6@(-12)
	.stabd 68,0,44
	clrl a6@(-20)
L6:
	moveq #50,d6
	cmpl a6@(-20),d6
	jle L7
	.stabd 68,0,45
	moveb a6@(-9),d2
	.stabd 68,0,46
	moveb a6@(-9),a6@(-34)
	.stabd 68,0,47
	moveq #-12,d3
	addl a6,d3
	.stabd 68,0,48
	moveq #-12,d6
	addl a6,d6
	movel d6,a6@(-38)
	.stabd 68,0,49
	movel a6@(-12),d4
	.stabd 68,0,50
	movel a6@(-12),a6@(-42)
	.stabd 68,0,52
	cmpb a6@(-34),d2
	jeq L8
	moveq #1,d6
	movel d6,a6@(-24)
L8:
	.stabd 68,0,53
	cmpl a6@(-38),d3
	jeq L9
	moveq #1,d6
	movel d6,a6@(-28)
L9:
	.stabd 68,0,54
	cmpl a6@(-42),d4
	jeq L10
	moveq #1,d6
	movel d6,a6@(-32)
L10:
	.stabd 68,0,55
	movel a6@(-12),d6
	asll #1,d6
	movel d6,a6@(-12)
L11:
	.stabd 68,0,44
	addql #1,a6@(-20)
	jra L6
L7:
L12:
	.stabd 68,0,58
	tstl a6@(-24)
	jeq L13
LBB3:
	.stabd 68,0,59
	addql #1,a6@(-16)
	.stabd 68,0,60
	movel a6@(8),a0
	tstl a0@(44)
	jeq L14
	pea 1:w
	pea _s81er$0
	jbsr _printf
	movel d0,d0
	addql #8,sp
L14:
LBE3:
L13:
	.stabd 68,0,63
	tstl a6@(-28)
	jeq L15
LBB4:
	.stabd 68,0,64
	addql #2,a6@(-16)
	.stabd 68,0,65
	movel a6@(8),a0
	tstl a0@(44)
	jeq L16
	pea 2:w
	pea _s81er$0
	jbsr _printf
	movel d0,d0
	addql #8,sp
L16:
LBE4:
L15:
	.stabd 68,0,68
	tstl a6@(-32)
	jeq L17
LBB5:
	.stabd 68,0,69
	addql #4,a6@(-16)
	.stabd 68,0,70
	movel a6@(8),a0
	tstl a0@(44)
	jeq L18
	pea 4:w
	pea _s81er$0
	jbsr _printf
	movel d0,d0
	addql #8,sp
L18:
LBE5:
L17:
	.stabd 68,0,76
	jbsr _regc
	movel d0,a6@(-12)
	.stabd 68,0,77
	movel a6@(8),a0
	tstl a0@(40)
	jeq L19
LBB6:
	.stabd 68,0,78
	tstl a6@(-12)
	jge L21
	pea LC0
	pea _badtest$2
	jbsr _printf
	movel d0,d0
	addql #8,sp
	jra L20
L21:
	.stabd 68,0,79
	pea LC0
	movel a6@(-12),sp@-
	pea _goodtest$3
	jbsr _printf
	movel d0,d0
	addw #12,sp
L20:
LBE6:
L19:
	.stabd 68,0,82
	jbsr _regp
	movel d0,a6@(-12)
	.stabd 68,0,83
	movel a6@(8),a0
	tstl a0@(40)
	jeq L22
LBB7:
	.stabd 68,0,84
	tstl a6@(-12)
	jge L24
	pea LC1
	pea _badtest$2
	jbsr _printf
	movel d0,d0
	addql #8,sp
	jra L23
L24:
	.stabd 68,0,85
	pea LC1
	movel a6@(-12),sp@-
	pea _goodtest$3
	jbsr _printf
	movel d0,d0
	addw #12,sp
L23:
LBE7:
L22:
	.stabd 68,0,88
	jbsr _regi
	movel d0,a6@(-12)
	.stabd 68,0,89
	movel a6@(8),a0
	tstl a0@(40)
	jeq L25
LBB8:
	.stabd 68,0,90
	tstl a6@(-12)
	jge L27
	pea LC2
	pea _badtest$2
	jbsr _printf
	movel d0,d0
	addql #8,sp
	jra L26
L27:
	.stabd 68,0,91
	pea LC2
	movel a6@(-12),sp@-
	pea _goodtest$3
	jbsr _printf
	movel d0,d5
	addw #12,sp
L26:
LBE8:
L25:
	.stabd 68,0,94
	movel a6@(-16),d0
	jra L1
LBE2:
L1:
	moveml a6@(-62),#0x7c
	unlk a6
	rts
.stabs "s81:F1",36,0,0,_s81
.stabs "pd0:p17=*18=xsdefs:",160,0,0,8
.stabs "defs:T18=s68cbits:1,0,32;ibits:1,32,32;\\",128,0,0,0
.stabs "sbits:1,64,32;lbits:1,96,32;ubits:1,128,32;\\",128,0,0,0
.stabs "fbits:1,160,32;dbits:1,192,32;fprec:6,224,32;\\",128,0,0,0
.stabs "dprec:6,256,32;flgs:1,288,32;flgm:1,320,32;\\",128,0,0,0
.stabs "flgd:1,352,32;flgl:1,384,32;rrc:1,416,32;\\",128,0,0,0
.stabs "crc:1,448,32;rfs:19=ar1;0;7;2,480,64;;",128,0,0,0
.stabs "s81er:v13",38,0,0,_s81er$0
.stabs "qs81:v14",38,0,0,_qs81$1
.stabs "ps:20=*2",128,0,0,-4
.stabs "pt:20",128,0,0,-8
.stabs "k:1",128,0,0,-12
.stabs "rc:1",128,0,0,-16
.stabs "j:1",128,0,0,-20
.stabs "crc:1",128,0,0,-24
.stabs "prc:1",128,0,0,-28
.stabs "irc:1",128,0,0,-32
.stabs "rchar:r2",64,0,0,2
.stabs "nrchar:2",128,0,0,-34
.stabs "rptr:r21=*1",64,0,0,3
.stabs "nrptr:21",128,0,0,-38
.stabs "rint:r1",64,0,0,4
.stabs "nrint:1",128,0,0,-42
.stabs "badtest:v15",38,0,0,_badtest$2
.stabs "goodtest:v16",38,0,0,_goodtest$3
.stabn 192,0,0,LBB2
.stabn 192,0,0,LBB3
.stabn 224,0,0,LBE3
.stabn 192,0,0,LBB4
.stabn 224,0,0,LBE4
.stabn 192,0,0,LBB5
.stabn 224,0,0,LBE5
.stabn 192,0,0,LBB6
.stabn 224,0,0,LBE6
.stabn 192,0,0,LBB7
.stabn 224,0,0,LBE7
.stabn 192,0,0,LBB8
.stabn 224,0,0,LBE8
.stabn 224,0,0,LBE2
.globl _regc
.text
	.even
_regc:
	.stabd 68,0,96
	link a6,#-386
	moveml #0x3f3c,sp@-
LBB9:
	.stabd 68,0,174
	clrb a6@(-2)
	.stabd 68,0,175
	moveb #1,a6@(-4)
	.stabd 68,0,176
	moveb #2,a6@(-6)
	.stabd 68,0,177
	moveb #3,a6@(-8)
	.stabd 68,0,178
	moveb #4,d2
	.stabd 68,0,179
	moveb #5,a6@(-10)
	.stabd 68,0,180
	moveb #6,d3
	.stabd 68,0,181
	moveb #7,a6@(-12)
	.stabd 68,0,182
	moveb #8,d4
	.stabd 68,0,183
	moveb #9,a6@(-14)
	.stabd 68,0,184
	moveb #10,d5
	.stabd 68,0,185
	moveb #11,a6@(-16)
	.stabd 68,0,186
	moveb #12,d6
	.stabd 68,0,187
	moveb #13,a6@(-18)
	.stabd 68,0,188
	moveb #14,d7
	.stabd 68,0,189
	moveb #15,a6@(-20)
	.stabd 68,0,190
	moveb #16,d0
	movew d0,a2
	.stabd 68,0,191
	moveb #17,a6@(-22)
	.stabd 68,0,192
	moveb #18,d0
	movew d0,a3
	.stabd 68,0,193
	moveb #19,a6@(-24)
	.stabd 68,0,194
	moveb #20,d0
	movew d0,a4
	.stabd 68,0,195
	moveb #21,a6@(-26)
	.stabd 68,0,196
	moveb #22,d0
	movew d0,a5
	.stabd 68,0,197
	moveb #23,a6@(-28)
	.stabd 68,0,198
	moveb #24,a6@(-156)
	.stabd 68,0,199
	moveb #25,a6@(-30)
	.stabd 68,0,200
	moveb #26,a6@(-158)
	.stabd 68,0,201
	moveb #27,a6@(-32)
	.stabd 68,0,202
	moveb #28,a6@(-160)
	.stabd 68,0,203
	moveb #29,a6@(-34)
	.stabd 68,0,204
	moveb #30,a6@(-162)
	.stabd 68,0,205
	moveb #31,a6@(-36)
	.stabd 68,0,206
	moveb #32,a6@(-164)
	.stabd 68,0,207
	moveb #33,a6@(-38)
	.stabd 68,0,208
	moveb #34,a6@(-166)
	.stabd 68,0,209
	moveb #35,a6@(-40)
	.stabd 68,0,210
	moveb #36,a6@(-42)
	.stabd 68,0,211
	moveb #37,a6@(-44)
	.stabd 68,0,212
	moveb #38,a6@(-46)
	.stabd 68,0,214
	movel a6,d0
	subql #4,d0
	movel d0,a6@(-170)
	movel a6,d0
	subql #2,d0
	movel d0,a6@(-174)
	movel a6@(-170),d0
	subl a6@(-174),d0
	movel d0,a6@(-154)
	.stabd 68,0,215
	movel a6,d0
	subql #6,d0
	movel d0,a6@(-178)
	movel a6,d0
	subql #4,d0
	movel d0,a6@(-182)
	movel a6@(-178),d0
	subl a6@(-182),d0
	movel d0,a6@(-150)
	.stabd 68,0,216
	movel a6,d0
	subql #8,d0
	movel d0,a6@(-186)
	movel a6,d0
	subql #6,d0
	movel d0,a6@(-190)
	movel a6@(-186),d0
	subl a6@(-190),d0
	movel d0,a6@(-146)
	.stabd 68,0,217
	moveq #-10,d0
	addl a6,d0
	movel d0,a6@(-194)
	movel a6,d0
	subql #8,d0
	movel d0,a6@(-198)
	movel a6@(-194),d0
	subl a6@(-198),d0
	movel d0,a6@(-142)
	.stabd 68,0,218
	moveq #-12,d0
	addl a6,d0
	movel d0,a6@(-202)
	moveq #-10,d0
	addl a6,d0
	movel d0,a6@(-206)
	movel a6@(-202),d0
	subl a6@(-206),d0
	movel d0,a6@(-138)
	.stabd 68,0,219
	moveq #-14,d0
	addl a6,d0
	movel d0,a6@(-210)
	moveq #-12,d0
	addl a6,d0
	movel d0,a6@(-214)
	movel a6@(-210),d0
	subl a6@(-214),d0
	movel d0,a6@(-134)
	.stabd 68,0,220
	moveq #-16,d0
	addl a6,d0
	movel d0,a6@(-218)
	moveq #-14,d0
	addl a6,d0
	movel d0,a6@(-222)
	movel a6@(-218),d0
	subl a6@(-222),d0
	movel d0,a6@(-130)
	.stabd 68,0,221
	moveq #-18,d0
	addl a6,d0
	movel d0,a6@(-226)
	moveq #-16,d0
	addl a6,d0
	movel d0,a6@(-230)
	movel a6@(-226),d0
	subl a6@(-230),d0
	movel d0,a6@(-126)
	.stabd 68,0,222
	moveq #-20,d0
	addl a6,d0
	movel d0,a6@(-234)
	moveq #-18,d0
	addl a6,d0
	movel d0,a6@(-238)
	movel a6@(-234),d0
	subl a6@(-238),d0
	movel d0,a6@(-122)
	.stabd 68,0,223
	moveq #-22,d0
	addl a6,d0
	movel d0,a6@(-242)
	moveq #-20,d0
	addl a6,d0
	movel d0,a6@(-246)
	movel a6@(-242),d0
	subl a6@(-246),d0
	movel d0,a6@(-118)
	.stabd 68,0,224
	moveq #-24,d0
	addl a6,d0
	movel d0,a6@(-250)
	moveq #-22,d0
	addl a6,d0
	movel d0,a6@(-254)
	movel a6@(-250),d0
	subl a6@(-254),d0
	movel d0,a6@(-114)
	.stabd 68,0,225
	moveq #-26,d0
	addl a6,d0
	movel d0,a6@(-258)
	moveq #-24,d0
	addl a6,d0
	movel d0,a6@(-262)
	movel a6@(-258),d0
	subl a6@(-262),d0
	movel d0,a6@(-110)
	.stabd 68,0,226
	moveq #-28,d0
	addl a6,d0
	movel d0,a6@(-266)
	moveq #-26,d0
	addl a6,d0
	movel d0,a6@(-270)
	movel a6@(-266),d0
	subl a6@(-270),d0
	movel d0,a6@(-106)
	.stabd 68,0,227
	moveq #-30,d0
	addl a6,d0
	movel d0,a6@(-274)
	moveq #-28,d0
	addl a6,d0
	movel d0,a6@(-278)
	movel a6@(-274),d0
	subl a6@(-278),d0
	movel d0,a6@(-102)
	.stabd 68,0,228
	moveq #-32,d0
	addl a6,d0
	movel d0,a6@(-282)
	moveq #-30,d0
	addl a6,d0
	movel d0,a6@(-286)
	movel a6@(-282),d0
	subl a6@(-286),d0
	movel d0,a6@(-98)
	.stabd 68,0,229
	moveq #-34,d0
	addl a6,d0
	movel d0,a6@(-290)
	moveq #-32,d0
	addl a6,d0
	movel d0,a6@(-294)
	movel a6@(-290),d0
	subl a6@(-294),d0
	movel d0,a6@(-94)
	.stabd 68,0,230
	moveq #-36,d0
	addl a6,d0
	movel d0,a6@(-298)
	moveq #-34,d0
	addl a6,d0
	movel d0,a6@(-302)
	movel a6@(-298),d0
	subl a6@(-302),d0
	movel d0,a6@(-90)
	.stabd 68,0,231
	moveq #-38,d0
	addl a6,d0
	movel d0,a6@(-306)
	moveq #-36,d0
	addl a6,d0
	movel d0,a6@(-310)
	movel a6@(-306),d0
	subl a6@(-310),d0
	movel d0,a6@(-86)
	.stabd 68,0,232
	moveq #-40,d0
	addl a6,d0
	movel d0,a6@(-314)
	moveq #-38,d0
	addl a6,d0
	movel d0,a6@(-318)
	movel a6@(-314),d0
	subl a6@(-318),d0
	movel d0,a6@(-82)
	.stabd 68,0,233
	moveq #-42,d0
	addl a6,d0
	movel d0,a6@(-322)
	moveq #-40,d0
	addl a6,d0
	movel d0,a6@(-326)
	movel a6@(-322),d0
	subl a6@(-326),d0
	movel d0,a6@(-78)
	.stabd 68,0,234
	moveq #-44,d0
	addl a6,d0
	movel d0,a6@(-330)
	moveq #-42,d0
	addl a6,d0
	movel d0,a6@(-334)
	movel a6@(-330),d0
	subl a6@(-334),d0
	movel d0,a6@(-74)
	.stabd 68,0,235
	moveq #-46,d0
	addl a6,d0
	movel d0,a6@(-338)
	moveq #-44,d0
	addl a6,d0
	movel d0,a6@(-342)
	movel a6@(-338),d0
	subl a6@(-342),d0
	movel d0,a6@(-70)
	.stabd 68,0,244
	movel a6@(-154),a6@(-54)
	.stabd 68,0,245
	moveq #1,d0
	movel d0,a6@(-50)
	.stabd 68,0,247
	clrl a6@(-66)
L29:
	moveq #22,d0
	cmpl a6@(-66),d0
	jle L30
	.stabd 68,0,248
	movel a6@(-50),a6@(-346)
	moveq #3,d0
	cmpl a6@(-346),d0
	jeq L33
	moveq #2,d0
	cmpl a6@(-346),d0
	jeq L34
	moveq #1,d0
	cmpl a6@(-346),d0
	jeq L35
	jra L32
	.stabd 68,0,249
L35:
	movel a6@(-66),a6@(-350)
