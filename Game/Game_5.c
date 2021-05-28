
#include <dos/dos.h>
#include <exec/interrupts.h>
#include <libraries/iffparse.h>
#include <intuition/intuition.h>
#include <datatypes/pictureclass.h>

#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/intuition_protos.h>
#include <clib/exec_protos.h>

#include "Screen.h"

enum
{
    USER,
    SAFE,
    DISP
};

struct board
    gameBoard = { 0 };

void initBoard(struct BitMap *gfx, struct BitMap *bm);

WORD currentTile = WALL;
WORD heroX = 1, heroY = 2;
WORD heroDir = 1;

struct IFFHandle
    *iff,
    *openIFF(STRPTR name, LONG mode),
    *openILBM(STRPTR name);

extern struct Screen
    *screen;

extern struct BitMap
    *graphics, *bitmaps[2];

extern struct Interrupt
    *is;

extern struct DBufInfo
    *dbi;

struct Window
    *window,
    *openWindow(void);

void closeIFF(struct IFFHandle *iff);

struct Window *initWindow(void)
{
    struct Window *w;

    if (w = OpenWindowTags(NULL,
        WA_CustomScreen,    screen,
        WA_Left,    0,
        WA_Top,     0,
        WA_Width,   screen->Width,
        WA_Height,  screen->Height,
        WA_Backdrop,    TRUE,
        WA_Borderless,  TRUE,
        WA_Activate,    TRUE,
        WA_RMBTrap,     TRUE,
        WA_SimpleRefresh,   TRUE,
        WA_BackFill,    LAYERS_NOBACKFILL,
        WA_IDCMP,       IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE,
        WA_ReportMouse, TRUE,
        TAG_DONE))
    {
        return(w);
    }
    return(NULL);
}

BOOL move(WORD dx, WORD dy)
{
    switch (gameBoard.tiles[heroY + dy][heroX + dx].type)
    {
        case WALL:
        case FRUIT:
        case FLAGSTONE:
            return(FALSE);
        case BOX:
            switch (gameBoard.tiles[heroY + dy + dy][heroX + dx + dx].type)
            {
                case FRUIT:
                    if (gameBoard.tiles[heroY + dy][heroX + dx].counter == 3)
                    {
                        return(FALSE);
                    }
                    gameBoard.tiles[heroY + dy + dy][heroX + dx + dx].counter = gameBoard.tiles[heroY + dy][heroX + dx].counter + 1;
                    gameBoard.tiles[heroY + dy + dy][heroX + dx + dx].type = BOX;
                    gameBoard.tiles[heroY + dy + dy][heroX + dx + dx].flags = 2;
                    gameBoard.tiles[heroY + dy][heroX + dx].counter = 0;
                    break;
                case FLOOR:
                    gameBoard.tiles[heroY + dy + dy][heroX + dx + dx].counter = gameBoard.tiles[heroY + dy][heroX + dx].counter;
                    gameBoard.tiles[heroY + dy + dy][heroX + dx + dx].type = BOX;
                    gameBoard.tiles[heroY + dy + dy][heroX + dx + dx].flags = 2;
                    gameBoard.tiles[heroY + dy][heroX + dx].counter = 0;
                    break;
                case FLAGSTONE:
                    printf("+%d points\n", gameBoard.tiles[heroY + dy][heroX + dx].counter);
                    gameBoard.tiles[heroY + dy][heroX + dx].counter = 0;

                    if ((gameBoard.tiles[heroY + dy + dy][heroX + dx + dx].counter += 1) == 4)
                    {
                        gameBoard.tiles[heroY + dy + dy][heroX + dx + dx].type = FLOOR;
                        gameBoard.tiles[heroY + dy + dy][heroX + dx + dx].counter = 0;
                    }
                    gameBoard.tiles[heroY + dy + dy][heroX + dx + dx].flags = 2;
                    break;
                default:
                    return(FALSE);
            }
            break;
    }

    gameBoard.tiles[heroY + dy][heroX + dx].type = HERO;
    gameBoard.tiles[heroY + dy][heroX + dx].flags = 2;
    gameBoard.tiles[heroY][heroX].type = FLOOR;
    gameBoard.tiles[heroY][heroX].flags = 2;
    return(TRUE);
}

BOOL handleUser(struct MsgPort *up)
{
    struct IntuiMessage *msg;
    BOOL done = FALSE;
    static BOOL paint = FALSE;
    static WORD prevx = -1, prevy = -1;

    while ((!done) && (msg = (struct IntuiMessage *)GetMsg(up)))
    {
        WORD tx = msg->MouseX >> 4, ty = msg->MouseY >> 4;

        if (msg->Class == IDCMP_RAWKEY)
        {
            WORD prevHeroX = heroX, prevHeroY = heroY;
            if (msg->Code == 0x45)
            {
                done = TRUE;
            }
            else if (msg->Code == 0x4e)
            {
                if (move(1, 0))
                {
                    heroX++;
                }
            }
            else if (msg->Code == 0x4f)
            {
                if (move(-1, 0))
                {
                    heroX--;
                }
            }
            else if (msg->Code == 0x4c)
            {
                if (move(0, -1))
                {
                    heroY--;
                }
            }
            else if (msg->Code == 0x4d)
            {
                if (move(0, 1))
                {
                    heroY++;
                }
            }
            if (prevHeroX != heroX || prevHeroY != heroY)
            {
                gameBoard.tiles[prevHeroY][prevHeroX].type = FLOOR;
                gameBoard.tiles[prevHeroY][prevHeroX].flags = 2;

                gameBoard.tiles[heroY][heroX].type = HERO;
                gameBoard.tiles[heroY][heroX].flags = 2;

                if (heroX < prevHeroX)
                    heroDir = 1;
                else if (heroX > prevHeroX)
                    heroDir = 2;
                else if (heroY < prevHeroY)
                    heroDir = 3;
                else if (heroY > prevHeroY)
                    heroDir = 4;
            }
        }
        else if (msg->Class == IDCMP_MOUSEBUTTONS)
        {
            if (msg->Code == (IECODE_LBUTTON|IECODE_UP_PREFIX))
            {
                paint = FALSE;
            }

            if (ty == 0)
            {
                if (tx >= 5 && tx <= 10)
                {
                    if (msg->Code == IECODE_LBUTTON)
                    {
                        currentTile = tx - 5;
                    }
                }
            }
            else if (msg->Code == IECODE_LBUTTON)
            {
                if (tx >= 1 && ty >= 2 && tx < 19 && ty < 15)
                {
                    if (currentTile == HERO)
                    {
                        gameBoard.tiles[heroY][heroX].type = FLOOR;
                        gameBoard.tiles[heroY][heroX].flags = 2;
                        heroX = tx;
                        heroY = ty;
                        gameBoard.tiles[heroY][heroX].type = HERO;
                        gameBoard.tiles[heroY][heroX].flags = 2;
                    }
                    else if (tx != heroX || ty != heroY)
                    {
                        gameBoard.tiles[ty][tx].type = currentTile;
                        gameBoard.tiles[ty][tx].flags = 2;
                        paint = TRUE;
                        prevx = tx;
                        prevy = ty;
                    }
                }
            }
        }
        else if (msg->Class == IDCMP_MOUSEMOVE)
        {
            if (paint && (tx != prevx || ty != prevy))
            {
                if (tx >= 1 && ty >= 2 && tx < 19 && ty < 15)
                {
                    if (tx != heroX || ty != heroY)
                    {
                        gameBoard.tiles[ty][tx].type = currentTile;
                        gameBoard.tiles[ty][tx].flags = 2;
                        prevx = tx;
                        prevy = ty;
                    }
                }
            }
        }
        ReplyMsg((struct Message *)msg);
    }
    return(done);
}

void handleSafe(APTR *safe, APTR *frame, struct MsgPort *sp)
{
    static BOOL drawn = FALSE;
    struct RastPort *rp = window->RPort;

    if (!*safe)
    {
        while (!GetMsg(sp))
        {
            WaitPort(sp);
        }
        *safe = (APTR)TRUE;
    }

    /* Draw here */

    if (!drawn)
    {
        BltBitMap(bitmaps[(LONG)(*frame) ^ 1], 0, 0, bitmaps[(LONG)*frame], 0, 0, screen->Width, 16, 0xc0, 0xff, NULL);
        drawn = TRUE;
    }
    rp->BitMap = bitmaps[(LONG)*frame];

    initBoard(graphics, rp->BitMap);
}

void handleDisp(APTR *safe, APTR *frame)
{
    if (*safe)
    {
        ChangeVPBitMap(&screen->ViewPort, bitmaps[(LONG)*frame], dbi);

        *safe = FALSE;
        (LONG)*frame ^= 1;
    }
}

void loop(void)
{
    struct MsgPort *up = window->UserPort, *sp = dbi->dbi_SafeMessage.mn_ReplyPort;
    struct copper *cop = (struct copper *)is->is_Data;
    ULONG signals[] =
    {
        1L << up->mp_SigBit,
        1L << sp->mp_SigBit,
        1L << cop->signal
    }, total = signals[USER]|signals[SAFE]|signals[DISP];
    BOOL done = FALSE;

    handleSafe(&dbi->dbi_UserData1, &dbi->dbi_UserData2, sp);

    while (!done)
    {
        ULONG result = Wait(total);

        if (result & signals[USER])
        {
            done = handleUser(up);
        }

        if (result & signals[SAFE])
        {
            handleSafe(&dbi->dbi_UserData1, &dbi->dbi_UserData2, sp);
        }

        if (result & signals[DISP])
        {
            handleDisp(&dbi->dbi_UserData1, &dbi->dbi_UserData2);
        }
    }
}

int main(void)
{
    struct IFFHandle *iff;

    if (iff = openILBM("Data/Graphics.iff"))
    {
        if (initScreen(320, 256, 5, iff))
        {
            if (window = initWindow())
            {
                loop();
                CloseWindow(window);
            }
        }
        closeIFF(iff);
    }
    return(RETURN_OK);
}

struct IFFHandle *openIFF(STRPTR name, LONG mode)
{
    struct IFFHandle *iff;
    LONG dosmodes[] =
    {
        MODE_OLDFILE,
        MODE_NEWFILE
    };

    if (iff = AllocIFF())
    {
        if (iff->iff_Stream = Open(name, dosmodes[mode & 1]))
        {
            InitIFFasDOS(iff);
            if (OpenIFF(iff, mode) == 0)
            {
                return(iff);
            }
            Close(iff->iff_Stream);
        }
        FreeIFF(iff);
    }
    return(NULL);
}

void closeIFF(struct IFFHandle *iff)
{
    CloseIFF(iff);
    Close(iff->iff_Stream);
    FreeIFF(iff);
}

struct IFFHandle *openILBM(STRPTR name)
{
    struct IFFHandle *iff;
    LONG props[] =
    {
        ID_ILBM, ID_BMHD,
        ID_ILBM, ID_CMAP
    };

    if (iff = openIFF(name, IFFF_READ))
    {
        if (PropChunks(iff, props, 2) == 0)
        {
            if (StopChunk(iff, ID_ILBM, ID_BODY) == 0)
            {
                if (StopOnExit(iff, ID_ILBM, ID_FORM) == 0)
                {
                    if (ParseIFF(iff, IFFPARSE_SCAN) == 0)
                    {
                        return(iff);
                    }
                }
            }
        }
        CloseIFF(iff);
    }
    return(NULL);
}
