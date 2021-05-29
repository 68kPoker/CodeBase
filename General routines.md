# General API notes

1. Blitter_4.c/h

	a. Exported functions

		- drawTile() - Draws single layer tile	
		- drawTileLayers() - Draws dual layer tile
		- drawTileRastPort - Draws tile into RastPort

	b. Notes

		This needs to be looked into to complete it. Generally it's 100% complete but may require fixes.

2. Joystick_2.c/h

	a. Exported functions

		- IOStd openJoy(struct InputEvent *ie) - sets up joystick
		- void closeJoy(IOStd io) - Closes joystick
		- void readEvent(IOStd io, struct InputEvent *ie) - reads next joystick event

	b. Notes

		This code is 100% complete and working.

3. Misc_2.c/h

	a. Exported functions

		- GWopenLibs(ULONG minVersion) - Opens required libraries (Intuition, GadTools, Graphics, IFFParse)
		- GWcleanup() - Closes opened libraries
		- GWerror(STRPTR errorDesc) - Prints error string
		- GWbailout(STRPTR errorDesc) - Prints error string and exits

	b. Notes

		This code is mainly to open libraries and setup basic system resources.
