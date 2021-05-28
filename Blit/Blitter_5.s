
	incdir	'includes:'
	include	'hardware/blit.i'
	include	'hardware/custom.i'

	xdef	@Octant
	xdef	@LineMode

custom	equ	$dff000

; Draw modes

COMP	equ	ABNC | NABC | NANBC
NORMAL	equ	ABNC | NABC | NANBC | ABC

; Calc octant, absolute values and swap dx/dy if needed

; Input:
; d0.w: dx
; d1.w: dy
; a0:	buffer for 2 words

@Octant:
	move.w	d2,-(a7)

	clr.w	d2

; Calc ABS and set signs:
	tst.w	d1
	bpl.s	.pos_dy
	neg.w	d1
	bset	#2,d2
.pos_dy:
	tst.w	d0
	bpl.s	.pos_dx
	neg.w	d0
	bset	#1,d2
.pos_dx:
	cmp.w	d1,d0
	bge.s	.dx_greater
	exg	d1,d0
	bset	#0,d2
.dx_greater:
	move.b	table(pc,d2.w),d2
	move.w	d0,(a0)+
	move.w	d1,(a0)+
	move.w	d2,(a0)
	move.w	d2,d0
	ext.l	d0

	move.w	(a7)+,d2
	rts

table:
	dc.b	OCTANT1
	dc.b	OCTANT2
	dc.b	OCTANT4
	dc.b	OCTANT3
	dc.b	OCTANT8
	dc.b	OCTANT7
	dc.b	OCTANT5
	dc.b	OCTANT6

; Setup Blitter for line mode

; Input:
; a0: dx, dy, oct
; a1: plane
; d0.w: x1 & 15
; d1.b: NORMAL or COMP
; d2.w: bpr

@LineMode:
	movem.l	d3-d5/a2,-(a7)

	move.w	#(SRCA|SRCC|DEST),d4
	ror.w	#4,d0
	or.w	d0,d4
	or.b	d1,d4

	move.w	#(LINEMODE),d5

	lea	custom,a2
	move.w	#$8000,bltadat(a2)
	moveq	#-1,d0
	move.w	d0,bltbdat(a2)
	move.w	d0,bltafwm(a2)
	move.w	d0,bltalwm(a2)

	movem.w	(a0)+,d0-d1/d3
	or.b	d3,d5

	lsl.w	#2,d1
	move.w	d1,bltbmod(a2)
	add.w	d0,d0
	sub.w	d0,d1
	bpl.s	.pos
	ori.w	#(SIGNFLAG),d5
.pos:
	move.l	d1,bltapt(a2)

	sub.w	d0,d1
	move.w	d1,bltamod(a2)
	move.w	d2,bltcmod(a2)
	move.w	d2,bltdmod(a2)
	move.l	a1,bltcpt(a2)
	move.l	a1,bltdpt(a2)
	move.w	d4,bltcon0(a2)
	move.w	d5,bltcon1(a2)
	lsr.w	#1,d0
	addq.w	#1,d0
	lsl.w	#6,d0
	ori.b	#2,d0
	move.w	d0,bltsize(a2)

	movem.l	(a7)+,d3-d5/a2
	rts
