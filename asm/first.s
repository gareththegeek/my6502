	; vasm -Fbin -dotdir xxx.s
	.org $ff00
halt:
	jmp halt

	.org $fffc
	.word halt
	.word $0000