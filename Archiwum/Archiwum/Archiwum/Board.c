
#include "Board.h"

/* Clear board struct */
void clearBoard( struct Board *board )
{
	UWORD x, y;
	TILE *tile = &board->tileBoard[ 0 ][ 0 ];

	for( y = 0; y < BOARD_HEIGHT; y++ )
		for( x = 0; x < BOARD_WIDTH; x++ ) {

			tile->floor.type = FT_NORMAL;
			tile->object.moved = TRUE;

			if( x == 0 || x == BOARD_WIDTH - 1 || y == 0 || y == BOARD_HEIGHT - 1)
				/* Place wall */
				tile->object.type = OT_WALL;
			else
				tile->object.type = OT_NONE;
			tile++;
		}

	/* Place player and exit */

	tile = &board->tileBoard[ 1 ][ 1 ];
	tile->object.type = OT_PLAYER;
	tile->object.info = &board->playerInfo;

	tile = &board->tileBoard[ BOARD_WIDTH - 2 ][ BOARD_HEIGHT - 1 ];
	tile->object.type = OT_EXIT;

	board->playerInfo.pos.x = 1;
	board->playerInfo.pos.y = 1;
}

/* Reposition player */
void placePlayer( struct Board *board, UWORD x, UWORD y)
{
	UWORD *prev_x = &board->playerInfo.pos.x;
	UWORD *prev_y = &board->playerInfo.pos.y;

	TILE *tile = &board->tileBoard[ *prev_y ][ *prev_x ];
	tile->object.type = OT_NONE;

	tile = &board->tileBoard[ y ][ x ];
	tile->object.type = OT_PLAYER;

	*prev_x = x;
	*prev_y = y;
}

BOOL moveObject( struct Board *board, UWORD x, UWORD y, WORD dir_x, WORD dir_y )
{
	TILE *tile = &board->tileBoard[ y ][ x ];

	x += dir_x;
	y += dir_y;

	TILE *target = &board->tileBoard[ y ][ x ];

	if( !target->processed )
		/* Process it first */
		processTile( board, x, y, target );

	if( tile->object.moved )
		return( FALSE );

	switch( target->object.type ) {
		case OT_NONE:
			/* Can move */
			target->object = tile->object;
			target->processed = TRUE;
			tile->object.type = OT_NONE;

			tile->object.moved = TRUE;
			target->object.moved = TRUE;

			if( target->floor.type == FT_FLAGSTONE )
				board->boxesPlaced++;

			if( tile->floor.type == FT_FLAGSTONE )
				board->boxesPlaced--;

			return( TRUE );

		default:
			return( FALSE );
	}
}

BOOL movePlayer( struct Board *board )
{

	struct ObjectInfo *objInfo = &board->playerInfo;
	UWORD x = objInfo->pos.x;
	UWORD y = objInfo->pos.y;
	WORD dir_x = board->dir_x;
	WORD dir_y = board->dir_y;

	TILE *tile = &board->tileBoard[ y ][ x ];

	x += dir_x;
	y += dir_y;

	TILE *target = &board->tileBoard[ y ][ x ];

	/* Process target first! */
	if( !target->processed )
		processTile( board, x, y, target );

	if( tile->object.moved )
		return( FALSE );

	switch( target->object.type ) {
		case OT_NONE:
			break;

		case OT_KEY:
			board->keysGot++;
			break;

		case OT_DOOR:
			if( board->keysGot == 0 )
				return( FALSE );
			board->keysGot--;
			break;

		case OT_BOX:
			/* Check if we can push */
			if( moveObject( board, x, y, dir_x, dir_y ) )
				break;
			return( FALSE );

		default:
			return( FALSE );
	}
	target->object = tile->object;
	board->playerInfo.pos.x += dir_x;
	board->playerInfo.pos.y += dir_y;
	target->processed = TRUE;
	target->object.moved = TRUE;
	tile->object.moved = TRUE;

	tile->object.type = OT_NONE;
	return( TRUE );
}

LONG processTile( struct Board *board, UWORD x, UWORD y, TILE *tile )
{
	tile->processed = TRUE;
	switch( tile->object.type ) {
		case OT_PLAYER:
			/* Move player if requested */
			if( board->dir_x || board->dir_y )
			{
				movePlayer( board );
			}

			break;

		case OT_BOX:
			/* Move box if slided */
			if( tile->floor.type == FT_SLIDER ) {
				WORD dir_x = 0, dir_y = 0;
				switch( tile->floor.info.direction ) {
					case SD_UP: 	dir_y = -1; break;
					case SD_DOWN: 	dir_y =  1; break;
					case SD_RIGHT: 	dir_x =  1; break;
					case SD_LEFT: 	dir_x = -1; break;
				}
				moveObject( board, x, y, dir_x, dir_y );
			}
			break;
	}
}

BOOL scanBoard( struct Board *board )
{
	UWORD x, y;

	board->keysGot = 0;
	board->boxesAll = 0;
	board->boxesPlaced = 0;

	for( y = 0; y < BOARD_HEIGHT; y++ ) {
		for( x = 0; x < BOARD_WIDTH; x++ ) {
			TILE *tile = &board->tileBoard[ y ][ x ];

			if( tile->object.type == OT_BOX )
				board->boxesAll++;

		}
	}
	return( TRUE );
}

/* Animate board */
void animateBoard( struct Board *board )
{
	UWORD x, y;

	for( y = 0; y < BOARD_HEIGHT; y++ ) {
		for( x = 0; x < BOARD_WIDTH; x++ ) {
			TILE *tile = &board->tileBoard[ y ][ x ];

			/* Mark as not yet processed */
			tile->processed = FALSE;
			tile->object.moved = FALSE;
		}
	}

	for( y = 0; y < BOARD_HEIGHT; y++ ) {
		for( x = 0; x < BOARD_WIDTH; x++ ) {
			TILE *tile = &board->tileBoard[ y ][ x ];

			if( !tile->processed ) {
				/* Process if it has not been processed yet */

				processTile( board, x, y, tile );
			}
		}
	}
}
