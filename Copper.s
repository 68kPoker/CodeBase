
	incdir	'includes:'
	include	'graphics/view.i'
	include	'lvo/exec_lib.i'

	xdef	_myCopper

_myCopper:

	movea.l	(a1)+,a0
	move.w	vp_Modes(a0),d0
	andi.w	#V_VP_HIDE,d0
	bne.s	.skip

	move.w	(a1)+,d1
	moveq	#1,d0
	movea.l	(a1),a1
	lsl.l	d1,d0
	movea.l	4.w,a6
	jsr	_LVOSignal(a6)
.skip:
	moveq	#0,d0
	rts
