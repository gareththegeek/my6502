
    .ifndef _VIA_INC_
    _VIA_INC_ = 1

    VIA		=	$8800
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

    .endif