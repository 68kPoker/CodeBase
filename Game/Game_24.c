
/* Game.c
 *
 * Here I would like to place main() routine which:
 * - allocates system resources (GUI, audio etc.),
 * - loads common game data (graphics, sounds),
 * - enters main loop.
 */

#include <stdio.h>
#include "debug.h"

#include <dos/dos.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <graphics/rpattr.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/layers_protos.h>

#include "Game.h"
#include "System.h"
#include "Engine.h"
#include "Joystick.h"
#include "IFF.h"

#include "Game_protos.h"
#include "IFF_protos.h"
#include "System_protos.h"

UBYTE version[] = "$VER: Magazyn 1.0";

/* drawBoard: Draw static board. */
void drawBoard(GFX *gfx, WIN *win, struct editBoard *eb)
{
    WORD x, y;
    struct Rectangle rect;
    WORD width, height;

    /* Obtain drawing bounds */
    GetRPAttrs(win->RPort, RPTAG_DrawBounds, &rect, TAG_DONE);

    D(bug("drawBoard: Redrawing board (%d/%d/%d/%d)\n", rect.MinX, rect.MinY, rect.MaxX, rect.MaxY));

    /* Calc width and height in tiles */
    width = MIN(BD_WIDTH, (rect.MaxX + 15) >> 4);
    height = MIN(BD_HEIGHT - 1, (rect.MaxY + 15) >> 4);

    D(bug("drawBoard: %d, %d, %d, %d\n", rect.MinX >> 4, rect.MinY >> 4, width, height));

    for (y = rect.MinY >> 4; y <= height; y++)
    {
        for (x = rect.MinX >> 4; x <= width; x++)
        {
            WORD tile = eb->tiles[y][x];
            BltBitMapRastPort(gfx->bitmap, tile << 4, 2 << 4, win->RPort, x << 4, y << 4, 16, 16, 0xc0);
        }
    }
}

/* convertTile: Convert run-time field to editor tile for drawing */
WORD convertTile(struct field *f)
{
    if (f->type == T_WALL)
    {
        if (f->sub.wall == W_SOLID)
        {
            return(TILE_WALL);
        }
    }
    else if (f->type == T_FLOOR)
    {
        if (f->sub.floor == F_NORMAL)
        {
            return(TILE_FLOOR);
        }
        else if (f->sub.floor == F_FLAGSTONE)
        {
            return(TILE_PLACE);
        }
    }
    else if (f->type == T_OBJECT)
    {
        if (f->sub.object == O_BOX)
        {
            return(TILE_BOX);
        }
        else if (f->sub.object == O_HERO)
        {
            return(TILE_HERO);
        }
    }
    return(TILE_FLOOR);
}

/* animateBoard: Draw animated board. */
void animateBoard(GFX *gfx, WIN *win, struct board *bd)
{
    WORD x, y;
    struct Rectangle rect;
    WORD width, height;

    GetRPAttrs(win->RPort, RPTAG_DrawBounds, &rect, TAG_DONE);

    D(bug("animateBoard: Animating board (%d/%d/%d/%d)\n", rect.MinX, rect.MinY, rect.MaxX, rect.MaxY));

    width = MIN(BD_WIDTH, (rect.MaxX + 15) >> 4);
    height = MIN(BD_HEIGHT - 1, (rect.MaxY + 15) >> 4);

    D(bug("animateBoard: %d, %d, %d, %d\n", rect.MinX >> 4, rect.MinY >> 4, width, height));

    for (y = rect.MinY >> 4; y <= height; y++)
    {
        if (y == 0)
            continue;
        for (x = rect.MinX >> 4; x <= width; x++)
        {
            WORD tile = convertTile(&bd->array[y][x]);
            BltBitMapRastPort(gfx->bitmap, tile << 4, 2 << 4, win->RPort, x << 4, y << 4, 16, 16, 0xc0);
        }
    }
}

/* Clean edited board. TODO: Move to game routines */
void cleanBoard(struct editBoard *eb)
{
    WORD x, y;

    D(bug("cleanBoard:\n"));

    for (y = 0; y < BD_HEIGHT; y++)
    {
        for (x = 0; x < BD_WIDTH; x++)
        {
            if (x == 0 || x == BD_WIDTH - 1 || y == 0 || y == BD_HEIGHT - 1)
            {
                eb->tiles[y][x] = TILE_WALL;
            }
            else
            {
                eb->tiles[y][x] = TILE_FLOOR;
            }
        }
    }
}

/* Handle joystick event: move hero.
 * Install in game mode */

LONG handleJoystick(SYSINFO *data, struct InputEvent *ie)
{
    D(bug("handleJoystick:\n"));

    /* Game specific info */
    struct gameInfo *gi = (struct gameInfo *)data->user;

    /* Run-time board */
    struct board *bd = &gi->gameBoard;

    /* Hero's field */
    struct field *f = &bd->array[bd->y][bd->x];

    /* Regions for window update */
    struct Region *reg, *prev;

    /* Update rectangle */
    struct Rectangle rect = { (bd->x - 2) << 4, (bd->y - 2) << 4, ((bd->x + 3) << 4) - 1, ((bd->y + 3) << 4) - 1};

    if (!(reg = NewRegion()))
    {
        printf("NewRegion() failed!\n");
    }
    else
    {
        /* Add rectangle to region */
        OrRectRegion(reg, &rect);

        /* Install it */
        prev = InstallClipRegion(data->board->WLayer, reg);

        if (ie->ie_Code == IECODE_NOBUTTON)
        {
            /* No button - move */
            if (ie->ie_X == -1 && ie->ie_Y == 0)
            {
                /* Move left */
                if (enter(bd, &bd->array[bd->y][bd->x - 1], f) == R_ENTER)
                {
                    /* Update hero's position and animate board */
                    bd->x--;
                }
            }
            else if (ie->ie_X == 1 && ie->ie_Y == 0)
            {
                if (enter(bd, &bd->array[bd->y][bd->x + 1], f) == R_ENTER)
                {
                    bd->x++;
                }
            }
            else if (ie->ie_X == 0 && ie->ie_Y == -1)
            {
                if (enter(bd, &bd->array[bd->y - 1][bd->x], f) == R_ENTER)
                {
                    bd->y--;
                }
            }
            else if (ie->ie_X == 0 && ie->ie_Y == 1)
            {
                if (enter(bd, &bd->array[bd->y + 1][bd->x], f) == R_ENTER)
                {
                    bd->y++;
                }
            }
        }

        /* Restore previous region */
        InstallClipRegion(data->board->WLayer, prev);
        DisposeRegion(reg);
        return(TRUE);
    }
    return(FALSE);
}

LONG handleMenuUserPort(SYSINFO *data, struct MsgPort *up)
{
    D(bug("handleMenuUserPort:\n"));

    struct IntuiMessage *msg;
    GAMEINFO *gi = (GAMEINFO *)data->user;

    while (msg = (struct IntuiMessage *)GetMsg(up))
    {
        ULONG class = msg->Class;
        UWORD code = msg->Code;
        WORD mx = msg->MouseX;
        WORD my = msg->MouseY;
        APTR iaddr = msg->IAddress;

        ReplyMsg((struct Message *)msg);

        /* Gadget released. TODO: call gadget handler */
        if (class == IDCMP_GADGETUP)
        {
            struct Gadget *gad = (struct Gadget *)iaddr;
            if (gad->GadgetID == GID_CLOSE)
            {
                closeMenu(data->menu);
                data->menu = NULL;

                return(FALSE);
            }
            else if (gad->GadgetID == GID_DEPTH)
            {

            }
        }
    }
}

LONG handleMainUserPort(SYSINFO *data, struct MsgPort *up)
{
    struct IntuiMessage *msg;
    GAMEINFO *gi = (GAMEINFO *)data->user;

    D(bug("handleMainUserPort:\n"));

    while (msg = (struct IntuiMessage *)GetMsg(up))
    {
        ULONG class = msg->Class;
        UWORD code = msg->Code;
        WORD mx = msg->MouseX;
        WORD my = msg->MouseY;
        APTR iaddr = msg->IAddress;

        ReplyMsg((struct Message *)msg);

        /* Gadget released. TODO: call gadget handler */
        if (class == IDCMP_GADGETUP)
        {
            struct Gadget *gad = (struct Gadget *)iaddr;
            if (gad->GadgetID == GID_CLOSE)
            {
                data->done = TRUE;
            }
            else if (gad->GadgetID == GID_DEPTH)
            {
                ScreenToBack(data->s);
            }
            else if (gad->GadgetID == GID_EDITOR || gad->GadgetID == GID_PANEL || gad->GadgetID == GID_OPTIONS)
            {
                struct Window *res;
                WORD x = 16 + ((gad->GadgetID - GID_PANEL) * 80);
                WORD y = 16;

                /* Call menu! */

                res = menu(data, gad->GadgetID, x, y);
            }
        }
    }
}

LONG handleBoardUserPort(SYSINFO *data, struct MsgPort *up)
{
    struct IntuiMessage *msg;
    GAMEINFO *gi = (GAMEINFO *)data->user;

    while (msg = (struct IntuiMessage *)GetMsg(up))
    {
        ULONG class = msg->Class;;
        UWORD code = msg->Code;
        WORD mx = msg->MouseX;
        WORD my = msg->MouseY;
        APTR iaddr = msg->IAddress;

        ReplyMsg((struct Message *)msg);

        /* Mouse buttons and mouse move. */
        if (class == IDCMP_MOUSEBUTTONS || class == IDCMP_MOUSEMOVE)
        {
            if (class == IDCMP_MOUSEBUTTONS)
            {
                if (code == IECODE_RBUTTON)
                {
                    struct Window *res;
                    /*
                    if ((res = menu(data, GID_OPTIONS, mx, my)) != -1)
                    {
                        gi->tile = res;
                    }
                    */
                }
                else if (code == (IECODE_LBUTTON|IECODE_UP_PREFIX))
                {
                    gi->paint = FALSE;
                }
            }

            mx >>= 4;
            my >>= 4;

            if (my >= 0)
            {
                if (class == IDCMP_MOUSEBUTTONS || mx != gi->prevx || my != gi->prevy)
                {
                    if (code == IECODE_LBUTTON)
                    {
                        gi->paint = TRUE;
                    }

                    if (gi->paint)
                    {
                        BltBitMapRastPort(data->data->gfx->bitmap, gi->tile << 4, 32, data->board->RPort, mx << 4, my << 4, 16, 16, 0xc0);
                        gi->editBoard.tiles[my][mx] = gi->tile;
                    }
                    gi->prevx = mx;
                    gi->prevy = my;
                }
            }
        }
        /* Refresh window */
        else if (class == IDCMP_REFRESHWINDOW)
        {
            BeginRefresh(data->board);
            drawBoard(data->data->gfx, data->board, &gi->editBoard);
            EndRefresh(data->board, TRUE);
        }

        /* Raw key press */
        else if (class == IDCMP_RAWKEY && gi->game)
        {
            struct field *f = &gi->gameBoard.array[gi->gameBoard.y][gi->gameBoard.x];
            struct Region *reg = NewRegion(), *prev;
            struct Rectangle rect =
            {
                (gi->gameBoard.x - 2) << 4,
                (gi->gameBoard.y - 2) << 4,
                ((gi->gameBoard.x + 3) << 4) - 1,
                ((gi->gameBoard.y + 3) << 4) - 1
            };

            OrRectRegion(reg, &rect);
            prev = InstallClipRegion(data->board->WLayer, reg);

            switch (code)
            {
                case LEFT_KEY:
                    if (enter(&gi->gameBoard, &gi->gameBoard.array[gi->gameBoard.y][gi->gameBoard.x - 1], f) == R_ENTER)
                    {
                        gi->gameBoard.x--;
                        animateBoard(data->data->gfx, data->board, &gi->gameBoard);
                    }
                    break;
                case RIGHT_KEY:
                    if (enter(&gi->gameBoard, &gi->gameBoard.array[gi->gameBoard.y][gi->gameBoard.x + 1], f) == R_ENTER)
                    {
                        gi->gameBoard.x++;
                        animateBoard(data->data->gfx, data->board, &gi->gameBoard);
                    }
                    break;
                case UP_KEY:
                    if (enter(&gi->gameBoard, &gi->gameBoard.array[gi->gameBoard.y - 1][gi->gameBoard.x], f) == R_ENTER)
                    {
                        gi->gameBoard.y--;
                        animateBoard(data->data->gfx, data->board, &gi->gameBoard);
                    }
                    break;
                case DOWN_KEY:
                    if (enter(&gi->gameBoard, &gi->gameBoard.array[gi->gameBoard.y + 1][gi->gameBoard.x], f) == R_ENTER)
                    {
                        gi->gameBoard.y++;
                        animateBoard(data->data->gfx, data->board, &gi->gameBoard);
                    }
                    break;
            }
            InstallClipRegion(data->board->WLayer, prev);
            DisposeRegion(reg);
        }
    }
}

/* TODO: change to specific - next - previous level loading */
BOOL changeLevel(SYSINFO *data)
{
    static UBYTE drawer[64]="Levels";
    LEV *lev;
    UBYTE name[256];
    GAMEINFO *gi = (GAMEINFO *)data->user;

    D(bug("changeLevel:\n"));

    if (lev = loadLevel(name))
    {
        gi->editBoard = lev->eb;
        return(TRUE);
    }
    return(FALSE);
}

/* TODO: Save current level */
void saveEditedLevel(SYSINFO *data)
{
    static UBYTE drawer[64]="Levels";
    LEV lev;
    UBYTE name[256];
    GAMEINFO *gi = (GAMEINFO *)data->user;

    D(bug("saveEditedLevel:\n"));

    lev.version = 1;
    lev.width = BD_WIDTH;
    lev.height = BD_HEIGHT;
    lev.eb = gi->editBoard;

    saveLevel(&lev, name);
}

/* initGameData: Setup game data */
GAMEDATA *initGameData(STRPTR initdir)
{
    GAMEDATA *gamedata;

    D(bug("initGameData:\n"));

    if (!(gamedata = AllocMem(sizeof(*gamedata), MEMF_PUBLIC)))
    {
        printf("AllocMem() failed!\n");
    }
    else
    {
        if (gamedata->graphics = loadGraphics("Data/Common.iff"))
        {
            gamedata->syspart.gfx = gamedata->graphics;
            return(gamedata);
        }
        FreeMem(gamedata, sizeof(*gamedata));
    }
    return(NULL);
}

void freeGameData(GAMEDATA *gamedata)
{
    D(bug("freeGameData:\n"));
    freeGraphics(gamedata->graphics);
    FreeMem(gamedata, sizeof(*gamedata));
}

GAMEINFO *initGame(GAMEDATA *gd, SYSINFO *data)
{
    GAMEINFO *gi;

    D(bug("initGame:\n"));

    if (!(gi = AllocMem(sizeof(*gi), MEMF_PUBLIC|MEMF_CLEAR)))
    {
        printf("AllocMem() failed!\n");
    }
    else
    {
        /* Setup callbacks */
        data->handleJoystick = NULL;
        data->handleMainUserPort = handleMainUserPort;
        data->handleBoardUserPort = handleBoardUserPort;
        data->handleMenuUserPort = handleMenuUserPort;

        gi->tile = TILE_WALL;

        return(gi);
    }
    return(NULL);
}

void freeGame(GAMEINFO *gi)
{
    D(bug("freeGame:\n"));
    FreeMem(gi, sizeof(*gi));
}

int main()
{
    GAMEDATA *gamedata; /* All game data */
    GAMEINFO *game;
    SYSINFO *sys; /* All system resources */

    D(bug("main:\n"));

    if (gamedata = initGameData("Data"))
    {
        gamedata->syspart.img = gamedata->img;
        gamedata->syspart.imgCount = IID_COUNT;
        if (sys = initSys(&gamedata->syspart))
        {
            if (game = initGame(gamedata, sys))
            {
                sys->user = (APTR)game;
                mainLoop(sys);

                freeGame(game);
            }
            freeSys(sys);
        }
        freeGameData(gamedata);
    }
    return(RETURN_OK);
}

int wbmain(struct WBStartup *wbs)
{
    return(main());
}
