
	incdir	'includes:'
	include	'lvo/exec_lib.i'

	xdef	_myCopper

_myCopper:
	move.w	(a1)+,d1
	movea.l	(a1),a1
	moveq	#1,d0
	lsl.l	d1,d0
	movea.l	(4).w,a6
	jsr	(_LVOSignal,a6)
	moveq	#0,d0
	rts
