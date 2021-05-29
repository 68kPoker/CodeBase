# General API notes

- Blit/Blitter_4.c/h

	- Exported functions

		- drawTile() - Draws single layer tile	
		- drawTileLayers() - Draws dual layer tile
		- drawTileRastPort - Draws tile into RastPort

	- Notes

		This needs to be looked into to complete it. Generally it's 100% complete but may require fixes.

- Joystick/Joystick_2.c/h

	- Exported functions

		- IOStd openJoy(struct InputEvent *ie) - sets up joystick
		- void closeJoy(IOStd io) - Closes joystick
		- void readEvent(IOStd io, struct InputEvent *ie) - reads next joystick event

	- Notes

		This code is 100% complete and working.

- Misc/Misc_2.c/h

	- Exported functions

		- GWopenLibs(ULONG minVersion) - Opens required libraries (Intuition, GadTools, Graphics, IFFParse)
		- GWcleanup() - Closes opened libraries
		- GWerror(STRPTR errorDesc) - Prints error string
		- GWbailout(STRPTR errorDesc) - Prints error string and exits

	- Notes

		This code is mainly to open libraries and setup basic system resources.

- Screen/Screen_51.c/h

	- Exported functions
	
		- allocBitMap() - Allocates displayable bitmap
		- openScreen() - Opens new screen with given bitmap (also opens font)
		- addCopperInt() - Adds copper interrupt server
		- remCopperInt() - Removes copper interrupt server
		- addCopperList() - Adds user copper-list

	- Notes
		
		This code is complete, but the DBufInfo should also be allocated (unless I will use RasInfo->BitMap).
		The DBufInfo creation is in other file anyway.

- Copper/Copper.s

	- Exported functions
	
		- myCopper() - Copper interrupt server. Send signal to main task when ViewPort is visible.
		
	- Notes
	
		This is written in assembly to properly handle Z condition code state.

- Windows/Windows_24.c/h

	- Exported functions
	
		- openWindow() - Opens backdrop window
		- mainLoop() - Main signal loop which covers window's UserPort as well as Safe message and Copper interrupt

- IFF/IFF_4.c/h

	- Exported functions
	
		- openIFile()/openIClip() - Opens Interchange File Format file fo reading or writing
		- openIFF() - Allocates and opens IFF stream
		- closeIFF() - Closes IFF stream
		- scanIFF() - Scans IFF file for chunks
		- obtainCMAP() - Obtains Color Map from ILBM
		- loadCMAP() - Loads colors into Color Map
		- obtainBMHD() - Obtains BitMap Header
		- loadILBM() - Reads ILBM body into BitMap

- Gadgets_5.c/h

	- Exported functions
	
		- initText() - Inits IntuiText
		- initButton() - Inits Gadget as Button
		- cutImage() - Cuts Image from BitMap
		- freeImage() - Frees Image data

- Audio/Audio_1.c/h

	- Exported functions
	
		- allocChannels() - Allocates audio channels
		- freeChannels() - Frees audio channels
		- playSample() - Plays sound
