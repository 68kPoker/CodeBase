
/* Draw various elements */

/* $Log:	draw.c,v $
 * Revision 1.1  12/.0/.0  .1:.4:.2  Robert
 * Initial revision
 *  */

#include <stdio.h>
#include <stdlib.h>
#include <exec/types.h>

#include "screen.h"
#include "iff.h"

#include <clib/graphics_protos.h>

#define BOARD_WIDTH  20
#define BOARD_HEIGHT 16

#define UPDATE_FLAG  0x8000

/* States */
enum
{
    NONE,
    WALL,
    DOOR,
    BOX,
    HERO
};

/* Floors */
enum
{
    FLOOR,
    FLAGSTONE,
    MUD,
    FILLED_MUD
};

/* Transparencies */
enum
{
    TRANSPARENT,
    SOLID
};

struct cell
{
    UWORD *state; /* This cell's state */
    UWORD *floor; /* This cell's floor */
    UWORD x, y; /* This cell's position */
};

struct board
{
    UWORD states[ BOARD_HEIGHT ][ BOARD_WIDTH ];
    UWORD floors[ BOARD_HEIGHT ][ BOARD_WIDTH ];
    WORD x, y; /* Hero position */
};

struct draw_context
{
    struct RastPort *rp;
    struct BitMap *gfx;
};

struct main_data
{
    struct screen s;
    struct board board;
    struct draw_context dc;
};

void draw_board( struct board *board, struct draw_context *target );
void for_each_cell( struct board *board, void( *command )( struct board *board, struct cell *c, APTR ), APTR );

WORD transparency( WORD state )
{
    if( state == WALL )
        return( SOLID );

    return( TRANSPARENT );
}

void draw_tile( struct RastPort *rp, struct BitMap *tilegfx, WORD state, WORD floor, WORD x, WORD y )
{
    enum
    {
        FLOOR_POS,
        OBJECT_POS,
        HIGH_POS
    };

    struct BitMap *gfx = tilegfx;

    if( state == NONE ) {
        /* No object, draw floor only */
        BltBitMapRastPort( gfx, floor << 4, FLOOR_POS << 4, rp, x << 4, y << 4, 16, 16, 0xc0 );
    }
    else if( transparency( state ) == SOLID ) {
        /* No floor, draw object only */
        BltBitMapRastPort( gfx, state << 4, OBJECT_POS << 4, rp, x << 4, y << 4, 16, 16, 0xc0 );
    }
    else
        /* Draw floor + object */
        if( floor == FLAGSTONE )
            BltBitMapRastPort( gfx, state << 4, HIGH_POS << 4, rp, x << 4, y << 4, 16, 16, 0xc0 );
        else
            BltBitMapRastPort( gfx, state << 4, OBJECT_POS << 4, rp, x << 4, y << 4, 16, 16, 0xc0 );
}

/* Draw single cell */
void draw_cell( struct board *board, struct cell *c, struct draw_context *target )
{
    if( !( *c->state & UPDATE_FLAG ) && !( *c->floor & UPDATE_FLAG ) )
        return;

    *c->state &= ~UPDATE_FLAG;
    *c->floor &= ~UPDATE_FLAG;

    draw_tile( target->rp, target->gfx, *c->state, *c->floor, c->x, c->y );
}

/* Draw the board in given context */
void draw_board( struct board *board, struct draw_context *target )
{
    /* Draw each tile on the board - draw it */
    for_each_cell( board, draw_cell, target );
}

/* Obtain pointer to first cell data */
struct cell *first_cell( struct board *board )
{
    struct cell *c;

    if( c = malloc( sizeof( *c ) ) ) {
        c->state = ( UWORD * ) board->states;
        c->floor = ( UWORD * ) board->floors;
        c->x = 0;
        c->y = 0;
    }
    return( c );
}

void free_cell( struct cell *c )
{
    if( c )
        free( c );
}

/* Iterate through cells */
struct cell *next_cell( struct board *board, struct cell *c )
{
    if( ++c->x == BOARD_WIDTH ) {
        if( ++c->y == BOARD_HEIGHT )
            return( NULL );
        c->x = 0;
    }
    ++c->state;
    ++c->floor;
    return( c );
}

/* Execute command for each cell */
void for_each_cell( struct board *board, void( *command )( struct board *board, struct cell *c, APTR context ), APTR context )
{
    struct cell *c;

    for( c = first_cell( board ); c != NULL; c = next_cell( board, c ) )
        command( board, c, context );

    free_cell( c );
}

void init_cell( struct board *board, struct cell *c, APTR context )
{
    *c->floor = FLOOR;
    if( c->x == 0 || c->x == BOARD_WIDTH - 1 || c->y == 0 || c->y == BOARD_HEIGHT - 1)
        *c->state = WALL | UPDATE_FLAG;
    else if( c->x == board->x && c->y == board->y )
        *c->state = HERO | UPDATE_FLAG;
    else
        *c->state = NONE | UPDATE_FLAG;
}

void clear_board( struct board *board )
{
    board->x = 1;
    board->y = 1;
    for_each_cell( board, init_cell, NULL );

    board->states[ 2 ][ 2 ] = board->states[2][3] = BOX | UPDATE_FLAG;
    board->floors[ 2 ][ 3 ] = FLAGSTONE | UPDATE_FLAG;
    board->floors[ 3 ][ 3 ] = MUD | UPDATE_FLAG;
}

struct cell *get_cell( struct cell *c, struct board *board, WORD x, WORD y )
{
    c->state = &board->states[ y ][ x ];
    c->floor = &board->floors[ y ][ x ];
    return( c );
}

void sibling( struct cell *s, struct cell *c, WORD dx, WORD dy )
{
    WORD offset = ( dy * BOARD_WIDTH ) + dx;

    s->state = c->state + offset;
    s->floor = c->floor + offset;
    s->x += dx;
    s->y += dy;
}

BOOL move_hero( struct board *board, WORD dx, WORD dy )
{
    WORD x = board->x, y = board->y;
    struct cell c, dest, past;
    get_cell( &c, board, x, y );
    sibling( &dest, &c, dx, dy );
    sibling( &past, &dest, dx, dy );

    switch( *dest.state )
    {
        case NONE:
            /* Only floor */
            switch( *dest.floor )
            {
                case MUD:
                    /* Mud */
                    return( FALSE );
            }
            break;
        case WALL:
            return( FALSE );
        case BOX:
            switch( *past.state )
            {
                case NONE:
                    switch( *past.floor )
                    {
                        case MUD:
                            /* Fill mud */
                            *past.floor = FILLED_MUD | UPDATE_FLAG;
                            break;
                        default:
                            /* Push box */
                            *past.state = BOX | UPDATE_FLAG;
                            break;
                    }
                    break;
                default:
                    return( FALSE );
            }
            break;
    }
    /* Move */
    *dest.state = HERO | UPDATE_FLAG;
    *c.state = NONE | UPDATE_FLAG;
    board->x += dx;
    board->y += dy;
    return( TRUE );
}

int main( void )
{
    struct main_data md;
    int dir;
    BOOL done = FALSE;
    WORD dx, dy;
    struct TextAttr ta =
    {
        "centurion.font", 9,
        FS_NORMAL,
        FPF_DISKFONT | FPF_DESIGNED
    };
    struct Rectangle dclip =
    {
        0, 0, 319, 255
    };

    clear_board( &md.board );

    if( open_screen( &md.s, 320, 256, 5, &ta, &dclip, LORES_KEY, NULL, "GearWorks", IDCMP_RAWKEY | IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE | IDCMP_GADGETUP ) )
    {
        struct IFFHandle *iff;
        if( ( iff = open_iff( "Data1/Gfx/Graphics.iff", IFFF_READ ) ) && scan_ilbm( iff ) && load_cmap( iff, md.s.s ) )
        {
            if( md.dc.gfx = load_bitmap( iff ) )
            {
                close_iff( iff );
                iff = NULL;
                md.dc.rp = md.s.w->RPort;
                md.dc.rp->RP_User = ( APTR ) md.dc.gfx;

                while( !done ) {
                    draw_board( &md.board, &md.dc );
                    if( scanf( "%d", &dir ) == 1 ) {
                        dx = dy = 0;
                        switch( dir ) {
                            case 1:
                                dx = -1;
                                break;
                            case 2:
                                dx = 1;
                                break;
                            case 3:
                                dy = -1;
                                break;
                            case 4:
                                dy = 1;
                                break;
                            default:
                                done = TRUE;
                        }
                        if( dx || dy )
                            move_hero( &md.board, dx, dy );
                    }
                    else
                        done = TRUE;
                }
                FreeBitMap( md.dc.gfx );
            }
            if( iff )
                close_iff( iff );
        }
        close_screen( &md.s );
    }
    return( 0 );
}
