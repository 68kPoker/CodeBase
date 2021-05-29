# Game specific API notes
	
- [Board/Cell_1.c/h](https://github.com/68kPoker/Magazyn/blob/master/Board/Cell_1.c)

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

- [Editor/Editor_1.c](https://github.com/68kPoker/Magazyn/blob/master/Editor/Editor_1.c)

	- Exported functions
	
		- odlegloscCentrum() - Calcs the distance from the center
		
- [Editor/BoardED_1.c](https://github.com/68kPoker/Magazyn/blob/master/Editor/BoardED_1.c)
	
	- Exported functions
	
		- boardDispatch() - Handles Board editor Gadget
		- selectionDispatch() - Handles Tile selection Gadget
		- initBoard() - Inits Board editor Gadget
		- initSelection() - Inits Tile selection Gadget

	- Notes
		
		This file has a nice Board-as-Gadget approach.

- [Engine/Engine_2.c/h](https://github.com/68kPoker/Magazyn/blob/master/Engine/Engine_2.c)

	- Exported functions
	
		- convertBoard() - Converts between editor and game format of the board
		- enter() - Cell entry handler
		- leave() - Cell exit handler

- [Engine/Object_1.c](https://github.com/68kPoker/Magazyn/blob/master/Engine/Object_1.c)

	- Exported functions
	
		- objectRotate() - Handles object rotation

- [Game/Game_2.c/h](https://github.com/68kPoker/Magazyn/blob/master/Game/Game_2.c)

	- Exported functions
	
		- newGame() - Clears game board
		- moveCheckHero() - Moves hero with checking
		- canMove() - Checks if hero can move
		- canPush() - Checks if hero can push box
		- moveHero() - Moves hero withouth checking
		- moveObject() - Moves (copies) object 
		- clearObject() - Discard object
		 		
- [Game/Game_32.c/h](https://github.com/68kPoker/Magazyn/blob/master/Game/Game_32.c)

	- Exported functions
	
		- loadBoard() - Load board data into record.
