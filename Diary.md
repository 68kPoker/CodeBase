# Warehouse - Diary

- 29 May 2021

	Added Notes on Appliaction Programming Interface (API) for general and game
	specific functions.

- 28 May 2021

	OK, I managed to gather 608+ source code files from this year only!
	I've put it into separate directories.
	The next step will be to get the most complete files and build the game.

- 16 February 2021

	Lots of good happened! Further description tommorow. Now I check-in the latest
	source code with my own Window-movement function, Copper-sync screen and 
	Tile-refresh function and IFF ILBM image loading routine. In place there
	will be Gadget-cutting function.

- 13 February 2021

	OK, so what's going on - the legacy source code have been archived and is used as
	reference.
	I'm writing code base using system structures:
	
		- Colorful (32 colors) screen to display game graphics,
		- Windows to provide graphics context (RastPort/Layer) for the board, editor etc.
		- WriteMask and Clip Regions to optimize graphics operations,
		- IFF Entry handlers to easily load IFF Files - ILBM, 8SVX and similar,
		- Blitter code to draw tiles in RastPorts,
		- Cells to implement board,
		- Copper interrupt and user Copperlist for animation.