# Game specific API notes
	
- Board/Cell_1.c/h

	- Exported functions
	
		- BOOL clearCell( CELL *cell, BOOL full )
		- BOOL objectPresent( CELL *cell )
		- object_t clearObject( CELL *cell )
		- floor_t clearFloor( CELL *cell )
		- void deleteCell( CELL *cell )
		- floor_t setFloor( CELL *cell, floor_t floor )
		- floor_t getFloor( CELL *cell )
		- object_t setObject( CELL *cell, object_t obj )
		- object_t getObject( CELL *cell )
		
	- Notes
	
		Further description is not required

- Editor/Editor_1.c

	- Exported functions
	
		- odlegloscCentrum() - Calcs the distance from the center
		
- Editor/BoardED_1.c
	
	- Exported functions
	
		- boardDispatch() - Handles Board editor Gadget
		- selectionDispatch() - Handles Tile selection Gadget
		- initBoard() - Inits Board editor Gadget
		- initSelection() - Inits Tile selection Gadget

	- Notes
		
		This file has a nice Board-as-Gadget approach.
