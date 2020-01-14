
/* Main message loop */

#include <intuition/intuition.h>
#include <exec/memory.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>

#include "DBufScreen.h"
#include "IFF.h"
#include "Windows.h"

#define ESC_KEY   0x45
#define UP_KEY    0x4C
#define DOWN_KEY  0x4D
#define RIGHT_KEY 0x4E
#define LEFT_KEY  0x4F

#define BOARD_WIDTH  20
#define BOARD_HEIGHT 14

enum
{
    TILE_FLOOR,
    TILE_WALL,
    TILE_BOX,
    TILE_PLACE,
    TILE_HERO,
    TILE_HERO2,
    TILE_GOLD,
    TILE_FRUIT,
    TILE_SKULL
};

enum
{
    SAFE,
    DISP,
    USERPORT,
    SIGNALS
};

struct gameBoard
{
    UWORD tiles[BOARD_HEIGHT][BOARD_WIDTH];
    WORD herox, heroy;
};

void addBlit(struct List *list, WORD type, WORD tile, WORD x, WORD y, WORD width, WORD height)
{
    struct blitOp *op;

    if (op = AllocMem(sizeof(*op), MEMF_PUBLIC))
    {
        op->type = type;
        op->tile = tile;
        op->x    = x;
        op->y    = y;
        op->width = width;
        op->height = height;
        AddTail(list, &op->node);
    }
}

LONG moveHero(struct screenUser *su, struct gameBoard *gb, WORD dx, WORD dy)
{
    WORD hx = gb->herox, hy = gb->heroy;
    WORD *here = &gb->tiles[hy][hx], *next;

    next = &gb->tiles[hy + dy][hx + dx];
    if ((*next) == TILE_WALL || (*next) == TILE_PLACE)
        return(FALSE);

    if ((*next) == TILE_BOX)
    {
        WORD *next2 = &gb->tiles[hy + dy + dy][hx + dx + dx];
        if ((*next2) != TILE_FLOOR && (*next2) != TILE_PLACE)
            return(FALSE);

        if ((*next2) == TILE_PLACE)
            *next2 = TILE_FRUIT;
        else
            *next2 = TILE_BOX;
        addBlit(&su->blits, DRAWICON, *next2, hx + dx + dx, hy + dy + dy + 1, 16, 16);
    }

    *here = TILE_FLOOR;
    *next = TILE_HERO;

    gb->herox += dx;
    gb->heroy += dy;

    addBlit(&su->blits, DRAWICON, TILE_FLOOR, hx, hy + 1, 16, 16);
    addBlit(&su->blits, DRAWICON, TILE_HERO, hx + dx, hy + dy + 1, 16, 16);
    return(TRUE);
}

LONG handleIDCMP(struct Window *w, struct gameBoard *board)
{
    struct MsgPort *mp = w->UserPort;
    struct IntuiMessage *msg;
    struct Screen *s = w->WScreen;
    struct screenUser *su = (struct screenUser *)s->UserData;
    static LONG drawmode = FALSE;
    static WORD prevx = -1, prevy = -1;
    static WORD tile = TILE_WALL;

    while (msg = (struct IntuiMessage *)GetMsg(mp))
    {
        ULONG class = msg->Class;
        WORD code = msg->Code;
        WORD mx = msg->MouseX;
        WORD my = msg->MouseY;

        ReplyMsg((struct Message *)msg);

        if (class == IDCMP_RAWKEY)
        {
            if (code == ESC_KEY)
            {
                return(TRUE);
            }
            else if (code == LEFT_KEY)
            {
                moveHero(su, board, -1, 0);
            }
            else if (code == RIGHT_KEY)
            {
                moveHero(su, board, 1, 0);
            }
            else if (code == UP_KEY)
            {
                moveHero(su, board, 0, -1);
            }
            else if (code == DOWN_KEY)
            {
                moveHero(su, board, 0, 1);
            }
        }
        else if (class == IDCMP_MOUSEBUTTONS)
        {
            if (code == IECODE_LBUTTON)
            {
                if (my >= 16 && my < 240)
                {
                    addBlit(&su->blits, DRAWICON, tile, mx >> 4, my >> 4, 16, 16);
                    drawmode = TRUE;
                    prevx = mx >> 4;
                    prevy = my >> 4;
                    board->tiles[(my >> 4) - 1][mx >> 4] = tile;
                }
                else if (my >= 240)
                {
                    if (mx < 144)
                        tile = mx >> 4;
                }
            }
            else
                drawmode = FALSE;
        }
        else if (class == IDCMP_MOUSEMOVE)
        {
            mx >>= 4;
            my >>= 4;
            if (my >= 1 && my < 15 && drawmode && (prevx != mx || prevy != my))
            {
                addBlit(&su->blits, DRAWICON, tile, mx, my, 16, 16);
                prevx = mx;
                prevy = my;
                board->tiles[my - 1][mx] = tile;
            }
        }
    }
    return(FALSE);
}

void initBoard(struct gameBoard *gb)
{
    WORD x, y;

    for (y = 0; y < BOARD_HEIGHT; y++)
    {
        for (x = 0; x < BOARD_WIDTH; x++)
        {
            if (x == 0 || x == BOARD_WIDTH - 1 || y == 0 || y == BOARD_HEIGHT - 1)
                gb->tiles[y][x] = TILE_WALL;
            else
                gb->tiles[y][x] = TILE_FLOOR;
        }
    }
    gb->herox = 1;
    gb->heroy = 1;
    gb->tiles[1][1] = TILE_HERO;
}

LONG loop(struct Screen *s, struct Window *w)
{
    struct screenUser *su = (struct screenUser *)s->UserData;
    ULONG signals[SIGNALS], total, result;
    BOOL done = FALSE;
    static struct gameBoard board;

    initBoard(&board);
    addBlit(&su->blits, DRAWICON, TILE_HERO, board.herox, board.heroy + 1, 16, 16);

    signals[SAFE] = 1L << su->mp[0]->mp_SigBit;
    signals[DISP] = 1L << su->mp[1]->mp_SigBit;
    signals[USERPORT] = 1L << w->UserPort->mp_SigBit;

    total = signals[SAFE] | signals[DISP] | signals[USERPORT];

    changeBitMap(s);

    while (!done)
    {
        result = Wait(total);
        if (result & signals[SAFE])
        {
            drawScreen(s);
        }
        if (result & signals[DISP])
        {
            changeBitMap(s);
        }
        if (result & signals[USERPORT])
        {
            done = handleIDCMP(w, &board);
        }
    }
    return(0);
}

int main()
{
    struct BitMap *bm[2], *gfx;
    struct ColorMap *cm;
    struct Screen *s;
    struct TextAttr ta = { "topaz.font", 8, FS_NORMAL, FPF_ROMFONT | FPF_DESIGNED };
    ULONG *palette;

    if (bm[0] = AllocBitMap(320, 256, 5, BMF_DISPLAYABLE|BMF_CLEAR, NULL))
    {
        if (bm[1] = AllocBitMap(320, 256, 5, BMF_DISPLAYABLE|BMF_CLEAR, NULL))
        {
            if (cm = GetColorMap(32))
            {
                if (gfx = loadPicture("Grafika/Magazyn.iff", cm))
                {
                    if (palette = AllocMem(((32 * 3) + 2) * sizeof(ULONG), MEMF_PUBLIC))
                    {
                        palette[0] = 32L << 16;
                        GetRGB32(cm, 0, 32, palette + 1);
                        palette[(32 * 3) + 1] = 0L;

                        if (s = openScreen(320, 256, 5, LORES_KEY, bm, &ta, palette, gfx))
                        {
                            struct Window *w;

                            if (w = openBackdrop(s))
                            {
                                loop(s, w);
                                CloseWindow(w);
                            }
                            closeScreen(s);
                        }
                        FreeMem(palette, ((32 * 3) + 2) * sizeof(ULONG));
                    }
                    FreeBitMap(gfx);
                }
                FreeColorMap(cm);
            }
            FreeBitMap(bm[1]);
        }
        FreeBitMap(bm[0]);
    }
    return(0);
}
