
#include <intuition/intuition.h>
#include <exec/types.h>

#define MAP_WIDTH  18
#define MAP_HEIGHT 15

/* Gadgets in editor */

enum
{
    GID_BOARD, /* Board editor */
    GID_SELECT, /* Tile selector */
    GID_PANEL, /* Map loader/saver etc. */
    GID_COUNT
};

struct map
{
    WORD tiles[MAP_HEIGHT][MAP_WIDTH];
};

/* Editor consists of four main functions: */

/* Place current tile on the board */
extern void placeTile(WORD x, WORD y);

/* Select current tile */
extern void selectTile(WORD tile);

/* Load and save map */
extern void saveMap(STRPTR name);

extern void loadMap(STRPTR name);

/* Editor has few variables: */

extern WORD currentTile; /* Current tile type */
extern WORD currentX, currentY; /* Current position */

extern struct map map; /* Currently edited map */

extern struct Gadget editGads[GID_COUNT]; /* Editor gadgets */

extern void initEditGadgets(void);

extern struct Window *openEditWindow(struct Screen *s);
