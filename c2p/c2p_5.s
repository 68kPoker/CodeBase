
; Fast WritePixelLine8 function (macro)
; (Writes to 16-pixels wide BitMap for 32-bit writing)

; Input: 	d0-d3: 	16 source pixels
;		a0:	$0f0f0f0f mask
;		a1:	$3333cccc mask
;		a2:	$00ff00ff mask
;		a3:	$55555555 mask
; Target: 	d0-d3: 	16 target pixels

	xdef	@writePixelArray8

mask4	equr	a0
mask2	equr	a1
mask8	equr	a2
mask1	equr	a3
mask	equr	d6
copy_hi	equr	d4
copy_lo	equr	d5
abcd	equr	d0
efgh	equr	d1
ijkl	equr	d2
mnop	equr	d3

Mask	macro
	move.l	\1,copy_hi
	move.l	\2,copy_lo
	and.l	mask,\1
	and.l	mask,copy_lo
	eor.l	\1,copy_hi
	eor.l	copy_lo,\2
	lsl.l	\3,\1
	lsr.l	\3,\2
	or.l	copy_lo,\1
	or.l	copy_hi,\2
	endm

Mask2	macro
	move.l	\1,copy_hi
	move.l	\2,copy_lo
	and.l	mask,\1
	and.l	mask,\2
	eor.l	\1,copy_hi
	eor.l	\2,copy_lo
	lsr.w	#2,\1
	lsr.w	#2,\2
	swap	\1
	swap	\2
	lsl.w	#2,\1
	lsl.w	#2,\2
	or.l	copy_hi,\1
	or.l	copy_lo,\2
	endm

Mask16	macro
	swap	\1
	eor.w	\1,\2
	eor.w	\2,\1
	eor.w	\1,\2
	move.l	\1,\3
	swap	\2
	move.l	\2,\3
	endm

C2P	macro
; abcd: a7a6a5a4a3a2a1a0 b7b6b5b4b3b2b1b0 c7c6c5c4c3c2c1c0 d7d6d5d4d3d2d1d0

	move.l	mask4,mask	; 1
	Mask	abcd,efgh,#4	; 11
	Mask	ijkl,mnop,#4	; 21

	move.l	mask2,mask	; 22
	Mask2	abcd,efgh	; 36
	Mask2	ijkl,mnop	; 50

	move.l	mask8,mask	; 51
	Mask	abcd,ijkl,#8	; 61
	Mask	efgh,mnop,#8	; 71

	move.l	mask1,mask	; 72
	Mask	ijkl,abcd,#1	; 82
	Mask	mnop,efgh,#1	; 92

;	Mask16	ijkl,abcd	; 97
;	Mask16	mnop,efgh	; 102

; ijkl:	a0b0c0d0e0f0g0h0 i0j0k0l0m0n0o0p0 a1b1c1d1e1f1g1h1 i1j1k1l1m1n1o1p1
; abcd: a2b2c2d2e2f2g2h2 i2j2k2l2m2n2o2p2 a3b3c3d3e3f3g3h3 i3j3k3l3m3n3o3p3
; mnop: a4b4c4d4e4f4g4h4 i4j4k4l4m4n4o4p4 a5b5c5d5e5f5g5h5 i5j5k5l5m5n5o5p5
; efgh: a6b6c6d6e6f6g6h6 i6j6k6l6m6n6o6p6 a7b7c7d7e7f7g7h7 i7j7k7l7m7n7o7p7

	endm

; a0: chunky
; a1: planar
; a2: end of planar
; d0: pixels per row

@writePixelArray8:

	movem.l	d2-d7/a2-a6,-(a7)

	movea.l	a0,a4
	movea.l	a1,a5
	movea.l	a2,a6

	move.l	#$0f0f0f0f,mask4
	move.l	#$3333cccc,mask2
	move.l	#$00ff00ff,mask8
	move.l	#$55555555,mask1

	move.w	d0,d7	; Pixels per row
	subi.w	#16,d7

.loop:
	movem.l	(a4)+,abcd-mnop
	adda.w	d7,a4 ; Modulo

	C2P

	Mask16	ijkl,abcd,(a5)+
	Mask16	mnop,efgh,(a5)+

	cmpa.l	a5,a6
	bgt.s	.loop

	movem.l	(a7)+,d2-d7/a2-a6

	rts
