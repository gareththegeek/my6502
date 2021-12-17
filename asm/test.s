	; vasm -Fbin -dotdir xxx.s

VIA		=	$9800
PORTB	=	VIA+$0
PORTA	=	VIA+$1
DDRB	=	VIA+$2
DDRA	=	VIA+$3
T1CL	=	VIA+$4
T1CH	=	VIA+$5
T1LL	=	VIA+$6
T1LH	=	VIA+$7
T2CL	=	VIA+$8
T2CH	=	VIA+$9
SR		=	VIA+$a
ACR		=	VIA+$b
PCR		=	VIA+$c
IFR		=	VIA+$d
IER		=	VIA+$e
PORTAA	=	VIA+$f

	.segment "CODE"

start:
	ldx #$fe		;
	txs				;
	lda PORTA


halt:
	jmp halt

	.segment "VECTORS"
	.word $0000
	.word start
	.word $0000