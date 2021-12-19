	; cl65 -o bin/rom.bin -C 6502.cfg -t bbc --cpu 65c02 --no-target-lib asm/main.s asm/led.s

	.include "params.inc"
	.include "led.inc"

	.code

msg:	.asciiz "Hello Params"

start:
	ldx #$fe		; Return stack pointer
	txs				;
	ldx #$00		; Data stack pointer
	
	jsr led_init	;

	dphp msg		; Pass message string via data stack
	jsr led_print	;
	dplp


halt:
	jmp halt


	.segment "VECTORS"
	.word $0000
	.word start
	.word $0000
