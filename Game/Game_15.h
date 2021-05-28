
#ifndef GAME_H
#define GAME_H

#ifndef GRAPHICS_GFX_H
#include <graphics/gfx.h>
#endif

#define RAS_WIDTH  320
#define RAS_HEIGHT 256
#define RAS_DEPTH  5
#define RAS_FLAGS  BMF_INTERLEAVED|BMF_CLEAR

#define MODEID     LORES_KEY

#define RGB(v) ((v)|((v)<<8)|((v)<<16)|((v)<<24))

/* Game template consists of graphics, graphics elements,
   bitmaps, screen, windows, gadgets, controllers and handlers. */

enum
{
    USER_PORT,  /* User port (IDCMP)    */
    SAFE_PORT,  /* Safe to write port   */
    DISP_PORT,  /* Safe to change port  */
    JOY_PORT,   /* Joystick port        */
    TIME_PORT,  /* Timer port           */
    MSG_PORTS   /* Count                */
};

struct gameTemplate
{
    struct gameGraphics *gfx; /* A list of graphics */
    struct gameBitMap   *bm;  /* A list of bitmaps  */
    struct gameScreen   *s;   /* A list of screens  */
    struct gameGels     *gels; /* A list of GELs    */
    struct gameControl  *con; /* A list of controls */
    struct gameHandler  *hnd; /* A list of handlers */
};

struct gameGraphics
{
    struct gameGraphics *next;
    struct BitMap       *bm;
    PLANEPTR            mask;
    ULONG               *colors;
    LONG                colsize;
};

struct gameBitMap
{
    struct gameBitMap   *next;
    struct BitMap       *bm;
    struct ScreenBuffer *sb;
};

struct gameScreen
{
    struct gameScreen   *next;
    struct Screen       *s;
    struct Window       *w;
    struct gameWindow   *windows;
};

struct gameWindow
{
    struct gameWindow   *next;
    struct Layer        *layer;
};

struct gameControl
{
    struct MsgPort      *ports[MSG_PORTS];
};

/* Obtain game template. Use it to build your game. */

struct gameTemplate *newGameTemplate(void);
struct gameBitMap   *newGameBitMap(void);
struct gameGraphics *newGameGraphics(STRPTR name);
struct gameScreen   *newGameScreen(struct gameBitMap *bm, struct gameGraphics *gfx);

void disposeGameTemplate(struct gameTemplate *p);
void disposeGameBitMap  (struct gameBitMap *p);
void disposeGameGraphics(struct gameGraphics *p);
void disposeGameScreen  (struct gameScreen *p);

#endif /* GAME_H */
