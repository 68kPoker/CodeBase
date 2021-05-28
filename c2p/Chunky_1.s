
; c2p cost calculation

; a7a6a5a4a3a2a1a0 b7b6b5b4b3b2b1b0 c7c6c5c4c3c2c1c0 d7d6d5d4d3d2d1d0

; division to 4 bits per pixel

; convert 128-bits single line

; param: chunky pixels: a0
; param: interleaved bitplanes: a1
; param: bytes per scanline: d0
; param: bytes to write: d1

	xdef	@WritePixelLineLo
	xdef	@WritePixelLineHi

@WritePixelLineLo:

	movem.l	d2-d7/a2-a6,-(a7)

	move.l	#$0f0f0f0f,d6
	move.l	#$ff00ff00,d7
	move.l	#$33333333,a6
	move.l	#$55555555,a5

	move.w	d0,a4
	asl.w	#2,d0
	sub.w	a4,d0
	subq.w	#4,d0
	move.w	d0,a3
	movea.l	a1,a2
	adda.w	d1,a2
.loop:
	move.l	(a0)+,d0
	and.l	d6,d0
	move.l	(a0)+,d1
	and.l	d6,d1

	lsl.l	#4,d0
	or.l	d1,d0

; a3a2a1a0 e3e2e1e0 b3b2b1b0 f3f2f1f0 c3c2c1c0 g3g2g1g0 d3d2d1d0 h3h2h1h0

	move.l	(a0)+,d1
	and.l	d6,d1
	move.l	(a0)+,d2
	and.l	d6,d2

	lsl.l	#4,d1
	or.l	d2,d1

; i3i2i1i0 m3m2m1m0 j3j2j1j0 n3n2n1n0 k3k2k1k0 o3o2o1o0 l3l2l1l0 p3p2p1p0

	move.l	d0,d2
	move.l	d1,d3

	and.l	d7,d0
	and.l	d7,d3

	eor.l	d0,d2
	eor.l	d3,d1

	lsr.l	#8,d3
	lsl.l	#8,d2

	or.l	d3,d0
	or.l	d2,d1

; a3a2a1a0 e3e2e1e0 i3i2i1i0 m3m2m1m0 c3c2c1c0 g3g2g1g0 k3k2k1k0 o3o2o1o0
; b3b2b1b0 f3f2f1f0 j3j2j1j0 n3n2n1n0 d3d2d1d0 h3h2h1h0 l3l2l1l0 p3p2p1p0

	move.l	(a0)+,d2
	and.l	d6,d2
	move.l	(a0)+,d3
	and.l	d6,d3

	lsl.l	#4,d2
	or.l	d3,d2

	move.l	(a0)+,d3
	and.l	d6,d3
	move.l	(a0)+,d4
	and.l	d6,d4

	lsl.l	#4,d3
	or.l	d4,d3

	move.l	d2,d4
	move.l	d3,d5

	and.l	d7,d2
	and.l	d7,d5

	eor.l	d2,d4
	eor.l	d5,d3

	lsr.l	#8,d5
	lsl.l	#8,d4

	or.l	d5,d2
	or.l	d4,d3

	swap	d2
	eor.w	d0,d2
	eor.w	d2,d0
	eor.w	d0,d2
	swap	d2

	swap	d3
	eor.w	d1,d3
	eor.w	d3,d1
	eor.w	d1,d3
	swap	d3

; a3a2a1a0 e3e2e1e0 i3i2i1i0 m3m2m1m0 A3A2A1A0 E3E2E1E0 I3I2I1I0 M3M2M1M0
; c3c2c1c0 g3g2g1g0 k3k2k1k0 o3o2o1o0 C3C2C1C0 G3G2G1G0 K3K2K1K0 O3O2O1O0

	move.l	a6,d5

	move.l	d0,d4
	and.l	d5,d0
	and.l	d2,d5

	eor.l	d0,d4
	eor.l	d5,d2

	lsl.l	#2,d0
	lsr.l	#2,d2

	or.l	d5,d0
	or.l	d4,d2

	move.l	a6,d5

	move.l	d1,d4
	and.l	d5,d1
	and.l	d3,d5

	eor.l	d1,d4
	eor.l	d5,d3

	lsl.l	#2,d1
	lsr.l	#2,d3

	or.l	d5,d1
	or.l	d4,d3

	move.l	a5,d5

	move.l	d0,d4
	and.l	d5,d0
	and.l	d1,d5

	eor.l	d0,d4
	eor.l	d5,d1

	add.l	d0,d0
	lsr.l	#1,d1

	or.l	d5,d0
	or.l	d4,d1

	move.l	a5,d5

	move.l	d2,d4
	and.l	d5,d2
	and.l	d3,d5

	eor.l	d2,d4
	eor.l	d5,d3

	add.l	d2,d2
	lsr.l	#1,d3

	or.l	d5,d2
	or.l	d4,d3

	move.l	d0,(a1)
	adda.w	a4,a1
	move.l	d1,(a1)
	adda.w	a4,a1
	move.l	d2,(a1)
	adda.w	a4,a1
	move.l	d3,(a1)
	suba.w	a3,a1

	cmpa.l	a1,a2
	bgt.s	.loop

	movem.l	(a7)+,d2-d7/a2-a6

	rts

@WritePixelLineHi:

	movem.l	d2-d7/a2-a6,-(a7)

	move.l	#$f0f0f0f0,d6
	move.l	#$ff00ff00,d7
	move.l	#$33333333,a6
	move.l	#$55555555,a5

	move.w	d0,a4
	asl.w	#2,d0
	sub.w	a4,d0
	subq.w	#4,d0
	move.w	d0,a3
	movea.l	a1,a2
	adda.w	d1,a2
.loop:
	move.l	(a0)+,d0
	and.l	d6,d0
	move.l	(a0)+,d1
	and.l	d6,d1

	lsr.l	#4,d1
	or.l	d1,d0

; a3a2a1a0 e3e2e1e0 b3b2b1b0 f3f2f1f0 c3c2c1c0 g3g2g1g0 d3d2d1d0 h3h2h1h0

	move.l	(a0)+,d1
	and.l	d6,d1
	move.l	(a0)+,d2
	and.l	d6,d2

	lsr.l	#4,d2
	or.l	d2,d1

; i3i2i1i0 m3m2m1m0 j3j2j1j0 n3n2n1n0 k3k2k1k0 o3o2o1o0 l3l2l1l0 p3p2p1p0

	move.l	d0,d2
	move.l	d1,d3

	and.l	d7,d0
	and.l	d7,d3

	eor.l	d0,d2
	eor.l	d3,d1

	lsr.l	#8,d3
	lsl.l	#8,d2

	or.l	d3,d0
	or.l	d2,d1

; a3a2a1a0 e3e2e1e0 i3i2i1i0 m3m2m1m0 c3c2c1c0 g3g2g1g0 k3k2k1k0 o3o2o1o0
; b3b2b1b0 f3f2f1f0 j3j2j1j0 n3n2n1n0 d3d2d1d0 h3h2h1h0 l3l2l1l0 p3p2p1p0

	move.l	(a0)+,d2
	and.l	d6,d2
	move.l	(a0)+,d3
	and.l	d6,d3

	lsr.l	#4,d3
	or.l	d3,d2

	move.l	(a0)+,d3
	and.l	d6,d3
	move.l	(a0)+,d4
	and.l	d6,d4

	lsr.l	#4,d4
	or.l	d4,d3

	move.l	d2,d4
	move.l	d3,d5

	and.l	d7,d2
	and.l	d7,d5

	eor.l	d2,d4
	eor.l	d5,d3

	lsr.l	#8,d5
	lsl.l	#8,d4

	or.l	d5,d2
	or.l	d4,d3

	swap	d2
	eor.w	d0,d2
	eor.w	d2,d0
	eor.w	d0,d2
	swap	d2

	swap	d3
	eor.w	d1,d3
	eor.w	d3,d1
	eor.w	d1,d3
	swap	d3

; a3a2a1a0 e3e2e1e0 i3i2i1i0 m3m2m1m0 A3A2A1A0 E3E2E1E0 I3I2I1I0 M3M2M1M0
; c3c2c1c0 g3g2g1g0 k3k2k1k0 o3o2o1o0 C3C2C1C0 G3G2G1G0 K3K2K1K0 O3O2O1O0

	move.l	a6,d5

	move.l	d0,d4
	and.l	d5,d0
	and.l	d2,d5

	eor.l	d0,d4
	eor.l	d5,d2

	lsl.l	#2,d0
	lsr.l	#2,d2

	or.l	d5,d0
	or.l	d4,d2

	move.l	a6,d5

	move.l	d1,d4
	and.l	d5,d1
	and.l	d3,d5

	eor.l	d1,d4
	eor.l	d5,d3

	lsl.l	#2,d1
	lsr.l	#2,d3

	or.l	d5,d1
	or.l	d4,d3

	move.l	a5,d5

	move.l	d0,d4
	and.l	d5,d0
	and.l	d1,d5

	eor.l	d0,d4
	eor.l	d5,d1

	add.l	d0,d0
	lsr.l	#1,d1

	or.l	d5,d0
	or.l	d4,d1

	move.l	a5,d5

	move.l	d2,d4
	and.l	d5,d2
	and.l	d3,d5

	eor.l	d2,d4
	eor.l	d5,d3

	add.l	d2,d2
	lsr.l	#1,d3

	or.l	d5,d2
	or.l	d4,d3

	move.l	d0,(a1)
	adda.w	a4,a1
	move.l	d1,(a1)
	adda.w	a4,a1
	move.l	d2,(a1)
	adda.w	a4,a1
	move.l	d3,(a1)
	suba.w	a3,a1

	cmpa.l	a1,a2
	bgt.s	.loop

	movem.l	(a7)+,d2-d7/a2-a6

	rts
