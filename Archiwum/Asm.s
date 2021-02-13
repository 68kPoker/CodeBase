
	incdir	'include:'
	include	'hardware/custom.i'
	include	'hardware/intbits.i'
	include	'utility/hooks.i'

	xdef	_myCopperIs
	xdef	_hookEntry

_myCopperIs:
	moveq	#1,d0
	move.w	(a1)+,d1
	lsl.l	d1,d0
	movea.l	(a1),a1
	movea.l	4.w,a6
	jsr	-324(a6)
;	lea	$dff000,a0 ; only for VBlank with high priority
	moveq	#0,d0
	rts

_hookEntry:
	pea.l	(a1)
	pea.l	(a2)
	pea.l	(a0)
	movea.l	h_SubEntry(a0),a0
	jsr	(a0)
	lea	12(a7),a7
	rts

;_blitterInt:
;	move.l	a0,-(a7)
;	movea.l	_task,a1
;	move.l	_bltsignal,d0
;	jsr	-324(a6)
;	move.l	(a7)+,a0

;	move.w	#INTF_BLIT,intreq(a0)
;	rts
