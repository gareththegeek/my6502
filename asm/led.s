
	.include "stack.inc"
	.include "via.inc"

	.export led_init
	.export led_print_char
	.export led_print_hex
	.export led_print_str

E  		= %10000000
RW 		= %01000000
RS 		= %00100000

ASC0	= $30
ASCCOLON= $3A
ASCA	= $41
ASC0toA = ASCA-ASCCOLON

	.code

; Execute LED display command stored in A
;	( -- )
;
.proc led_command
	sta PORTB
	lda #0         ; Clear RS/RW/E bits
	sta PORTA
	lda #E         ; Set E bit to send instruction
	sta PORTA
	lda #0         ; Clear RS/RW/E bits
	sta PORTA
	rts
.endproc


; Initialise LED display
;	( -- )
;
.proc led_init
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
.endproc


; Print character to LED display
;	( char -- )
;
.proc led_print_char
	char	= 0

	lda char,x		; get character from data stack
	sta PORTB		; and send to led display
	lda #RS         ; Set RS; Clear RW/E bits
	sta PORTA
	lda #(RS | E)   ; Set E bit to send instruction
	sta PORTA
	lda #RS         ; Clear E bits
	sta PORTA

	pull
	rts
.endproc


; Print 16 bit hexadecimal value to LED display
; 	( value -- )
;
.proc led_print_hex
	value 	= 0

	.macro asc
	clc
	adc		#ASC0		; Ascii '0'
	clc
	cmp		#ASCCOLON	; Check if greater than '9'
	bcc		:+
	clc
	adc		#ASC0toA	; Make character begin from 'A'
:
	.endmacro

	txa					; Begin with the high byte
	tay
	iny
@loop:
	lda		value,y		; Get value to print
	lsr					; Get high nybble
	lsr
	lsr
	lsr
	asc					; Convert to ascii
	push				; Pass high nybble ascii
	jsr	led_print_char	; Print high nybble
	
	lda		value,y		; Get value to print 
	and		#$0f		; Get low nybble
	asc					; Convert to ascii
	push				; Pass low nybble ascii
	jsr	led_print_char	; Print low nybble

	tya
	lsr
	bcc		@end		; if y is odd this is the
	dey					;	high byte so decrement
	jmp		@loop		; 	and loop
@end:
	
	pull
	rts
.endproc


; Print string to LED display
;	( lpMessage hpMessage -- )
;
.proc led_print_str
	plmsg	= 0
	phmsg	= 1
@loop:
	lda (plmsg,x)		; load character using data stack [x, x+1] as pointer to character
	beq @end			; exit loop when we reach null character (a == 0)

	push				; push a to data stack
	jsr led_print_char	; print character
	
	inc plmsg,x			; increment string pointer low byte
	bne @loop			; check for a carry
	inc phmsg,x			; carry therefore increment high byte
	jmp @loop			;
@end:
	pull
	rts
.endproc
