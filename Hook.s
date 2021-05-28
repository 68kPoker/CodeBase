
	section kod,code

	xdef	_hookEntry

h_SubEntry	equ 12

_hookEntry:
          pea.l	(a1)
          pea.l	(a2)
          pea.l	(a0)
          movea.l    h_SubEntry(a0),a0
          jsr       (a0)
          lea       12(sp),sp
          rts

	end
