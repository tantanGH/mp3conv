PUSH		MACRO	R0
		MOVEM.L	R0,-(SP)
		ENDM
POP		MACRO	R0
		MOVEM.L	(SP)+,R0
		ENDM
push		MACRO	R0
		MOVEM.L	R0,-(SP)
		ENDM
pop		MACRO	R0
		MOVEM.L	(SP)+,R0
		ENDM

SUPER		MACRO
		PUSH	D0-A6
		MOVE.L	#$81,D0
		MOVE.L	#0,D1
		MOVEA.L	D1,A1
		TRAP	#$0F
		POP	D0-A6
		ENDM

PRINT		MACRO	R0
		PUSH	D0-D7/A0-A6
		PEA	R0
		DC.W	__PRINT
		ADDQ.L	#4,SP
		POP	D0-D7/A0-A6
		ENDM

		* PCM->ADPCM変換のマクロ

onef		macro	RD,RA,RD2
		move.w	(a1)+,d5
		sub.w	RD,d5
		bpl.w	@f
		neg.w	d5
		or.b	#$8,RD2
	@@:	cmp.w	(RA),d5		* 4/8
		bcc.w	1f
		subq.w	#4,RA
		bra	@f
	1:	addq.w	#4,RA
	@@:	cmp.w	(RA)+,d5
		bcc.w	@f
		subq.w	#4,RA
	@@:	cmp.w	(RA),d5
		bcs	@f
		addq.w	#2,RA
	@@:	addq.w	#8,RA
		addq.w	#8,RA
		btst	#3,RD2
		bne	2f		* minus
		add.w	(RA),RD
		bvc	3f

		sub.w	(RA),RD
		move.w	32(RA),D5
		beq	@f
		add.w	-(RA),RD
		bra	3f
	@@:	or.b	#$8,RD2
		add.w	16(RA),RD
		bra	3f

	2:	addq.w	#8,RA
		addq.w	#8,RA
		add.w	(RA),RD
		bvc	1f

		sub.w	(RA),RD
		move.w	16(RA),D5
		beq	@f
		add.w	-(RA),RD
		bra	1f
	@@:	and.b	#$F7,RD2
		add.w	-16(RA),RD
		bra	1f
	3:	addq.w	#8,RA
		addq.w	#8,RA

	1:	addq.w	#8,RA
		addq.w	#8,RA
		or.w	(RA),RD2
		addq.w	#8,RA
		addq.w	#8,RA
		add.w	(RA),RA
		endm
