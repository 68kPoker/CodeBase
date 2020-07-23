
          incdir    'includes:'
          include   'utility/hooks.i'

          xdef      _hookEntry

_hookEntry:
          pea.l     (a1)
          pea.l     (a2)
          pea.l     (a0)
          move.l    h_SubEntry(a0),a0
          jsr       (a0)
          lea       12(sp),sp
          rts
