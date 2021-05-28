
	xdef	_myCopperCode

_LVOSignal	equ	-324

_myCopperCode:
	movea.l	(4).w,a6
	movea.l	(a1)+,a0
	move.w	(a1),d1
	moveq	#1,d0
	lsl.l	d1,d0
	movea.l	a0,a1
	jsr	(_LVOSignal,a6)

	moveq	#0,d0
	rts
