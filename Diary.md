# Warehouse - Diary

13 February 2021

OK, so what's going on - the legacy source code have been archived and is used as
reference.
I'm writing code base using system structures:
- Colorful (32 colors) screen to display game graphics,
- Windows to provide graphics context (RastPort/Layer) for the board, editor etc.
- WriteMask and Clip Regions to optimize graphics operations,
- IFF Entry handlers to easily load IFF Files - ILBM, 8SVX and similar,
- Blitter code to draw tiles in RastPorts,
- Cells to implement board,
