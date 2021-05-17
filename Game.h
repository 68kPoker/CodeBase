
#ifndef GAME_H
#define GAME_H

#include <exec/types.h>

#define BOARD_WIDTH  20
#define BOARD_HEIGHT 16

/*
** Cell object types 
*/

enum
{
	NO_OBJECT,
	WALL_OBJECT,
	BOX_OBJECT
};	

enum
{
	NORMAL_FLOOR,
	FLAGSTONE_FLOOR
};	

/*
** Variable types
*/

typedef struct cellObject
{
	BYTE type;
	BYTE id;
} cellObject;	

typedef struct Cell
{
	cellObject floor, object;
} Cell;

typedef struct Board
{
	Cell cells[ BOARD_HEIGHT ][ BOARD_WIDTH ];
} Board;

/*
** Prototypes
*/

VOID newGame( VOID );
BOOL canMove( WORD dirX, WORD dirY );
BOOL canPush( WORD dirX, WORD dirY );
BOOL moveCheckHero( WORD dirX, WORD dirY );

const WORD boardWidth = BOARD_WIDTH, boardHeight = BOARD_HEIGHT;

extern Board board;

/*
** Global variables
*/

extern struct Cell *heroCell; /* Pointer to hero's cell */
extern WORD placedBoxes;

#endif /* GAME_H */
