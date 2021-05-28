;------------------------------
	; a0: bitplane 0
	; a1: bitplane 4
	; a2: bitplane 1
	; a3: bitplane 5
	; a4: BPR*2
	; a5: source buffer
	; a6: end of buffer

c2p:
	move.l	#$cccc3333,d6	; mask
	move.l	#$11111111,d7	; mask
	
.loop:
	move.l	(a5)+,d1
	move.l	d1,d5
	and.l	d6,d1
	eor.l	d1,d5
	lsr.w	#4,d5
	swap	d5
	lsl.w	#4,d5
	or.l	d5,d1		; a7a6a5a4e7e6e5e4 i7i6i5i4m7m6m5m4 a3a2a1a0e3e2e1e0 i3i2i1i0m3m2m1m0

	move.l	(a5)+,d2
	move.l	d2,d5
	and.l	d6,d2
	eor.l	d2,d5
	lsr.w	#4,d5
	swap	d5
	lsl.w	#4,d5
	or.l	d5,d2

	move.l	(a5)+,d3
	move.l	d3,d5
	and.l	d6,d3
	eor.l	d3,d5
	lsr.w	#4,d5
	swap	d5
	lsl.w	#4,d5
	or.l	d5,d3

	move.l	(a5)+,d4
	move.l	d4,d5
	and.l	d6,d4
	eor.l	d4,d5
	lsr.w	#4,d5
	swap	d5
	lsl.w	#4,d5
	or.l	d5,d4

	move.l	d1,d0
	and.l	d7,d0
	add.l	d0,d0

	move.l	d2,d5
	and.l	d7,d5
	or.l	d5,d0
	add.l	d0,d0

	move.l	d3,d5
	and.l	d7,d5
	or.l	d5,d0
	add.l	d0,d0

	move.l	d4,d5
	and.l	d7,d5
	or.l	d5,d0		; d0: a4b4c4d4e4f4g4h4 i4j4k4l4m4n4o4p4 a0b0c0d0e0f0g0h0 i0j0k0l0m0n0o0p0

	move.w	d0,(a0)+	; save planar bitplane 0
	swap	d0
	move.w	d0,(a1)+	; save planar bitplane 4

	lsr.l	#1,d1
	lsr.l	#1,d2
	lsr.l	#1,d3
	lsr.l	#1,d4

	move.l	d1,d0
	and.l	d7,d0
	add.l	d0,d0

	move.l	d2,d5
	and.l	d7,d5
	or.l	d5,d0
	add.l	d0,d0

	move.l	d3,d5
	and.l	d7,d5
	or.l	d5,d0
	add.l	d0,d0

	move.l	d4,d5
	and.l	d7,d5
	or.l	d5,d0		; d0: a5b5c5d5e5f5g5h5 i5j5k5l5m5n5o5p5 a1b1c1d1e1f1g1h1 i1j1k1l1m1n1o1p1

	move.w	d0,(a2)+	; save planar bitplane 1
	swap	d0
	move.w	d0,(a3)+	; save planar bitplane 5

	lsr.l	#1,d1
	lsr.l	#1,d2
	lsr.l	#1,d3
	lsr.l	#1,d4
	
	move.l	d1,d0
	and.l	d7,d0
	add.l	d0,d0

	move.l	d2,d5
	and.l	d7,d5
	or.l	d5,d0
	add.l	d0,d0

	move.l	d3,d5
	and.l	d7,d5
	or.l	d5,d0
	add.l	d0,d0

	move.l	d4,d5
	and.l	d7,d5
	or.l	d5,d0		; d0: a6b6c6d6e6f6g6h6 i6j6k6l6m6n6o6p6 a2b2c2d2e2f2g2h2 i2j2k2l2m2n2o2p2

	move.w	d0,(-2,a0,a4.w)	; save planar bitplane 2
	swap	d0
	move.w	d0,(-2,a1,a4.w)	; save planar bitplane 6

	lsr.l	#1,d1
	lsr.l	#1,d2
	lsr.l	#1,d3
	lsr.l	#1,d4

	move.l	d1,d0
	and.l	d7,d0
	add.l	d0,d0

	move.l	d2,d5
	and.l	d7,d5
	or.l	d5,d0
	add.l	d0,d0

	move.l	d3,d5
	and.l	d7,d5
	or.l	d5,d0
	add.l	d0,d0

	move.l	d4,d5
	and.l	d7,d5
	or.l	d5,d0		; d0: a7b7c7d7e7f7g7h7 i7j7k7l7m7n7o7p7 a3b3c3d3e3f3g3h3 i3j3k3l3m3n3o3p3

	move.w	d0,(-2,a2,a4.w)	; save planar bitplane 3
	swap	d0
	move.w	d0,(-2,a3,a4.w)	; save planar bitplane 7
	
	cmpa.l	a6,a5
	blt.s	.loop

	rts