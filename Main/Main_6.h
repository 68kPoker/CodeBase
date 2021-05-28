
/* Root game structure */

#ifndef GAME_ROOT_H
#define GAME_ROOT_H

#ifndef GRAPHICS_GFX_H
#include <graphics/gfx.h>
#endif

#ifndef INTUITION_CLASSUSR_H
#include <intuition/classusr.h>
#endif

#define BUFFERS 2 /* Screen buffers count */

#define RAS_WIDTH   320
#define RAS_HEIGHT  256
#define RAS_DEPTH   5

#define SCR_WIDTH   320
#define SCR_HEIGHT  256
#define SCR_MODEID  LORES_KEY

#define ESC_KEY     0x45

#define IDCMP_FLAGS IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE|IDCMP_GADGETDOWN

#define PANEL_WIDTH  32
#define PANEL_HEIGHT 224

#define MENU_WIDTH   320
#define MENU_HEIGHT  32
#define BOARD_WIDTH  288
#define BOARD_HEIGHT 224

enum
{
    GID_BRICKS=1,
    GID_BOARD,
    GID_MENU
};

enum
{
    SCREEN_WRITE,
    SCREEN_CHANGE,
    WINDOW_IDCMP,
    JOYSTICK,
    TIMER,
    INPUTS
};

enum
{
    IDCMP_MESSAGE,
    WRITE_MESSAGE
};

struct gameRoot
{
    struct gameGraphics *graphics; /* Root graphics (graphics list) */
    struct gameScreen   *screen;
    struct gameWindow   *window; /* Root window (window list) */
    struct gameControl  *control; /* Joystick/mouse/keyboard control */
    WORD tile; /* Current tile */
    BOOL done;
};

struct gameGraphics
{
    struct gameGraphics *pred, *succ;
    Object *o;
    struct BitMap *brbitmap; /* Brush bitmap */
    PLANEPTR      mask; /* Graphics mask */
    ULONG  *colors, count;
};

struct gameScreen
{
    struct BitMap       *bitmaps[BUFFERS];
    struct Screen       *intuiScreen;
    struct ScreenBuffer *buffers[BUFFERS];
    struct MsgPort      *writePort, *changePort;
    BOOL safeToWrite, safeToChange;
    WORD frame;
};

struct gameControl
{
    struct Window       *window; /* For IDCMP, gadgets and layer */
    struct IOStdReq     *joyIO; /* Gameport.device */
    struct timerequest  *timerIO; /* Timer.device */
};

/* Own screen part */
struct gameWindow
{
    struct gameWindow *pred, *succ;
    struct Layer *layer; /* Layer for this window */
    struct gameGadget *gadgets;
    APTR userData;

    /* Window handler */
    ULONG (*handler)(struct gameRoot *gr, struct gameWindow *gw, struct gameMessage *gm);
};

struct gameGadget
{
    struct gameGadget *pred, *succ;
    struct Gadget *gadget;
};

struct newGadget
{
    WORD left, top, width, height, ID;
    APTR userData;
};

struct gameMessage
{
    WORD type;
    union
    {
        struct IntuiMessage *intuiMsg;
    } msg;
};

extern BOOL initGame(struct gameRoot *gr);
extern VOID closeGame(struct gameRoot *gr);
extern LONG playGame(struct gameRoot *gr);

extern BOOL initScreen(struct gameRoot *gr, struct gameScreen *gs);
extern VOID closeScreen(struct gameRoot *gr, struct gameScreen *gs);

extern BOOL initControl(struct gameRoot *gr, struct gameControl *gc);
extern VOID closeControl(struct gameRoot *gr, struct gameControl *gc);

extern BOOL initWindow(struct gameRoot *gr, struct gameWindow *gw);
extern VOID closeWindow(struct gameRoot *gr, struct gameWindow *gw);

extern BOOL initGadget(struct gameRoot *gr, struct gameGadget *gg, struct gameWindow *gw, struct newGadget *ng);
extern VOID closeGadget(struct gameRoot *gr, struct gameGadget *gg);

extern BOOL addPanel(struct gameRoot *gr, struct gameWindow *gw);

extern BOOL initGraphics(struct gameRoot *gr, struct gameGraphics *gfx, STRPTR name);
extern VOID closeGraphics(struct gameRoot *gr, struct gameGraphics *gfx);

#endif /* GAME_ROOT_H */
