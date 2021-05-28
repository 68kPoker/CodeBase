
#ifndef SYSTEM_H
#define SYSTEM_H

#include "WinGad.h"
#include "IFF.h"

#include <intuition/classes.h>
#include <intuition/classusr.h>

/* Available signals */
enum
{
    MAIN_USERPORT,
    BOARD_USERPORT,
    MENU_USERPORT,
    JOYSTICK,
    SOURCES
};

typedef struct sysInfo SYSINFO;
typedef struct sysData SYSDATA;

struct sysData /* What I want to provide for system */
{
    GFX *gfx;

    Class *cl;
    Object **img; /* Images */
    WORD imgCount;
};

struct sysInfo /* System run-time components */
{
    SYSDATA *data;

    struct Screen *s;
    struct Window *main, *board, *menu; /* Three windows */
    struct Joystick *joy;

    WORD activeMenu;

    BOOL done;

    APTR user; /* Game-specific data */

    LONG (*handleJoystick)(SYSINFO *sys, struct InputEvent *ie);
    LONG (*handleMainUserPort)(SYSINFO *sys, struct MsgPort *mp);
    LONG (*handleBoardUserPort)(SYSINFO *sys, struct MsgPort *mp);
    LONG (*handleMenuUserPort)(SYSINFO *sys, struct MsgPort *mp);
};

#endif /* SYSTEM_H */
