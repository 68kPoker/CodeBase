
	incdir	'includes:'
	include	'hardware/custom.i'
	include	'hardware/blit.i'
	include	'graphics/gfx.i'
	include	'lvo/exec_lib.i'

	xdef	_func
	xdef	_clean

	xref	_signal
	xref	_task

	STRUCTURE totalNode,bn_SIZEOF
		APTR	tn_BitMap
		APTR	tn_Gfx

_func:
	movem.l	a2-a3,-(a7)

	move.w	#$09f0,bltcon0(a0)
	move.w	#0,bltcon1(a0)

	movea.l	tn_BitMap(a1),a2
	movea.l	tn_Gfx(a1),a3

	move.l	bm_Planes(a3),bltapt(a0)
	move.w	#0,bltamod(a0)

	move.l	bm_Planes(a2),bltdpt(a0)
	move.w	#0,bltdmod(a0)

	move.w	#$ffff,bltafwm(a0)
	move.w	#$ffff,bltalwm(a0)

	move.w	bm_Rows(a2),bltsizv(a0)
	move.w	bm_BytesPerRow(a2),d0
	lsr.w	#1,d0
	move.w	d0,bltsizh(a0)

	moveq	#0,d0

	movem.l	(a7)+,a2-a3

	rts

_clean:
	movem.l	d0-d1/a1/a6,-(a7)

	movea.l	_task,a1
	moveq	#1,d0
	move.w	_signal,d1
	lsl.l	d1,d0
	movea.l	4.w,a6
	jsr	_LVOSignal(a6)

	moveq	#0,d0
	movem.l	(a7)+,d0-d1/a1/a6
	rts
