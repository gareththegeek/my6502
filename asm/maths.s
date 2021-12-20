
    .export mul8

    .code


; Multiply two 8bit numbers to produce a 16bit result
;   ( opA opB -- lRES hRES)
;
.proc mul8
    opA = 0
    opB = 2

    lda #0          ;
    ldy #8          ;
    lsr opB,x       ;
@loop:
    bcc @skip       ;
    clc
    adc opA,x       ;
@skip:
    ror             ;
    ror opB,x       ;
    dey             ;
    bne @loop       ;
    sta opA,x       ;

    rts
.endproc
