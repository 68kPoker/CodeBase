
/* Engine.c
 *
 * Here the game engine routine is placed.
 * It consists of board converting and board engine.
 *
 * TODO: Add more results of what sound to be played upon movement,
 *       and what area to update.
 */

#include <exec/types.h>

#include "Engine.h"
#include "Game.h"
#include "Engine_protos.h"

BOOL convertBoard(struct board *bd, struct editBoard *eb)
{
    WORD x, y;
    BOOL success = TRUE;

    bd->allBoxes = 0;

    for (y = 0; y < BD_HEIGHT; y++)
    {
        for (x = 0; x < BD_WIDTH; x++)
        {
            struct field *f = &bd->array[y][x];
            switch (eb->tiles[y][x])
            {
                case TILE_FLOOR:
                    f->type = T_FLOOR;
                    f->floor = f->sub.floor = F_NORMAL;
                    break;
                case TILE_WALL:
                    f->type = T_WALL;
                    f->sub.wall = W_SOLID;
                    f->floor = F_NORMAL;
                    break;
                case TILE_BOX:
                    f->type = T_OBJECT;
                    f->sub.object = O_BOX;
                    f->floor = F_NORMAL;
                    bd->allBoxes++;
                    break;
                case TILE_PLACE:
                    f->type = T_FLOOR;
                    f->floor = f->sub.floor = F_FLAGSTONE;
                    break;
                case TILE_HERO:
                    f->type = T_OBJECT;
                    f->sub.object = O_HERO;
                    f->floor = F_NORMAL;
                    bd->x = x;
                    bd->y = y;
                    break;
            }
        }
    }
    bd->placedBoxes = 0;
    return(success);
}

int enter( struct board *bd, struct field *dest, struct field *src )
{
    int object = src->sub.object, result;
    struct field *next = dest + ( dest - src );

    switch( dest->type ) {
        case T_FLOOR:
            if( dest->sub.floor == F_FLAGSTONE && object == O_BOX )
                bd->placedBoxes++;

            result = R_ENTER;
            break;
        case T_WALL:
            if( dest->sub.wall == W_DOOR && object == O_HERO && bd->keys > 0 ) {
                bd->keys--;
                dest->type = T_FLOOR;
                dest->sub.floor = F_NORMAL;
            }
            result = R_BLOCK;
            break;
        case T_OBJECT:
            if( object == O_HERO ) {
                result = R_ENTER;
                switch( dest->sub.object ) {
                    case O_BOX:
                        result = enter( bd, next, dest );
                        break;
                    case O_KEY:
                        bd->keys++;
                        break;
                    case O_SCROLL:
                        printf("This is example scroll.\n");
                        break;
                    default:
                        result = R_BLOCK;
                }
            }
            else
                result = R_BLOCK;
            break;
    }
    if( result == R_ENTER ) {
        leave( bd, src );
        if( dest->type == T_FLOOR )
            dest->floor = dest->sub.floor;
        dest->type = T_OBJECT;
        dest->sub.object = object;

        src->type = T_FLOOR;
        src->sub.floor = src->floor;
    }
    return( result );
}

int leave( struct board *bd, struct field *src )
{
    if( src->sub.object == O_BOX && src->floor == F_FLAGSTONE )
        bd->placedBoxes--;
}
