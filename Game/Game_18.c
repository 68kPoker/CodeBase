
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <libraries/iffparse.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>

#include "IFF.h"
#include "ILBM.h"
#include "GUI.h"

#define ESC_KEY 0x45

enum
{
    GID_SKILL,
    GADGETS
};

void drawTitle(struct BitMap *gfx, struct BitMap *bitmaps[])
{
    struct RastPort rp;
    WORD x, y;

    BltBitMap(gfx, 0, 0, bitmaps[0], 0, 0, 320, 16, 0xc0, 0xff, NULL);
    BltBitMap(gfx, 240, 16, bitmaps[0], 240, 16, 80, 240, 0xc0, 0xff, NULL);

/*
    InitRastPort(&rp);
    rp.BitMap = bitmaps[0];
    SetAPen(&rp, 9);
    RectFill(&rp, 0, 16, 239, 255);

    BltBitMap(gfx, 36, 34, bitmaps[0], 36, 34, 180, 17, 0xc0, 0xff, NULL);
    BltBitMap(gfx, 36, 255-195, bitmaps[0], 36, 255-195, 180, 17, 0xc0, 0xff, NULL);
*/
    for (y = 0; y < 15; y++)
    {
        for (x = 0; x < 15; x++)
        {
            if (x == 0 || x == 14 || y == 0 || y == 14)
                BltBitMap(gfx, 258, 255-221, bitmaps[0], x << 4, (y + 1) << 4, 16, 16, 0xc0, 0xff, NULL);
            else
                BltBitMap(gfx, 275, 255-221, bitmaps[0], x << 4, (y + 1) << 4, 16, 16, 0xc0, 0xff, NULL);
        }
    }
}

void playGame(struct Window *w, struct BitMap *gfx)
{
    BOOL done = FALSE;
    struct MsgPort *mp = w->UserPort;
    struct RastPort *rp = w->RPort;
    WORD tilex = 1, tiley = 0;

    while (!done)
    {
        struct IntuiMessage *msg;

        WaitPort(mp);
        while (msg = (struct IntuiMessage *)GetMsg(mp))
        {
            ULONG class = msg->Class;
            WORD code = msg->Code;
            WORD mx = msg->MouseX;
            WORD my = msg->MouseY;
            APTR iaddress = msg->IAddress;

            ReplyMsg((struct Message *)msg);

            if (class == IDCMP_RAWKEY)
            {
                if (code == ESC_KEY)
                {
                    done = TRUE;
                }
            }
            else if (class == IDCMP_MOUSEBUTTONS)
            {
                if (my >= 16)
                {
                    if (mx < 240)
                    {
                        if (code == IECODE_LBUTTON)
                        {
                            BltBitMapRastPort(gfx, 241 + (tilex * 17), 255-238 + (tiley * 17), w->RPort, mx & 0xfff0, my & 0xfff0, 16, 16, 0xc0);
                        }
                        else if (code == (IECODE_LBUTTON|IECODE_UP_PREFIX))
                        {

                        }
                    }
                    else if (mx < 309 && my < 255-204)
                    {
                        WORD ypos = (my - 16);

                        SetAPen(rp, 16);
                        Move(rp, 240 + (tilex * 17), 16 + (tiley * 17));
                        Draw(rp, 240 + (tilex * 17) + 17, 16 + (tiley * 17));
                        Draw(rp, 240 + (tilex * 17) + 17, 16 + 17 + (tiley * 17));
                        Draw(rp, 240 + (tilex * 17), 16 + 17 + (tiley * 17));
                        Draw(rp, 240 + (tilex * 17), 17 + (tiley * 17));

                        tilex = (mx - 241) / 17;
                        tiley = (my - (255 - 239)) / 17;
                        SetAPen(rp, 19);
                        Move(rp, 240 + (tilex * 17), 16 + (tiley * 17));
                        Draw(rp, 240 + (tilex * 17) + 17, 16 + (tiley * 17));
                        Draw(rp, 240 + (tilex * 17) + 17, 16 + 17 + (tiley * 17));
                        Draw(rp, 240 + (tilex * 17), 16 + 17 + (tiley * 17));
                        Draw(rp, 240 + (tilex * 17), 17 + (tiley * 17));
                    }
                }
            }
        }
    }
}

void initGadget(struct Gadget *gad, WORD id, WORD x, WORD y, WORD w, WORD h)
{

}

int main()
{
    struct BitMap *bitmaps[2];
    struct screenInfo si;
    struct IFFInfo ii;
    struct IFFHandle *iff;
    struct BitMap *gfx;
    struct ColorMap *cm;
    struct Window *w;
    static struct Gadget gadgets[GADGETS];

    if (iff = openIFF("Data/Graphics.iff", IFFF_READ, &ii))
    {
        if (parseIFF(iff, ilbmProps, ID_ILBM) == 0)
        {
            if (cm = getColorMap(iff))
            {
                if (gfx = getBitMap(iff))
                {
                    closeIFF(iff, &ii);
                    iff = NULL;
                    if (allocBitMaps(bitmaps, LORES_KEY, 5))
                    {
                        drawTitle(gfx, bitmaps);
                        if (getColors(&si, cm))
                        {
                            WaitBlit();
                            if (w = openWindow(&si, bitmaps))
                            {
                                playGame(w, gfx);
                                closeWindow(w, &si);
                            }
                            freeColors(&si);
                        }
                        freeBitMaps(bitmaps);
                    }
                    FreeBitMap(gfx);
                }
                FreeColorMap(cm);
            }
        }
        if (iff)
            closeIFF(iff, &ii);
    }
    return(RETURN_OK);
}
