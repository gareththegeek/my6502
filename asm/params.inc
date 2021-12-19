    .ifndef _PARAMS_INC_
    _PARAMS_INC_ = 1

    ; Data Push Pointer (dphp)
    ; Pass the specified pointer to a subroutine by pushing to the data stack
    ; The address will be accessible in the called subroutine using e.g. lda (n,x)
    ;  where n is the parameter position
    ;
    .macro  dphp addr
            lda #>addr		;
            dex				;
            sta 0,x			;
            lda #<addr		;
            dex				;
            sta 0,x	        ;
    .endmacro

    ; Data Pull Pointer (dplp)
    ; Remove a pointer from the data stack following a call to a subroutine
    ;
    .macro  dplp
            inx             ;
            inx             ;
    .endmacro

    ; Data Push A (dpha)
    ; Push value in A register to subroutine via data stack
    ; The value will be accessible to the called subroutine using e.g. lda n,x
    ;  where n is the parameter position
    .macro  dpha
            dex				; pass contents of a register
	        sta 0,x			;  via data stack
    .endmacro

    ; Data Pull A (dpla)
    ; Remove a byte value from the data stack following a call to a subroutine
    ;
    .macro  dpla
            inx             ;
    .endmacro

    .endif
