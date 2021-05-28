
          incdir    'Includes:'
          include   'hardware/custom.i'
          include   'hardware/intbits.i'

          xdef      _blitterHandler

_blitterHandler:

          move.l    a0,-(a7)

          move.l    (a1)+,a0
          move.l    (a1),d0
          movea.l   a0,a1
          jsr       (-324,a6)

          move.l    (a7)+,a0
          move.w    #INTF_BLIT,(intreq,a0)
          rts
