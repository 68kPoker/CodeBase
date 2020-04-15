
/* Game.h */

#include <exec/types.h>

#define BOARD_WIDTH  20
#define BOARD_HEIGHT 16

/* Template size */

#define TMP_WIDTH    20
#define TMP_HEIGHT   16

enum
{
    FLOOR_BACKGROUND,
    FLOOR_NORMAL,
    FLOOR_FLAGSTONE,
    FLOOR_WALL
};

enum
{
    OBJECT_BOX,
    OBJECT_HERO
};

struct tile
{
    UBYTE floor, objectID; /* Floor type and object identifier */
};

struct object
{
    WORD kind;
    UBYTE active;
    UBYTE id;
    WORD x, y; /* Position */
    WORD frame; /* Active frame */
    WORD offset; /* Movement offset */
};

struct board
{
    struct Screen *screen;
    struct tile tiles[BOARD_HEIGHT][BOARD_WIDTH];
    struct object *objs; /* Array of objects */
    WORD objCount; /* Object count */
    WORD boxCount; /* Amount of boxes */
    WORD placedCount; /* Amount of placed boxes */
    WORD keys; /* Amout of keys */
};

/* Create board template */

BOOL initBoard(struct board *board, WORD tmpwidth, WORD tmpheight);

BOOL loadBoard(struct board *board);

BOOL saveBoard(struct board *board);

/* Free memory */

void freeBoard(struct board *board);

void drawBoard(struct board *board);

void drawFloor(struct board *board, WORD x, WORD y);

/* Move object in given direction */

void moveObject(struct board *board, struct object *obj, WORD offsetx, WORD offsety);
