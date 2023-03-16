
		include	iocscall.mac
		include	doscall.mac
		include	ym2608_adpcmlib.mac

		.xdef	ptoa_make_buffer
		.xdef	ptoa_init
		.xdef	ptoa_exec
		.xdef	atop_make_buffer
		.xdef	atop_init
		.xdef	atop_exec
		.xdef	ad_set_panpot	* 以下、追加分のコール
		.xdef	atop_mem
		.xdef	atop_set
		.xdef	atop_null_exec

		.offset	0
free_head:
stereo:		dc.l	0		* ステレオのファイルの場合、−１になります。
cnva_add:	dc.l	0		* PCM変換用バッファの格納アドレス
					* 2048*(bufx+1)バイトの広さが必要
pcma_add:	dc.l	0		* pcm格納アドレス
ada_add:	dc.l	0		* adpcmの格納アドレス
x:		dc.l	0		* 以下、計算課程の値
y:		dc.l	0
rx:		dc.l	0
ry:		dc.l	0
lx:		dc.l	0
ly:		dc.l	0
x1:		dc.l	0
rx1:		dc.l	0
lx1:		dc.l	0
ra:		dc.l	0
la:		dc.l	0
rback:		dc.l	0
lback:		dc.l	0
back:		dc.l	0

free_tail:
		.data
TITLE:		.dc.b	'adpcmlib.a ver0.02 (C)1995 Otankonas',0

		.quad
		.text

bufx		EQU	68

*****************************************************

ad_set_panpot:
	* ステレオかモノラルかの指定をする : d0
	push	a6
	lea	work_area,a6
	move.w	d0,stereo(a6)
	pop	a6
	rts

	* 現在のADPCM用レジスタの値を得る
	* 記録用レジスタには、a0を使用する

atop_mem:
	push	a0/a6
	lea	work_area,a6
	tst.w	stereo(a6)
	bne	@f

	* monauralの場合

	move.l	back(a6),(a0)+
	move.l	x1(a6),(a0)+
	pop	a0/a6
	rts
@@:
	* stereoの場合

	move.l	rback(a6),(a0)+
	move.l	lback(a6),(a0)+
	move.l	rx1(a6),(a0)+
	move.l	lx1(a6),(a0)+
	pop	a0/a6
	rts

	* ADPCM用レジスタに値をセットする
	* 記録用レジスタには、a0を使用する

atop_set:
	push	a0/a6
	lea	work_area,a6
	tst.w	stereo(a6)
	bne	@f

	* monauralの場合

	move.l	(a0)+,back(a6)
	move.l	(a0)+,x1(a6)
	pop	a0/a6
	rts
@@:
	* stereoの場合

	move.l	(a0)+,rback(a6)
	move.l	(a0)+,lback(a6)
	move.l	(a0)+,rx1(a6)
	move.l	(a0)+,lx1(a6)

	pop	a0/a6
	rts

atop_null_exec:
	* PCM->ADPCM変換を行うふりをする
	* 引数 : 変換するバイト数 d0
	*	 読み込むADPCMのバッファのアドレス a0
	push	d0-d4/a0-a2
	lea	work_area,a6
	move.l	a0,ada_add(a6)
	tst.w	stereo(a6)
	bne	@f
	bsr	conv_monon
	pop	d0-d4/a0-a2
	rts
@@:	bsr	conv_stereon
	pop	d0-d4/a0-a2
	rts

*****************************************************

ptoa_make_buffer:
	* 全ての初期化をする。変換用のバッファを作る。
	* これをコールしないと、変換がうまくいかない

	push	d0-a6
	lea	work_area,a6
	clr.w	stereo(a6)		* 初期値はモノラル
	clr.l	x(a6)
	clr.l	y(a6)
	clr.l	rx(a6)
	clr.l	ry(a6)
	clr.l	lx(a6)
	clr.l	ly(a6)
	move.l	#BUFFER+6,ra(a6)
	move.l	#BUFFER+6,la(a6)
	bsr	MAKE_BUFFER		* バッファをつくる
	pop	d0-a6
	rts

ptoa_init:
	* PCM->ADPCM変換をする前にコールしておく
	* 継続用バッファをフラッシュする
	* ステレオかモノラルかの指定をする : d0

	push	a6
	lea	work_area,a6
	move.w	d0,stereo(a6)
	clr.l	x(a6)
	clr.l	y(a6)
	clr.l	rx(a6)
	clr.l	ry(a6)
	clr.l	lx(a6)
	clr.l	ly(a6)
	move.l	#BUFFER+6,ra(a6)
	move.l	#BUFFER+6,la(a6)
	pop	a6
	rts

ptoa_exec:
	* PCM->ADPCM変換を行う
	* 引数 : 変換するバイト数 d0
	*	 読み込むPCMのバッファのアドレス a0
	*	 書き込むADPCMのバッファのアドレス a1
	push	d0-a6
	lea	work_area,a6
	move.l	a0,pcma_add(a6)
	move.l	a1,ada_add(a6)
	tst.w	stereo(a6)
	bne	@f
	bsr	conv_monob
	pop	d0-a6
	rts
@@:	bsr	conv_stereob
	pop	d0-a6
	rts


atop_make_buffer:
	* 全ての初期化をする。変換用のバッファを作る。
	* これをコールしないと、変換がうまくいかない
	* 変換バッファのアドレスを指定すること : a0

	push	d0-a6
	lea	work_area,a6
	move.l	a0,cnva_add(a6)		* 変換バッファのアドレス
*	clr.w	stereo			* 初期値はモノラル
  clr.w stereo(a6)  * debug by tantan 2023/02/23
	move.l	cnva_add(a6),x1(a6)
	move.l	cnva_add(a6),lx1(a6)
	move.l	cnva_add(a6),rx1(a6)
	clr.l	rback(a6)
	clr.l	lback(a6)
	clr.l	back(a6)
	bsr	buffer_making
	pop	d0-a6
	rts

atop_init:
	* ADPCM->PCM変換をする前にコールしておく
	* 継続用バッファをフラッシュする
	* ステレオかモノラルかの指定をする : d0

	push	a6
	lea	work_area,a6
	move.w	d0,stereo(a6)
	move.l	cnva_add(a6),x1(a6)
	move.l	cnva_add(a6),lx1(a6)
	move.l	cnva_add(a6),rx1(a6)
	clr.l	rback(a6)
	clr.l	lback(a6)
	clr.l	back(a6)
	pop	a6
	rts

atop_exec:
	* PCM->ADPCM変換を行う
	* 引数 : 変換するバイト数 d0
	*	 読み込むADPCMのバッファのアドレス a0
	*	 書き込むPCMのバッファののアドレス a1
	push	d0-a6
	lea	work_area,a6
	move.l	a0,ada_add(a6)
	move.l	a1,pcma_add(a6)
	tst.w	stereo(a6)
	bne	@f
	bsr	conv_mono
	pop	d0-a6
	rts
@@:	bsr	conv_stereo
	pop	d0-a6
	rts

MAKE_BUFFER:
		MOVEQ	#0,D7
		MOVEQ	#16,D6
		MOVE.L	BUFFER_ADR,A1
		LEA	table3,A0
		LEA	table4W,A2
		MOVEQ	#0,D0
	@@:	BSR	MBOFAX
		addq.w	#2,a0
		add.w	#80,a1
		ADDQ.W	#1,D0
		CMP.B	#bufx+2,D0
		BCS	@b
		RTS
MBOFAX:		BSR	MAKE_BAI
		BSR	MAKE_CODE
		BSR	SET_NEXTADR
		RTS
MAKE_BAI:	MOVEQ	#1,D1
		LEA	(A1),A4
		LEA	16(A1),A3
		LEA	32(A1),A5
		MOVE.W	(A0),D2
	1:	MOVE.W	D2,D3
		MULU	D1,D3
		LSR.L	#3,D3
		MOVE.W	D3,(A3)+
		MOVE.W	D2,D3
		MULU	D1,D3
		NEG.L	D3
		ASR.L	#3,D3
		MOVE.W	D3,(A5)+

		ADDQ.W	#1,D1
		MOVE.W	D2,D3
		MULU	D1,D3
		LSR.L	#3,D3
		MOVE.W	D3,(A4)+
		ADDQ.W	#1,D1
		CMP.B	#17,D1
		BNE	1b
		MOVE.W	D2,-(A4)
		RTS
MAKE_CODE:	LEA	48(A1),A4
		MOVEQ	#0,D1
		MOVEQ	#7,D2
	1:	MOVE.W	D1,(A4)+
		ADDQ.W	#1,D1
		DBRA	D2,1b
		RTS
SET_NEXTADR:	LEA	(A2),A4
		LEA	64(A1),A3
		MOVE.L	BUFFER_ADR,D3
		MOVE.W	#7,D1
	1:	MOVE.W	(A4)+,D2
		BSR	GET_VAL
		mulu	#80,d2
		AND.W	#$FFFF,D2
		ADD.W	D3,D2
		SUB.L	A3,D2
		ADD.L	#6,d2
		MOVE.W	D2,(A3)+
		DBRA	D1,1b
		RTS
GET_VAL:	ADD.W	D0,D2
		BMI	2f
		CMP.W	#bufx+1,D2
		BCC	1f
		RTS
	1:	MOVE.W	#bufx,D2
		RTS
	2:	MOVE.w	D7,D2
		RTS

		* ステレオのルーチン

conv_stereob:
		subq.l	#2,d0

		move.l	pcma_add(a6),a1
		move.l	la(a6),a2
		move.l	ra(a6),a3
		move.l	ada_add(a6),a4

		moveq	#0,d6
		moveq	#0,d7
		move.w	ry(a6),d2
		move.w	ly(a6),d1
	lpb:	onef	d2,a3,d6
		lsl.w	#4,d6
		onef	d1,a2,d7
		lsl.w	#4,d7
		onef	d2,a3,d6
		onef	d1,a2,d7

		move.b	d6,(a4)+
		move.b	d7,(a4)+
		moveq	#0,d6
		move.b	d6,d7
		subq.l	#8,d0
		bcc	lpb

		move.w	d1,ly(a6)
		move.l	a2,la(a6)
		move.w	d2,ry(a6)
		move.l	a3,ra(a6)
		rts

		* モノラルのルーチン

conv_monob:
		subq.l	#1,d0

		move.l	pcma_add(a6),a1
		move.l	ra(a6),a3
		move.l	ada_add(a6),a4

		moveq	#0,d6
		move.w	y(a6),d2
	lpbm:	onef	d2,a3,d6
		lsl.w	#4,d6
		onef	d2,a3,d6
		move.b	d6,(a4)+
		moveq	#0,d6
		subq.l	#4,d0
		bcc	lpbm

		move.w	d2,y(a6)
		move.l	a3,ra(a6)

		rts

buffer_making:	move.l	cnva_add(a6),a0
		lea	table3,a1
		lea	table4,a2
		moveq	#0,d7
	loop1:	moveq	#0,d1
	loop2:	move.l	d7,d0
		move.w	d1,d2
		and.w	#$70,d2
		lsr.b	#3,d2
		move.w	(a2,d2.w),d3
		add.b	#1,d2
		move.l	d0,d6
		add.l	d6,d6
		move.w	(a1,d6.w),d6

		mulu	d6,d2
		btst	#7,d1
		beq	@f
		neg.l	d2
	@@:
		asr.l	#3,d2

		move.w	d2,(a0)+
		add.b	d3,d0
		bpl	@f
		moveq	#0,d0
	@@:	cmp.w	#bufx,d0
		bcs	@f

		moveq	#bufx,d0
	@@:	move.w	d1,d2
		and.w	#$7,d2
		lsl.b	#1,d2
		move.w	(a2,d2.w),d3
		add.b	#1,d2
		move.l	d0,d6
		add.l	d6,d6
		move.w	(a1,d6.w),d6

		mulu	d6,d2
		btst	#3,d1
		beq	@f
		neg.l	d2
	@@:
		asr.l	#3,d2

		move.w	d2,(a0)+
		add.b	d3,d0
		bpl	@f
		moveq	#0,d0
	@@:	cmp.w	#bufx,d0
		bcs	@f
		moveq	#bufx,d0

	@@:	lsl.l	#3,d0
		lsl.l	#8,d0
		add.l	cnva_add(a6),d0
		sub.l	a0,d0
		move.l	d0,(a0)+

		addq.b	#1,d1
		bcc	loop2
		addq.b	#1,d7
		cmp.b	#bufx+1,d7
		bne	loop1

		move.l	cnva_add(a6),x1(a6)
		move.l	cnva_add(a6),lx1(a6)
		clr.l	back(a6)
		clr.l	lback(a6)
		rts

		* 入力 D0 : データの個数

conv_mono:
		subq.l	#1,d0

		move.l	x1(a6),a0
		move.l	ada_add(a6),a1
		move.l	pcma_add(a6),a2
		move.l	back(a6),d1
		moveq	#0,d4
@@:		move.w	d4,d3
		move.b	(a1)+,d3
		lsl.w	#3,d3
		add.w	d3,a0

		add.w	(a0)+,d1
		move.w	d1,(a2)+
		add.w	(a0)+,d1
		move.w	d1,(a2)+

		add.l	(a0),a0
		subq.l	#1,d0
		bcc	@b
		move.l	d1,back(a6)
		move.l	a0,x1(a6)
		rts

conv_stereo:
		subq.l	#1,d0

		move.l	rx1(a6),a0
		move.l	lx1(a6),a1
		move.l	ada_add(a6),a2
		move.l	pcma_add(a6),a3
		move.l	rback(a6),d1
		move.l	lback(a6),d2
		moveq	#0,d4

@@:
		move.w	d4,d3
		move.b	(a2)+,d3
		lsl.w	#3,d3
		add.w	d3,a0

		move.w	d4,d3
		move.b	(a2)+,d3
		lsl.w	#3,d3
		add.w	d3,a1

		add.w	(a0)+,d1
		move.w	d1,(a3)+
		add.w	(a1)+,d2
		move.w	d2,(a3)+

		add.w	(a0)+,d1
		move.w	d1,(a3)+
		add.w	(a1)+,d2
		move.w	d2,(a3)+

		add.l	(a0),a0
		add.l	(a1),a1

		subq.l	#2,d0
		bcc	@b

		move.l	d1,rback(a6)
		move.l	d2,lback(a6)
		move.l	a0,rx1(a6)
		move.l	a1,lx1(a6)
		rts

	* データを書き込むふりをする

conv_monon:
		subq.l	#1,d0

		move.l	x1(a6),a0
		move.l	ada_add(a6),a1
		move.l	back(a6),d1
		moveq	#0,d4
@@:		move.w	d4,d3
		move.b	(a1)+,d3
		lsl.w	#3,d3
		add.w	d3,a0

		add.w	(a0)+,d1
		add.w	(a0)+,d1

		add.l	(a0),a0
		subq.l	#1,d0
		bcc	@b
		move.l	d1,back(a6)
		move.l	a0,x1(a6)
		rts

conv_stereon:
		subq.l	#1,d0

		move.l	rx1(a6),a0
		move.l	lx1(a6),a1
		move.l	ada_add(a6),a2
		move.l	rback(a6),d1
		move.l	lback(a6),d2
		moveq	#0,d4

@@:
		move.w	d4,d3
		move.b	(a2)+,d3
		lsl.w	#3,d3
		add.w	d3,a0

		move.w	d4,d3
		move.b	(a2)+,d3
		lsl.w	#3,d3
		add.w	d3,a1

		add.w	(a0)+,d1
		add.w	(a1)+,d2

		add.w	(a0)+,d1
		add.w	(a1)+,d2

		add.l	(a0),a0
		add.l	(a1),a1

		subq.l	#2,d0
		bcc	@b

		move.l	d1,rback(a6)
		move.l	d2,lback(a6)
		move.l	a0,rx1(a6)
		move.l	a1,lx1(a6)
		rts


		.data
		.quad
work_area:	ds.b	free_tail-free_head	* オフセットの分取っておく
BUFFER:		DS.B	(bufx+3)*80
BUFFER_ADR:	DC.L	BUFFER
table4W:	dc.w	-1,-1,-1,-1,2,4,6,8
table4:		dc.w	-1,-1,-1,-1,2,4,6,8
table3:		dc.w	16,17,19,21,23,25,28,31,34,37,41,45,50,55,60,66,73,80,88,97,107
		dc.w	118,130,143,157,173,190,209,230,253,279,307,337,371,408,449,494
		dc.w	544,598,658,724,796,875,963,1060,1166,1282,1411,1552
		dc.w	1707,1877,2065,2272,2499,2749,3023,3325,3657,4022
		dc.w	4424,4866,5352,5887,6475
		dc.w	7122,7834,8617,9478,10425
		dc.w	32
		.quad

		end
