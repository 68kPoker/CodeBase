
/* Main message loop */

#include <intuition/intuition.h>
#include <exec/memory.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>

#include "DBufScreen.h"
#include "IFF.h"
#include "Windows.h"
#include "Board.h"

/* Signals */
enum {
    SAFE,
    DISP,
    USERPORT,
    SIGNALS
};

struct Library *IntuitionBase, *GfxBase, *IFFParseBase;

/* Handle IDCMP messages */
LONG handleIDCMP(struct Window *w, struct Board *board)
{
    struct MsgPort *mp = w->UserPort;
    struct IntuiMessage *msg;
    struct Screen *s = w->WScreen;
    struct screenUser *su = (struct screenUser *)s->UserData;
    static LONG drawmode = FALSE;
    static WORD prevx = -1, prevy = -1;
    static WORD curIcon = ICON_WALL;
    DIFF diff;
    COORDS coords;
    TILE tile;

    tile = iconToTile(curIcon);

    while (msg = (struct IntuiMessage *)GetMsg(mp)) {
        ULONG class = msg->Class;
        WORD code = msg->Code;
        WORD mx = msg->MouseX;
        WORD my = msg->MouseY;

        ReplyMsg((struct Message *)msg);

        if (class == IDCMP_RAWKEY) {
            if (code == ESC_KEY) {
                return(TRUE);
            }
            else if (code == LEFT_KEY) {
                diff.dx = -1;
                diff.dy = 0;
                heroMove(board, diff);
            }
            else if (code == RIGHT_KEY) {
                diff.dx = 1;
                diff.dy = 0;
                heroMove(board, diff);
            }
            else if (code == UP_KEY) {
                diff.dx = 0;
                diff.dy = -1;
                heroMove(board, diff);
            }
            else if (code == DOWN_KEY) {
                diff.dx = 0;
                diff.dy = 1;
                heroMove(board, diff);
            }
        }
        else if (class == IDCMP_MOUSEBUTTONS) {
            if (code == IECODE_LBUTTON) {
                if (my >= 16 && my < 240) {
                    /* Paint tile */

                    coords.x = mx >> 4;
                    coords.y = my >> 4;
                    drawmode = TRUE; /* Enable draw mode */
                    prevx = mx >> 4;
                    prevy = my >> 4;
                    setTile(board, coords, tile.type, tile.subType);
                }
                else if (my >= 240) {
                    /* Select tile */

                    if (mx < (ICONS << 4)) {
                        curIcon = mx >> 4;
                        tile = iconToTile(curIcon);
                    }
                }
            }
            else if (code == (IECODE_LBUTTON | IECODE_UP_PREFIX))
                drawmode = FALSE;
        }
        else if (class == IDCMP_MOUSEMOVE) {
            mx >>= 4;
            my >>= 4;
            if (my >= 1 && my < 15 && drawmode && (prevx != mx || prevy != my)) {
                prevx = mx;
                prevy = my;
                coords.x = mx;
                coords.y = my;
                setTile(board, coords, tile.type, tile.subType);
            }
        }
    }
    return(FALSE);
}

void initBoard(struct Board *gb)
{
    WORD x, y;
    TILE tile;
    COORDS coords;

    for (y = 0; y < HEIGHT; y++) {
        for (x = 0; x < WIDTH; x++) {
            coords.x = x;
            coords.y = y;
            if (x == 0 || x == WIDTH - 1 || y == 0 || y == HEIGHT - 1) {
                tile.type = WALL;
                tile.subType = 0;
                setTile(gb, coords, WALL, 0);
            }
            else {
                tile.type = FLOOR;
                tile.subType = 0;
                setTile(gb, coords, FLOOR, 0);
            }
        }
    }
    gb->heroCoords.x = 1;
    gb->heroCoords.y = 2;
    setTile(gb, gb->heroCoords, HERO, 0);
}

LONG loop(struct Screen *s, struct Window *w)
{
    struct screenUser *su = (struct screenUser *)s->UserData;
    ULONG signals[SIGNALS], total, result;
    BOOL done = FALSE;
    static struct Board board;

    board.blits = &su->blits;

    initBoard(&board);

    signals[SAFE] = 1L << su->mp[0]->mp_SigBit;
    signals[DISP] = 1L << su->mp[1]->mp_SigBit;
    signals[USERPORT] = 1L << w->UserPort->mp_SigBit;

    total = signals[SAFE] | signals[DISP] | signals[USERPORT];

    changeBitMap(s);

    while (!done) {
        result = Wait(total);
        if (result & signals[SAFE]) {
            drawScreen(s);
        }
        if (result & signals[DISP]) {
            changeBitMap(s);
        }
        if (result & signals[USERPORT]) {
            done = handleIDCMP(w, &board);
        }
    }
    return(0);
}

BOOL openLibs()
{
    if (IntuitionBase = OpenLibrary("intuition.library", 39))
    {
        if (GfxBase = OpenLibrary("graphics.library", 39))
        {
            if (IFFParseBase = OpenLibrary("iffparse.library", 39))
            {
                return(TRUE);
            }
            CloseLibrary(GfxBase);
        }
        CloseLibrary(IntuitionBase);
    }
    return(FALSE);
}

void closeLibs()
{
    CloseLibrary(IFFParseBase);
    CloseLibrary(GfxBase);
    CloseLibrary(IntuitionBase);
}

int main()
{
    struct BitMap *bm[2], *gfx;
    struct ColorMap *cm;
    struct Screen *s;
    struct TextAttr ta = { "topaz.font", 8, FS_NORMAL, FPF_ROMFONT | FPF_DESIGNED };
    ULONG *palette;

    if (!openLibs())
        return(20);

    if (bm[0] = AllocBitMap(320, 256, 5, BMF_DISPLAYABLE|BMF_CLEAR, NULL)) {
        if (bm[1] = AllocBitMap(320, 256, 5, BMF_DISPLAYABLE|BMF_CLEAR, NULL)) {
            if (cm = GetColorMap(32)) {
                if (gfx = loadPicture("Grafika/Magazyn.iff", cm)) {
                    if (palette = AllocMem(((32 * 3) + 2) * sizeof(ULONG), MEMF_PUBLIC)) {
                        palette[0] = 32L << 16;
                        GetRGB32(cm, 0, 32, palette + 1);
                        palette[(32 * 3) + 1] = 0L;

                        if (s = openScreen(320, 256, 5, LORES_KEY, bm, &ta, palette, gfx, TRUE)) {
                            struct Window *w;

                            if (w = openBackdrop(s)) {
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
    closeLibs();
    return(0);
}
