	.ifndef _STACK_INC_
	_STACK_INC_ = 1

	; Push 16bit to data stack
	; If value is provided, value-low,value-high is pushed onto the data stack
	; If value is not provided, a,00 is pushed
	;
	.macro  push value
		.ifnblank value
			lda		#>value		;
		.endif
			dex					;
		.ifnblank value
			sta		0,x			;
			lda		#<value		;
		.else
			stz		0,x			;
		.endif
			dex					;
			sta		0,x	        ;
	.endmacro

	; Push 16bit zero to data stack
	;
	.macro	push0
			dex
			stz		0,x
			dex
			stz		0,x
	.endmacro

	; Pull 16bit from data stack
	;
	.macro	pull
			inx		;
			inx		;
	.endmacro

	.endif
