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

	; push $1234
	; push $56
	; jsr div16
	; jsr led_print_hex	; 0010
	; push $20
	; jsr led_print_char	; space
	; jsr led_print_hex	; 0036
	; push $20
	; jsr led_print_char	; space
	push $37
	push $12
	jsr div8
	jsr led_print_hex	;
	jsr led_print_hex	;


	; push $1234
	; jsr led_print_hex

	; push $20
	; jsr led_print_char

	; ;push msg			; Pass message string pointer via data stack
	; ;jsr led_print_str	;

	; push $12			; 0012
	; jsr led_print_hex	;

	; push $2a			; *
	; jsr led_print_char	;

	; push $56			; 0056
	; jsr led_print_hex	;

	; push $3d			; =
	; jsr led_print_char	;

	; push $12
	; push $56
	; jsr mul8
	; jsr led_print_hex	; 060C

	; push $1234
	; push $5678
	; jsr mul16
	; swap2
	; jsr led_print_hex	; 0626
	; jsr led_print_hex	; 0060

	; ; Test 16-bit multiply
	; push $1111
	; push $2222
	; jsr mul16
	; swap2
	; jsr led_print_hex	; 0246
	; jsr led_print_hex	; 8642

halt:
	jmp halt


	.segment "VECTORS"
	.word $0000
	.word start
	.word $0000
