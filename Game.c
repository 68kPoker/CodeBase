
/*
** Magazyn (Warehouse)
** Game.c
*/

#include <exec/types.h>

#include "Game.h"

static void moveHero( WORD dirX, WORD dirY );
static void moveObject( Cell *cell, WORD dirX, WORD dirY );
static void clearObject( Cell *cell );

/*
** Global variables
*/

Board board;

struct Cell *heroCell = &board.cells[ 1 ][ 1 ]; /* Pointer to hero's cell */
WORD placedBoxes = 0;

/*
** Reset board.
*/
VOID newGame( VOID )
{
	WORD x, y;
	cellObject floor = { NORMAL_FLOOR }, wall = { WALL_OBJECT };
	
	for( y = 0; y < BOARD_HEIGHT; y++ )
	{
		for( x = 0; x < BOARD_WIDTH; x++ )
		{
			board.cells[ y ][ x ].floor = floor;
			if( x == 0 || y == 0 || x == BOARD_WIDTH - 1 || y == BOARD_HEIGHT - 1 )
			{
				board.cells[ y ][ x ].object = wall;
			}
		}	
	}
	heroCell = &board.cells[ 1 ][ 1 ];
	placedBoxes = 0;	
}

/*
** moveCheckHero moves hero in specified direction with checking.
** Returns TRUE if move was done.
*/
BOOL moveCheckHero( WORD dirX, WORD dirY )
{
	if( canMove( dirX, dirY ) ) 
	{
		moveHero( dirX, dirY );
		return( TRUE );
	}
	return( FALSE );
}

/*
** canMove checks if hero can move.
*/
BOOL canMove( WORD dirX, WORD dirY )
{
	Cell *target = heroCell + ( dirY * boardWidth ) + dirX;
	
	switch( target->object.type )
	{
		case NO_OBJECT:
			return( TRUE );
		
		case BOX_OBJECT:
			return( canPush( dirX, dirY ) );
			
		default:	
			return( FALSE );
	}		
}

BOOL canPush( WORD dirX, WORD dirY )
{
	Cell *target = heroCell + ( dirY * boardWidth ) + dirX;
	Cell *pastTarget = target + ( dirY * boardWidth ) + dirX;
	
	switch( pastTarget->object.type )
	{
		case NO_OBJECT:
			switch( target->floor.type )
			{
				case FLAGSTONE_FLOOR:
					placedBoxes--;
					break;
			}		
			
			switch( pastTarget->floor.type )
			{
				case FLAGSTONE_FLOOR:
					placedBoxes++;
					break;
			}			
			
			return( TRUE );
			
		default:
			return( FALSE );
	}	
}

/* 
** moveHero moves hero without checking (private).
*/
static void moveHero( WORD dirX, WORD dirY )
{
	moveObject( heroCell, dirX, dirY );
	clearObject( heroCell );
}

/* 
** moveObject copies object in given direction.
*/
static void moveObject( Cell *cell, WORD dirX, WORD dirY )
{
	Cell *target = cell + ( dirY * boardWidth ) + dirX;
	
	target->object = cell->object;
}

/*
** clearObject discards object.
*/
static void clearObject( Cell *cell )
{	
	cellObject noObject = { NO_OBJECT };

	cell->object = noObject;
}	
