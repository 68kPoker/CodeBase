
#include "Tiles.h"

/* Default icons for tile types conversion function */
WORD tileToIcon( TILE tile )
{
    WORD icon;

    switch( tile.type ) {
        case BOX:   icon = ICON_BOX_1   + tile.subType; break;
        case PLACE: icon = ICON_PLACE_1 + tile.subType; break;
        case ITEM:  icon = ICON_GOLD    + tile.subType; break;
        case ARROW: icon = ICON_UP      + tile.subType; break;
        default:    icon = tile.type; break; /* Basic */
    }
    return( icon );
}

/* Default tile types for icons */
TILE iconToTile( WORD icon )
{
    TILE tile;

    switch( icon ) {
        case ICON_BOX_1:
        case ICON_BOX_2:
        case ICON_BOX_3:
            tile.type    = BOX;
            tile.subType = icon - ICON_BOX_1;
            break;
        case ICON_PLACE_1:
        case ICON_PLACE_2:
        case ICON_PLACE_3:
            tile.type    = PLACE;
            tile.subType = icon - ICON_PLACE_1;
            break;
        case ICON_GOLD:
        case ICON_FRUIT:
        case ICON_SKULL:
            tile.type    = ITEM;
            tile.subType = icon - ICON_GOLD;
            break;
        case ICON_UP:
        case ICON_RIGHT:
        case ICON_DOWN:
        case ICON_LEFT:
            tile.type    = ARROW;
            tile.subType = icon - ICON_UP;
            break;
        default:
            /* Basic */
            tile.type    = icon;
            tile.subType = 0;
            break;
    }
    return( tile );
}
