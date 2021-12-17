	; cl65 -o bin/rom.bin -C 6502.cfg -t bbc --cpu 65c02 --no-target-lib asm/main.s asm/led.s

	.include "led.inc"

	.code


start:
	ldx #$fe		;
	txs				;
	
	jsr led_init	;
	jsr led_print	;


halt:
	jmp halt


	.segment "VECTORS"
	.word $0000
	.word start
	.word $0000
