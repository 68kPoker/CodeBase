
/* Magazyn: Okienka */

#ifndef WINDOWS_H
#define WINDOWS_H

#include <exec/types.h>
#include <intuition/intuition.h>

#define GAME_VERSION 2

#define BOARD_WIDTH  20
#define BOARD_HEIGHT 16

#define CURSOR_COLOR 28

#define ESC_KEY      0x45
#define UP_KEY       0x4C
#define DOWN_KEY     0x4D
#define RIGHT_KEY    0x4E
#define LEFT_KEY     0x4F

#define ID_MAGA MAKE_ID('M','A','G','A')
#define ID_NAGL MAKE_ID('N','A','G','L')
#define ID_PLAN MAKE_ID('P','L','A','N')
#define ID_STAN MAKE_ID('S','T','A','N')

#define DELAY 10

enum
{
    PAN_NEWGAME,
    PAN_LOADGAME,
    PAN_SAVEGAME,
    PAN_RESTART,
    PAN_QUIT,
    PAN_BUTTONS
};

enum
{
    REQ_OK,
    REQ_CANCEL,
    REQ_BUTTONS
};

enum
{
    IMG_BUTTON,
    IMG_PUSHED,
    IMG_IMAGES
};

enum
{
    WINACT_NONE,
    WINACT_ESC
};

/* Typy okien gîównych */
enum WindowTypes
{
    WIN_BOARD,  /* Okno z planszâ + paletâ              */
    WIN_PANEL,  /* Okno z panelem sterowania            */
    WIN_STATUS, /* Okno ze statusem (aktualny stan gry) */
    WIN_REQ,    /* Requester                            */
    WINDOWS
};

/* Typy requesterów */
enum RequesterTypes
{
    REQ_MESSAGE,  /* Okno z wiadomoôciâ */
    REQ_QUESTION, /* Okno z zapytaniem  */
    REQUESTERS
};

/* Typy zapytaï */
enum QuestionTypes
{
    QUEST_SELECT, /* Wybór   */
    QUEST_YES_NO, /* Pytanie */
    QUESTIONS
};

/* Typy wyborów */
enum SelectionTypes
{
    SELECT_NAME,  /* Wybór imienia */
    SELECT_LEVEL, /* Wybór poziomu */
    SELECTIONS
};

/* Typy kafelków */
enum TileKinds
{
    TILE_FLOOR,
    TILE_WALL,
    TILE_OBJECT,
    TILE_ITEM,
    TILES
};

enum
{
    PAL_FLOOR,
    PAL_WALL,
    PAL_BOX,
    PAL_HERO,
    PAL_FLAGSTONE,
    PAL_MUD,
    PAL_FRUIT,
    PALS
};

enum FloorKinds
{
    FLOOR_NORMAL,
    FLOOR_FLAGSTONE,
    FLOOR_FILLEDMUD,
    FLOORS
};

enum WallKinds
{
    WALL_NORMAL,
    WALL_MUD,
    WALL_DOOR,
    WALLS
};

enum ObjectKinds
{
    OBJ_BOX,
    OBJ_HERO,
    OBJ_PLACEDBOX,
    OBJECTS
};

enum
{
    ITEM_FRUIT,
    ITEM_KEY,
    ITEMS
};

typedef struct
{
    UWORD kind : 4;
    UWORD subKind : 4;
    UWORD floor : 4;
} TILE;

struct winData
{
    WORD kind;
};

/* Dane planszy */
struct boardData
{
    struct winData win;
    TILE board[BOARD_HEIGHT][BOARD_WIDTH];
    UWORD x, y; /* Pozycja bohatera */
    UWORD boxCount, placed;
    BOOL cursor;
    UWORD cursorX, cursorY; /* Pozycja kursora */
    WORD pal;
    BOOL paste;
    TILE currentTile; /* Bieûâcy kafel */
    struct BitMap *gfx; /* Grafika */
    WORD points, keys;
    WORD dir, angle;
};

struct panelData
{
    struct winData win;
    struct IntuiText texts[PAN_BUTTONS];
    struct Image images[IMG_IMAGES];
    struct Gadget gads[PAN_BUTTONS];
    struct BitMap *gfx;
};

struct reqData
{
    struct winData win;
    struct IntuiText texts[REQ_BUTTONS];
    struct Gadget gads[REQ_BUTTONS];
    struct BitMap *gfx;
};

#endif /* WINDOWS_H */
