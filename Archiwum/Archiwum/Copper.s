
	xdef _myCopper

_LVOSignal	equ	-324

_myCopper:

	movea.l	(a1)+,a0	; Task
	move.w	(a1),d1		; Signal

	movea.l	a0,a1
	moveq	#1,d0
	lsl.l	d1,d0
	movea.l	4.w,a6
	jsr	_LVOSignal(a6)

	moveq	#0,d0
	rts
