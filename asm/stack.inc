	.ifndef _STACK_INC_
	_STACK_INC_ = 1

	;_CMOS_ = 1

	; Push 16-bit to data stack
	;
	; If value is provided
	;	( -- value )
	;
	; If value is not provided
	;	( -- A )
	;
	.macro  push value
			dex					;
		.ifnblank value
			lda		#>value		;
			sta		0,x			;
			lda		#<value		;
		.else
			stz		0,x			;
		.endif
			dex					;
			sta		0,x	        ;
	.endmacro

	; Push 16-bit zero to data stack
	;	( -- 0 )
	;
	.macro	push0
			dex
			.ifdef _CMOS_
			stz		0,x			; this doesn't work on my bogus r65c02 which turns out to be a 6502 (nmos)
			.else
			lda		#0			;
			sta		0,x			;
			.endif
			dex
			.ifdef _CMOS_
			stz		0,x
			.else
			sta		0,x
			.endif
	.endmacro

	; Pull 16-bit from data stack
	;	( x -- )
	;
	.macro	pull
			inx		;
			inx		;
	.endmacro

	; Swap the top two 16-bit values of the data stack around
	;	( x y -- y x )
	;
	.macro 	swap2
			lda		0,x		;
			ldy 	2,x		;
			sta		2,x		;
			sty		0,x		;
			lda		1,x		;
			ldy		3,x		;
			sta		3,x		;
			sty		1,x		;
	.endmacro

	.endif
