
	.include "params.inc"
	.include "via.inc"

	.export led_init
	.export led_print

E  = %10000000
RW = %01000000
RS = %00100000

	.code

led_command:
	sta PORTB
	lda #0         ; Clear RS/RW/E bits
	sta PORTA
	lda #E         ; Set E bit to send instruction
	sta PORTA
	lda #0         ; Clear RS/RW/E bits
	sta PORTA
	rts


led_init:
	lda #%11111111 ; Set all pins on port B to output
	sta DDRB

	lda #%11100000 ; Set top 3 pins on port A to output
	sta DDRA

	lda #%00111000 ; Set 8-bit mode; 2-line display; 5x8 font
	jsr led_command

	lda #%00001110 ; Display on; cursor on; blink off
	jsr led_command

	lda #%00000110 ; Increment and shift cursor; don't shift display
	jsr led_command

	rts


; Print character to LED display
; Parameters:
;	x+0: Character to print
led_char:
	lda 0,x			; get character from data stack
	sta PORTB		; and send to led display
	lda #RS         ; Set RS; Clear RW/E bits
	sta PORTA
	lda #(RS | E)   ; Set E bit to send instruction
	sta PORTA
	lda #RS         ; Clear E bits
	sta PORTA

	rts


; Print string to LED display
; Parameters:
;	x+0,x+1: Pointer to null-terminated string
led_print:
@loop:
	lda (0,x)		; load character using data stack [x, x+1] as pointer to character
	beq @end		; exit loop when we reach null character (a == 0)
	
	dpha			; push a to data stack
	jsr led_char	; print character
	dpla			; pop character from data stack
	
	inc 0,x			; increment string pointer low byte
	bne @loop		; check for a carry
	inc 1,x			; carry therefore increment high byte
	jmp @loop		;
@end:
	rts
