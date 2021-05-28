
#include <stdio.h>
#include "debug.h"

#include <intuition/screens.h>
#include <exec/memory.h>
#include <graphics/rpattr.h>
#include <libraries/asl.h>

#include <clib/asl_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/layers_protos.h>

#include "Game.h"
#include "Game_protos.h"

#include "System.h"
#include "System_protos.h"

#include "Engine.h"
#include "Engine_protos.h"

#include "IFF.h"
#include "IFF_protos.h"

#include "Joystick.h"
#include "Joystick_protos.h"

#include "Screen.h"
#include "Screen_protos.h"

#include "WinGad.h"
#include "WinGad_protos.h"

UWORD pens[NUMDRIPENS + 1] = { 0 };

ULONG cols[] = { 4 << 16,
    RGB(164), RGB(98), RGB(65),
    RGB(75), RGB(17), RGB(0),
    RGB(255), RGB(255), RGB(255),
    RGB(231), RGB(173), RGB(137),
    0 };

/* Show menu. TODO: Implement in game routines and allow interaction. */
struct Window *menu(SYSINFO *data, WORD gid, WORD x, WORD y)
{
    WIN *menu;

    D(bug("menu:\n"));

    if (data->activeMenu != -1)
    {
        closeMenu(data->menu);
        data->menu = NULL;
    }

    if (gid != data->activeMenu)
    {
        if (menu = data->menu = openMenu(data->s, data->data->img, data->data->gfx, gid, x, y))
        {
            data->activeMenu = gid;
            return(menu);
        }
    }
    data->activeMenu = -1;
    return(NULL);
}

/* initData: Setup basic system resources. */
SYSINFO *initSys(SYSDATA *sysdata)
{
    SYSINFO *data;

    D(bug("initSys:\n"));
    if (data = AllocMem(sizeof(*data), MEMF_PUBLIC|MEMF_CLEAR))
    {
        GFX *gfx = sysdata->gfx;
        SCR *scr;

        data->data = sysdata;

        if (data->s = scr = openScreen(sysdata))
        {
            Class *cl;

            if (data->data->cl = cl = makeImage())
            {
                if (initImages(sysdata->img, cl, gfx->bitmap))
                {
                    WIN *win;
                    SCRINFO *si = (SCRINFO *)scr->UserData;

                    /* scr->RastPort.BitMap = si->bitmaps[1]; */

                    if (data->main = openWindow(scr, data->data->img))
                    {
                        if (data->board = win = openBoardWindow(scr))
                        {
                            if (data->joy = openJoystick())
                            {
                                /* Set buffer */
                                /* win->RPort->BitMap = si->bitmaps[1]; */

                                data->activeMenu = -1;
                                return(data);
                            }
                            CloseWindow(data->board);
                        }
                        closeWindow(data->main);
                    }
                    freeImages(data->data->img, data->data->imgCount);
                }
                FreeClass(cl);
            }
            closeScreen(scr);
        }
        FreeMem(data, sizeof(*data));
    }
    return(NULL);
}

/* freeData: Free all resources */
void freeSys(SYSINFO *data)
{
    D(bug("freeSys:\n"));

    closeJoystick(data->joy);
    CloseWindow(data->board);
    closeWindow(data->main);
    freeImages(data->data->img, data->data->imgCount);
    FreeClass(data->data->cl);
    closeScreen(data->s);
    FreeMem(data, sizeof(*data));
}

/* mainLoop: Main signal handling loop */
void mainLoop(SYSINFO *data)
{
    /* Signal masks */
    D(bug("mainLoop:\n"));
    ULONG signals[SOURCES] =
    {
        1L << data->main->UserPort->mp_SigBit,
        1L << data->board->UserPort->mp_SigBit,
        0,
        1L << data->joy->io->io_Message.mn_ReplyPort->mp_SigBit
    };

    enum
    {
        MAINWIN,
        BOARDWIN,
        MENUWIN
    };

    /* Window User Port */
    struct MsgPort *up[] =
    {
        data->main->UserPort,
        data->board->UserPort,
        NULL
    };

    /* All signals ORed together */
    ULONG total = 0L;
    WORD i;

    for (i = 0; i < SOURCES; i++)
    {
        total |= signals[i];
    }

    data->done = FALSE;

    /* While not done */
    while (!data->done)
    {
        /* Wait for all signals */
        ULONG result = Wait(total);

        /* Handle signals depending on the result */

        /* Joystick handler */
        if (result & signals[JOYSTICK])
        {
            /* Joystick event received: read next event */
            readEvent(data->joy);

            if (data->handleJoystick)
            {
                data->handleJoystick(data, &data->joy->ie);
            }
        }

        /* UserPort handler */
        if (result & signals[MAIN_USERPORT])
        {
            if (data->handleMainUserPort)
            {
                data->handleMainUserPort(data, up[MAINWIN]);
            }
        }

        if (result & signals[BOARD_USERPORT])
        {
            if (data->handleBoardUserPort)
            {
                data->handleBoardUserPort(data, up[BOARDWIN]);
            }
        }

        if (result & signals[MENU_USERPORT])
        {
            if (data->handleMenuUserPort)
            {
                data->handleMenuUserPort(data, up[MENUWIN]);
            }
        }

        if (data->menu)
        {
            up[MENUWIN] = data->menu->UserPort;
            signals[MENU_USERPORT] = 1L << up[MENUWIN]->mp_SigBit;
            total |= signals[MENU_USERPORT];
        }
        else
        {
            total &= ~signals[MENU_USERPORT];
            signals[MENU_USERPORT] = 0L;
        }

    }
    if (data->menu)
    {
        closeMenu(data->menu);
    }
}
