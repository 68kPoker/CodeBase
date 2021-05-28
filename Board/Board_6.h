
/* Ready */

#ifndef BOARD_H
#define BOARD_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#define BOARD_WIDTH  20
#define BOARD_HEIGHT 16

/* Tile consists of floor and optional object */

typedef struct Tile {
	struct Floor {
		UWORD type;
		union FloorInfo {
			UWORD direction; /* FT_SLIDER */
		} info;
	} floor;

	struct Object {
		UWORD type; /* Basic type */
		BOOL moved;
		struct ObjectInfo *info; /* Optional info */
	} object;
	BOOL processed;
} TILE;

/* Basic object types */

enum Objects {
	OT_NONE, /* No object */
	OT_WALL,
	OT_BOX,
	OT_KEY,
	OT_DOOR,
	OT_EXIT,
	OT_PLAYER
};

/* Floor basic types */

enum FloorTypes {
	FT_NONE, /* No floor */
	FT_NORMAL,
	FT_FLAGSTONE,
	FT_SLIDER
};

/* Slider floor directions */

enum Sliders {
	SD_UP,
	SD_RIGHT,
	SD_DOWN,
	SD_LEFT
};

/* Optional info for object */

typedef struct ObjectInfo {
	struct Position {
		UWORD x, y;
	} pos;
} OBJECT_INFO;

/* Board struct */

struct Board {
	TILE tileBoard[ BOARD_HEIGHT ][ BOARD_WIDTH ];
	struct ObjectInfo playerInfo;
	WORD dir_x, dir_y; /* Player movement direction */
	UWORD boxesAll, boxesPlaced, keysGot;
};

/* Prototypes */

/* Clear board struct */
void clearBoard( struct Board *board );

/* Reposition player */
void placePlayer( struct Board *board, UWORD x, UWORD y);

BOOL scanBoard( struct Board *board );
void animateBoard( struct Board *board );

#endif /* BOARD_H */
