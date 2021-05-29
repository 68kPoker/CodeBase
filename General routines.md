# General API notes

1. Blit/Blitter_4.c/h

	a. Exported functions

		- drawTile() - Draws single layer tile	
		- drawTileLayers() - Draws dual layer tile
		- drawTileRastPort - Draws tile into RastPort

	b. Notes

		This needs to be looked into to complete it. Generally it's 100% complete but may require fixes.

2. Joystick/Joystick_2.c/h

	a. Exported functions

		- IOStd openJoy(struct InputEvent *ie) - sets up joystick
		- void closeJoy(IOStd io) - Closes joystick
		- void readEvent(IOStd io, struct InputEvent *ie) - reads next joystick event

	b. Notes

		This code is 100% complete and working.

3. Misc/Misc_2.c/h

	a. Exported functions

		- GWopenLibs(ULONG minVersion) - Opens required libraries (Intuition, GadTools, Graphics, IFFParse)
		- GWcleanup() - Closes opened libraries
		- GWerror(STRPTR errorDesc) - Prints error string
		- GWbailout(STRPTR errorDesc) - Prints error string and exits

	b. Notes

		This code is mainly to open libraries and setup basic system resources.

4. Screen/Screen_51.c/h

	a. Exported functions
	
		- allocBitMap() - Allocates displayable bitmap
		- openScreen() - Opens new screen with given bitmap (also opens font)
		- addCopperInt() - Adds copper interrupt server
		- remCopperInt() - Removes copper interrupt server
		- addCopperList() - Adds user copper-list

	b. Notes
		
		This code is complete, but the DBufInfo should also be allocated (unless I will use RasInfo->BitMap).
		The DBufInfo creation is in other file anyway.

5. Copper/Copper.s

	a. Exported functions
	
		- myCopper() - Copper interrupt server. Send signal to main task when ViewPort is visible.
		
	b. Notes
	
		This is written in assembly to properly handle Z condition code state.

6. Windows/Windows_24.c/h

	a. Exported functions
	
		- openWindow() - Opens backdrop window
		- mainLoop() - Main signal loop which covers window's UserPort as well as Safe message and Copper interrupt

7. IFF/IFF_4.c/h

	a. Exported functions
	
		- openIFile()/openIClip() - Opens Interchange File Format file fo reading or writing
		- openIFF() - Allocates and opens IFF stream
		- closeIFF() - Closes IFF stream
		- scanIFF() - Scans IFF file for chunks
		- obtainCMAP() - Obtains Color Map from ILBM
		- loadCMAP() - Loads colors into Color Map
		- obtainBMHD() - Obtains BitMap Header
		- loadILBM() - Reads ILBM body into BitMap

8. Gadgets_5.c/h

	a. Exported functions
	
		- initText() - Inits IntuiText
		- initButton() - Inits Gadget as Button
		- cutImage() - Cuts Image from BitMap
		- freeImage() - Frees Image data
