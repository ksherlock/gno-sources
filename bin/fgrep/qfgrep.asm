          keep  Qfgrep
          mcopy Qfgrep.Macros
          case  ON
*         gen   ON
*         list  ON
*         expand ON


**************************************************************
*
*    Quick fixed grep utility. Optimized to search quickly
*    for a case-insensitive match.
*
**************************************************************


bufflen   gequ  $8000
cr        gequ  $d
lf        gequ  $a
maxolen   gequ  500

DataBlock privdata
override  dc    i'0'
prevcret  dc    i'bufflen|-1-1'
strlength dc    i'0'
fnum      dc    i'0'
flptr     dc    i4'0'
          end


qinit     start
          using DataBlock

          CSUB  (2:len),0

          phb
          phk
          plb
          lda   #bufflen|-1-1
          sta   prevcret
          stz   override
          lda   len
          sta   strlength
          plb
          RET
          end


nsqinit   start
          using DataBlock

          CSUB  (2:numstr,4:lenptr),0

          phb
          phk
          plb
          lda   numstr
          sta   fnum
          lda   lenptr
          sta   flptr
          lda   lenptr+2
          sta   flptr+2
          lda   #bufflen|-1-1
          sta   prevcret
          stz   override
          plb
          RET
          end


qfgrep    start
          using DataBlock

notfound  equ   -1
abortval  equ   -2
invert    equ   $4
exact     equ   $2
lastcret  equ   $1
chkoffset equ   $3
invflag   equ   $5

          CSUB  (2:stat,4:bufptr,2:uplim,4:lcnt,4:str1,4:str2,4:ostrptr),6

          LONG  I,M
          phb
          phk
          plb
          lda   override                 ; Check for premature abort
          beq   mainproc
          stz   override
          brl   mvret                    ; Ask for next buffer
mainproc  lda   stat
          and   #invert
          sta   invflag
          lda   stat
          and   #exact
          bne   mp2
          SHORT M
          lda   #spos-spbra-2
          xba
          lda   #$80                     ; The hex code for BRA opcode
          LONG  M
          sta   spbra
          bra   mp3
mp2       lda   #$EAEA                   ; Two NOPs
          sta   spbra
mp3       lda   bufptr
          sta   lplac+1
          sta   spos+1
          sta   flshld+1
          SHORT M
          lda   bufptr+2
          sta   lplac+3
          sta   spos+3
          sta   flshld+3
          ldy   uplim
          cpy   #bufflen
          beq   findlst
          dey
          lda   [bufptr],Y
          cmp   #cr
          beq   fix2
          cmp   #lf
          beq   fix2
          iny
          lda   #cr
          sta   [bufptr],Y
fix2      sty   lastcret
          cpy   prevcret
          beq   abort
          bra   setchk
findlst   dey
          cpy   prevcret
          bpl   find3
abort     LONG  M
          plb
          RET   2:#abortval
find3     LONGA OFF
          lda   [bufptr],Y
          cmp   #cr
          beq   foundit
          cmp   #lf
          bne   findlst
foundit   sty   lastcret
setchk    ldx   prevcret
          inx
          stx   chkoffset
          lda   [str1]
          sta   C1+1
          lda   [str2]
          sta   C2+1
spos      lda   >000000,X
          cmp   #cr
          beq   jnextlin
          cmp   #lf
          beq   jnextlin
C1        cmp   #0
          beq   srchstr
C2        cmp   #0
          beq   srchstr
          inx
spbra     bra   spos
          dex
          jsr   flushret
jnextlin  brl   nextline
srch3     ldx   chkoffset
          inx
          bra   spos
srchstr   stx   chkoffset
          inx
          ldy   #1
sstr2     cpy   strlength
          beq   foundone
lplac     lda   >000000,X
          cmp   [str1],Y
          bne   sstr3
          iny
          inx
          bra   sstr2
sstr3     cmp   [str2],Y
          bne   srch3
          iny
          inx
          bra   sstr2
foundone  dex
          jsr   flushret
          txy
          lda   #0
          sta   [bufptr],Y
          LONG  M
          lda   stat
          and   #exact
          beq   fone4
          txa
          clc                            ; Yes, I do mean CLEAR the carry flag
          sbc   prevcret
          cmp   strlength
          bne   nextline
          lda   invflag
          bne   nextln2
fone4     lda   [lcnt]
          inc   A
          sta   [lcnt]
          bne   fone7
          ldy   #2
          lda   [lcnt],Y
          inc   A
          sta   [lcnt],Y
fone7     cpx   lastcret
          bne   fone3
          inc   override
fone3     phx
          ldx   prevcret
          inx
          clc
          txa
          adc   bufptr
          sta   [ostrptr]
          lda   bufptr+2
          adc   #0
          ldy   #2
          sta   [ostrptr],Y
          plx
          lda   prevcret
          sta   chkoffset
          stx   prevcret
          plb
          RET   2:chkoffset
nextline  LONG  M
          lda   invflag
          bne   fone4
nextln2   lda   [lcnt]
          inc   A
          sta   [lcnt]
          bne   nxt3
          ldy   #2
          lda   [lcnt],Y
          inc   A
          sta   [lcnt],Y
nxt3      stx   chkoffset
          stx   prevcret
          cpx   lastcret
          beq   mvret
          inx
          SHORT M
          brl   spos
mvret     LONGA ON
          phb
          lda   bufptr
          clc
          adc   #bufflen|-1
          tax
          SHORT M
          lda   bufptr+2
          xba
          lda   bufptr+2
          adc   #0
          LONG  M
          sta   mvinstr+1
          ldy   bufptr
          lda   #bufflen|-1-1
mvinstr   mvn   00,00
          plb
          lda   prevcret
          sec
          sbc   #bufflen|-1
          sta   prevcret
          plb
          RET   2:#notfound
          end


nsqfgrep  start
          using DataBlock

notfound  equ   -1
abortval  equ   -2
invert    equ   $4
exact     equ   $2
lastcret  equ   $1
chkoffset equ   $3
invflag   equ   $5
str1      equ   $7
str2      equ   $b
flens     equ   $f
snum      equ   $13


          CSUB  (2:stat,4:bufptr,2:uplim,4:lcnt,4:pstr1,4:pstr2,4:ostrptr),20

          LONG  I,M
          phb
          phk
          plb
          lda   override                 ; Check for premature abort
          beq   mainproc
          stz   override
          brl   mvret                    ; Ask for next buffer
mainproc  lda   stat
          and   #invert
          sta   invflag
          lda   stat
          and   #exact
          bne   mp2
          SHORT M
          lda   #spos-spbra-2
          xba
          lda   #$80                     ; The hex code for BRA opcode
          LONG  M
          sta   spbra
          bra   mp3
mp2       lda   #$EAEA                   ; Two NOPs
          sta   spbra
mp3       lda   bufptr
          sta   lplac+1
          sta   spos+1
          sta   flshld+1
          SHORT M
          lda   bufptr+2
          sta   lplac+3
          sta   spos+3
          sta   flshld+3
          ldy   uplim
          cpy   #bufflen
          beq   findlst
          dey
          lda   [bufptr],Y
          cmp   #cr
          beq   fix2
          cmp   #lf
          beq   fix2
          iny
          lda   #cr
          sta   [bufptr],Y
fix2      sty   lastcret
          cpy   prevcret
          beq   abort
          bra   setchk
findlst   dey
          cpy   prevcret
          bpl   find3
abort     LONG  M
          plb
          RET   2:#abortval
find3     LONGA OFF
          lda   [bufptr],Y
          cmp   #cr
          beq   foundit
          cmp   #lf
          bne   findlst
foundit   sty   lastcret
setchk    LONG  M
          ldx   prevcret
          inx
          stx   chkoffset
          lda   flptr
          sta   flens
          lda   flptr+2
          sta   flens+2
          lda   [flens]
          sta   strlength
          lda   [pstr1]
          sta   str1
          lda   [pstr2]
          sta   str2
          ldy   #2
          lda   [pstr1],Y
          sta   str1+2
          lda   [pstr2],Y
          sta   str2+2
          stz   snum
spos2     SHORT M
          lda   [str1]
          sta   C1+1
          lda   [str2]
          sta   C2+1
spos      lda   >000000,X
          cmp   #cr
          beq   nextstr
          cmp   #lf
          beq   nextstr
C1        cmp   #0
          beq   srchstr
C2        cmp   #0
          beq   srchstr
          inx
spbra     bra   spos
          lda   snum
          inc   A
          cmp   fnum
          bne   nextstr2
          dex
          jsr   flushret
          brl   nextline
nextstr   LONG  M
          lda   snum
          inc   A
          cmp   fnum
          bne   nextstr2
          brl   nextline
nextstr2  sta   snum
          rol   A
          tay
          lda   [flens],Y
          sta   strlength
          tya
          rol   A
          tay
          lda   [pstr1],Y
          sta   str1
          lda   [pstr2],Y
          sta   str2
          iny
          iny
          lda   [pstr1],Y
          sta   str1+2
          lda   [pstr2],Y
          sta   str2+2
          ldx   prevcret
          inx
          bra   spos2
srch3     LONGA OFF
          ldx   chkoffset
          inx
          bra   spos
srchstr   stx   chkoffset
          inx
          ldy   #1
sstr2     cpy   strlength
          beq   foundone
lplac     lda   >000000,X
          cmp   [str1],Y
          bne   sstr3
          iny
          inx
          bra   sstr2
sstr3     cmp   [str2],Y
          bne   srch3
          iny
          inx
          bra   sstr2
foundone  dex
          jsr   flushret
          txy
          lda   #0
          sta   [bufptr],Y
          LONG  M
          lda   stat
          and   #exact
          beq   fone4
          txa
          clc                            ; Yes, I do mean CLEAR the carry flag
          sbc   prevcret
          cmp   strlength
          bne   nextstr
          lda   invflag
          bne   nextln2
fone4     lda   [lcnt]
          inc   A
          sta   [lcnt]
          bne   fone7
          ldy   #2
          lda   [lcnt],Y
          inc   A
          sta   [lcnt],Y
fone7     cpx   lastcret
          bne   fone3
          inc   override
fone3     phx
          ldx   prevcret
          inx
          clc
          txa
          adc   bufptr
          sta   [ostrptr]
          lda   bufptr+2
          adc   #0
          ldy   #2
          sta   [ostrptr],Y
          plx
          lda   prevcret
          sta   chkoffset
          stx   prevcret
          plb
          RET   2:chkoffset
nextline  LONG  M
          lda   invflag
          bne   fone4
nextln2   lda   [lcnt]
          inc   A
          sta   [lcnt]
          bne   nxt3
          ldy   #2
          lda   [lcnt],Y
          inc   A
          sta   [lcnt],Y
nxt3      stx   chkoffset
          stx   prevcret
          cpx   lastcret
          beq   mvret
          SHORT M
          brl   setchk
mvret     LONGA ON
          phb
          lda   bufptr
          clc
          adc   #bufflen|-1
          tax
          SHORT M
          lda   bufptr+2
          xba
          lda   bufptr+2
          adc   #0
          LONG  M
          sta   mvinstr+1
          ldy   bufptr
          lda   #bufflen|-1-1
mvinstr   mvn   00,00
          plb
          lda   prevcret
          sec
          sbc   #bufflen|-1
          sta   prevcret
          plb
          RET   2:#notfound
          end



flushret  private

          LONGA OFF
flsh1     inx
flshld    entry
          lda   >000000,X
          cmp   #cr
          beq   flsh2
          cmp   #lf
          bne   flsh1
flsh2     rts
          end
