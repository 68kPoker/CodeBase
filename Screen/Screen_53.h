
#ifndef SCREEN_H
#define SCREEN_H

#include <exec/types.h>

#include "Game.h"

#define UP_KEY      0x4c
#define DOWN_KEY    0x4d
#define RIGHT_KEY   0x4e
#define LEFT_KEY    0x4f

#define ESC_KEY     0x45

#define SCREEN_WIDTH  20
#define SCREEN_HEIGHT 16

#define MAX_WINDOWS   5
#define MAX_ACTIONS   10

#define MAX_TILE FRUIT_TILE

enum
{
    CLOSE_ACTION=1,
    OPEN_MENU,
    SELECT_TILE
};

enum
{
    CLOSE_GAD=1,
    CLOSE_GAD_PUSHED,
    BIG_BUTTON,
    BIG_BUTTON_PUSHED,
    FLOOR_TILE,
    WALL_TILE,
    SKULL_TILE,
    PLACE_TILE,
    HERO_TILE,
    DOOR_TILE,
    KEY_TILE,
    FRUIT_TILE,
    PLACED_TILE
};

enum
{
    BACK_WIN,
    MENU_WIN
};

enum
{
    SAFE_TO_DRAW,
    COPPER_SYNC,
    USER_MESSAGE,
    /* JOYSTICK */
    SIGNAL_SOURCES
};

/* Attached to interrupt */

struct copISData
{
    struct ViewPort *vp;
    WORD signal;
    struct Task *task;
};

struct tile
{
    UBYTE gfx, action; /* Graphics and action */
    UBYTE update, aux;
    UBYTE delay, newgfx;
};

struct window
{
    WORD id;
    UWORD left, top, width, height; /* In tiles */
    struct tile *array; /* Graphics + actions */

    void (*action[MAX_ACTIONS])(struct Screen *s, struct window *w, struct tile *t, BOOL up);

    BOOL close;
    struct tile *active;
};

struct windowData
{
    UBYTE array[SCREEN_HEIGHT][SCREEN_WIDTH]; /* Window ID */
    struct window windows[MAX_WINDOWS];
};

/* Attached to screen */

struct screenData
{
    struct ScreenBuffer *sb[2];
    struct MsgPort *mp;
    BOOL safe;
    UWORD frame;
    struct Interrupt *is;
    WORD signal;
    struct Window *w;
    struct BitMap *gfx;
    struct windowData win;
    UWORD tile;
    struct gameBoard board;
};

struct Screen *openScreen(void);
void closeScreen(struct Screen *s);

void safeToDraw(struct Screen *s);
void changeScreen(struct Screen *s);

#endif /* SCREEN_H */
