
    .include "stack.inc"

    .export mul8
    .export mul16

    .code


; Multiply two 8-bit numbers to produce a 16-bit result
;   ( opA opB -- result )
;
.proc mul8
    opA0    = 0     ; A parameter 8-bit -low
    opA1    = 1     ; empty
    opB0    = 2     ; B parameter 8-bit -low
    opB1    = 3     ; empty

    res0    = 2     ; Result 16-bit     -low
    res1    = 3     ;                   -high

    lda #0          ;
    ldy #8          ;
    lsr opB0,x      ;
@loop:
    bcc @skip       ;
    clc
    adc opA0,x      ;
@skip:
    ror             ;
    ror opB0,x      ;
    dey             ;
    bne @loop       ;
    sta res1,x      ;

    pull
    rts
.endproc

; Multiply two 16-bit numbers to produce a 32-bit result
;   ( opA opB -- highResult lowResult )
;
.proc mul16
    opA0    = 2     ; A parameter   -low
    opA1    = 3     ; 16-bit        -high
    opB0    = 4     ; B parameter   -low
    opB1    = 5     ; 16-bit        -high

    m0      = opA0  ; Multiplier    -low
    m1      = opA1  ; 16-bit        -high
    res0    = 0     ; Result        -lowest
    res1    = 1     ; 32-bit        -lower
    res2    = opB0  ;               -higher
    res3    = opB1  ;               -highest

    push0       ; ( b a -- b a 0 ) -> ( m resl resh )
    
    ldy #16     ;
@loop:
    asl res0,x  ; Shift 32-bit result over one bit position
    rol res1,x  ;
    rol res2,x  ;
    rol res3,x  ;
    bcc @skip   ; Skip add if high bit shifted out is 0
    clc
    lda m0,x    ; Otherwise add multiplier
    adc res0,x  ;
    sta res0,x  ;
    lda m1,x    ;
    adc res1,x  ;
    sta res1,x  ;

    lda #0      ; Carry into lower of the 2 high bytes
    adc res2,x  ;
    sta res2,x  ;
@skip:
    dey         ;
    bne @loop   ;

    lda res0,x  ; ( m resl resh -- resh resl )
    sta m0,x    ;
    lda res1,x  ;
    sta m1,x    ;
    pull        ;

    rts
.endproc
