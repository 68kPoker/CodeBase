
*
* Wygodne i przejrzyste C2P.
*
* Wejscie: A0 - adres 16 pikseli
*
* opc:     D4 - maska
*          D5 - maska
*          A6 - maska
*
* Wynik:   D0 - bitplany 4/0
*          D1 - bitplany 5/1
*          D2 - bitplany 6/2
*          D3 - bitplany 7/3
*

          xdef      @writePixelLine8

          incdir    'includes:'
          include   'graphics/gfx.i'

*         A0 - piksele
*         A1 - bitmapa interleaved
*         D0 - wiersz
*         D1 - sîowo
*         D2 - dîugoôê w sîowach

@writePixelLine8:

          movem.l   d2-d7/a2-a6,-(a7)

          mulu.w    (bm_BytesPerRow,a1),d0
          add.w     d1,d1
          ext.l     d1
          add.l     d1,d0
          lea       (bm_Planes,a1),a1

          movea.l   (a1)+,a2  ; Planes[0]
          movea.l   (a1),a4   ; Planes[1]
          lea       (a2,d2.w*2),a5
          suba.l    a2,a4     ; Bytes per row

          adda.l    d0,a2
          adda.l    d0,a5

	move.l	#$0f0ff0f0,d4
	move.l	#$33333333,d5
	move.l	#$55555555,a6

.loop:
          bsr.s     c2p

          move.w    d0,(a2)
          movea.w   a4,a3
          move.w    d1,(a2,a3.w)
          adda.w    a4,a3
          move.w    d2,(a2,a3.w)
          adda.w    a4,a3
          move.w    d3,(a2,a3.w)
          adda.w    a4,a3
          swap      d0
          swap      d1
          swap      d2
          swap      d3
          move.w    d0,(a2,a3.w)
          adda.w    a4,a3
          move.w    d1,(a2,a3.w)
          adda.w    a4,a3
          move.w    d2,(a2,a3.w)
          adda.w    a4,a3
          move.w    d3,(a2,a3.w)

          adda.l    #2,a2
          cmpa.l    a5,a2
          blt.s     .loop

          movem.l   (a7)+,d2-d7/a2-a6
          rts

c2p:
	lea	(8,a0),a1

	move.b	(a0)+,d0	; piksel 0
	lsl.w	#8,d0
	move.b	(a1)+,d0	; piksel 8
	swap	d0
	move.b	(a0)+,d1	; piksel 1
	lsl.w	#8,d1
	move.b	(a1)+,d1	; piksel 9
	swap	d1
	move.b	(a0)+,d2	; piksel 2
	lsl.w	#8,d2
	move.b	(a1)+,d2	; piksel 10
	swap	d2
	move.b	(a0)+,d3
	lsl.w	#8,d3
	move.b	(a1)+,d3
	swap	d3

	move.b	(a0)+,d0	; piksel 4
	lsl.w	#8,d0
	move.b	(a1)+,d0	; piksel 12
	swap	d0
	move.b	(a0)+,d1	; piksel 5
	lsl.w	#8,d1
	move.b	(a1)+,d1	; piksel 13
	swap	d1
	move.b	(a0)+,d2	; piksel 6
	lsl.w	#8,d2
	move.b	(a1)+,d2	; piksel 14
	swap	d2
	move.b	(a0)+,d3
	lsl.w	#8,d3
	move.b	(a1)+,d3
	swap	d3

	movea.l   a1,a0     ; Aktualizacja

	move.l	d0,d6		; 0, 8, 4, 12
	and.l	d4,d0
	eor.l	d0,d6
	lsr.w	#4,d0
	swap	d0
	lsl.w	#4,d0
	or.l	d6,d0		; 0, 4, 8, 12 (bity 7-4/3-0)

	move.l	d1,d6		; 1, 9, 5, 13
	and.l	d4,d1
	eor.l	d1,d6
	lsr.w	#4,d1
	swap	d1
	lsl.w	#4,d1
	or.l	d6,d1

	move.l	d2,d6		; 2, 10, 6, 14
	and.l	d4,d2
	eor.l	d2,d6
	lsr.w	#4,d2
	swap	d2
	lsl.w	#4,d2
	or.l	d6,d2

	move.l	d3,d6		; 3, 11, 7, 15
	and.l	d4,d3
	eor.l	d3,d6
	lsr.w	#4,d3
	swap	d3
	lsl.w	#4,d3
	or.l	d6,d3

	move.l	d0,d6
	move.l	d2,d7
	and.l	d5,d0
	and.l	d5,d7
	eor.l	d0,d6
	eor.l	d7,d2
	lsl.l	#2,d0
	lsr.l	#2,d2
	or.l	d7,d0		; 0, 2, 4, 6, 8, 10, 12, 14 (bity 5-4/1-0)
	or.l	d6,d2		; 0, 2, 4, 6, 8, 10, 12, 14 (bity 7-6/3-2)

	move.l	d1,d6
	move.l	d3,d7
	and.l	d5,d1
	and.l	d5,d7
	eor.l	d1,d6
	eor.l	d7,d3
	lsl.l	#2,d1
	lsr.l	#2,d3
	or.l	d7,d1		; 1, 3, 5, 7, 9, 11, 13, 15 (bity 5-4/1-0)
	or.l	d6,d3		; 1, 3, 5, 7, 9, 11, 13, 15 (bity 7-6/3-2)

	exg	d5,a6

	move.l	d0,d6
	move.l	d1,d7
	and.l	d5,d0
	and.l	d5,d7
	eor.l	d0,d6
	eor.l	d7,d1
	add.l	d0,d0
	lsr.l	#1,d1
	or.l	d7,d0		; 0-15 (bity 4/0)
	or.l	d6,d1		; 0-15 (bity 5/1)

	move.l	d2,d6
	move.l	d3,d7
	and.l	d5,d2
	and.l	d5,d7
	eor.l	d2,d6
	eor.l	d7,d3
	add.l	d2,d2
	lsr.l	#1,d3
	or.l	d7,d2		; 0-15 (bity 6/2)
	or.l	d6,d3		; 0-15 (bity 7/3)

	exg	d5,a6

; Wynik w d0-d3

	rts
