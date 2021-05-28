
#include <stdio.h>

#include "Tiles.h"

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

int test()
{
    int x, y;
    static struct board bd;
    int dx, dy;

    for( y = 0; y < B_HEIGHT; y++ ) {
        for( x = 0; x < B_WIDTH; x++ ) {
            struct field *c = &bd.array[ y ][ x ];
            if( x == 0 || y == 0 || x == B_WIDTH - 1 || y == B_HEIGHT - 1 ) {
                c->type = T_WALL;
                c->sub.wall = W_SOLID;
            }
            else {
                c->type = T_FLOOR;
                c->sub.floor = F_NORMAL;
            }
        }
    }
    bd.array[ 1 ][ 1 ].floor = F_NORMAL;
    bd.array[ 1 ][ 1 ].type = T_OBJECT;
    bd.array[ 1 ][ 1 ].sub.object = O_HERO;

    bd.array[ 2 ][ 1 ].floor = F_NORMAL;
    bd.array[ 2 ][ 1 ].type = T_OBJECT;
    bd.array[ 2 ][ 1 ].sub.object = O_BOX;

    bd.array[ 3 ][ 1 ].floor = F_FLAGSTONE;
    bd.array[ 3 ][ 1 ].type = T_FLOOR;
    bd.array[ 3 ][ 1 ].sub.floor = F_FLAGSTONE;

    bd.array[ 2 ][ 2 ].floor = F_NORMAL;
    bd.array[ 2 ][ 2 ].type = T_OBJECT;
    bd.array[ 2 ][ 2 ].sub.object = O_SCROLL;

    bd.array[ 2 ][ 3 ].floor = F_NORMAL;
    bd.array[ 2 ][ 3 ].type = T_OBJECT;
    bd.array[ 2 ][ 3 ].sub.object = O_KEY;

    bd.array[ 1 ][ 2 ].floor = F_NORMAL;
    bd.array[ 1 ][ 2 ].type = T_WALL;
    bd.array[ 1 ][ 2 ].sub.wall = W_DOOR;

    x = y = 1;
    while (scanf("%d %d", &dx, &dy) == 2) {
        printf("Moving to %d %d\n", x + dx, y + dy);
        int result = enter( &bd, &bd.array[ y + dy ][ x + dx ], &bd.array[ y ][ x ] );
        printf("Result = %d (Boxes = %d, Keys = %d)\n", result, bd.placedBoxes, bd.keys );
        if( result == R_ENTER ) {
            x += dx;
            y += dy;
        }
    }
    return( 0 );
}
