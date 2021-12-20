	; cl65 -o bin/rom.bin -C 6502.cfg -t bbc --cpu 65c02 --no-target-lib asm/main.s asm/led.s asm/maths.s
	; npm start --prefix nodeprog COM6 ../bin/rom.bin f000

	.include "stack.inc"
	.include "led.inc"
	.include "maths.inc"

	.code

msg:	.asciiz "Hello World"

start:
	ldx #$fe		; Return stack pointer
	txs				;
	ldx #$00		; Data stack pointer
	
	jsr led_init	;

	;push msg			; Pass message string pointer via data stack
	;jsr led_print_str	;

	push $34			; 34
	jsr led_print_hex	;

	push $2a			; *
	jsr led_print_char	;

	push $5b			; 5B
	jsr led_print_hex	;

	push $3d			; =
	jsr led_print_char	;

	push $34
	push $5b
	jsr mul8
	jsr led_print_hex	; 12
	jsr led_print_hex	; 7C

halt:
	jmp halt


	.segment "VECTORS"
	.word $0000
	.word start
	.word $0000
