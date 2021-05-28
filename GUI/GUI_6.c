
/*
 * Podstawowe dziaîania:
 *
 * 1. Alokacja BitMap ekranu o podanym rozmiarze,
 * 2. Wczytanie grafiki oraz palety z pliku IFF,
 * 3. Przygotowanie funkcji czyszczenia ekranu,
 * 4. Otwarcie ekranu,
 * 5. Otwarcie okna powitalnego z podstawowymi dziaîaniami,
 *    a) Konwersja grafik przycisków.
 * 6. Otwarcie gîównego okna gry,
 * 7. Otwarcie panelu.
 */

#include <graphics/gfx.h>
#include <intuition/screens.h>
#include <intuition/intuition.h>
#include <graphics/rpattr.h>
#include <hardware/custom.h>
#include <hardware/blit.h>

#include <clib/intuition_protos.h>

void bltTemplate(struct BitMap *gfx, WORD left, WORD top, WORD width, WORD height, struct BitMap *bm);

struct Screen *openScreen(struct BitMap *bm, struct ColorMap *cm, struct BitMap *backbm)
{
    struct Screen *s = NULL;
    ULONG *pal;
    static struct Hook myHook =
    {
        { NULL, NULL },
        (ULONG (*)()) fastScreenBackFill,
        NULL,
        NULL
    };

    struct Hook *backFill = &myHook;

    backFill->h_Data = (APTR)backbm;

    printf("%d colors.\n", cm->Count);
    if (pal = AllocMem(((cm->Count * 3) + 2) * sizeof(ULONG), MEMF_PUBLIC))
    {
        pal[0] = cm->Count << 16;
        GetRGB32(cm, 0, cm->Count, pal + 1);
        pal[(cm->Count * 3) + 1] = 0L;

        s = OpenScreenTags(NULL,
            SA_Left,        0,
            SA_Top,         0,
            SA_BitMap,      bm,
            SA_Colors32,    pal,
            SA_Quiet,       TRUE,
            SA_Exclusive,   TRUE,
            SA_ShowTitle,   FALSE,
            SA_Draggable,   FALSE,
            SA_BackFill,    backFill,
            TAG_DONE);

        FreeMem(pal, ((cm->Count * 3) + 2) * sizeof(ULONG));
    }
    return(s);
}

struct Window *openBackdropWindow(struct Screen *s, struct BitMap *gfx, struct BitMap *bm)
{
    struct Window *w;
    static struct Hook myHook =
    {
        { NULL, NULL },
        (ULONG (*)()) fastBackFill,
        NULL,
        NULL
    };
    struct Hook *backFill = &myHook;
    WORD width = s->Width, height = s->Height;

    backFill->h_Data = (APTR)bm;

    if (w = OpenWindowTags(NULL,
        WA_CustomScreen,    s,
        WA_Left,            0,
        WA_Top,             0,
        WA_Width,           width,
        WA_Height,          height,
        WA_Borderless,      TRUE,
        WA_SimpleRefresh,   TRUE,
        WA_BackFill,        backFill,
        WA_Activate,        TRUE,
        WA_RMBTrap,         TRUE,
        WA_Backdrop,        TRUE,
        WA_IDCMP,           IDCMP_REFRESHWINDOW,
        TAG_DONE))
    {
        return(w);
    }
    return(NULL);
}


struct Window *openIntroWindow(struct Screen *s, struct BitMap *gfx, struct BitMap **bmptr)
{
    struct Window *w;
    WORD width = 160, height = 160;
    WORD left = 0, top = 0;
    struct Hook myHook =
    {
        { NULL, NULL },
        (ULONG (*)()) fastBackFill,
        NULL,
        NULL
    };

    struct Hook *backFill = &myHook;
    struct BitMap *bm;

    if (bm = AllocBitMap(width, height, s->RastPort.BitMap->Depth, 0, NULL))
    {
        bltTemplate(gfx, 0, 16, 16, 16, bm);
        backFill->h_Data = (APTR)bm;

        if (w = OpenWindowTags(NULL,
            WA_CustomScreen,    s,
            WA_Left,            left,
            WA_Top,             top,
            WA_Width,           width,
            WA_Height,          height,
            WA_Borderless,      TRUE,
            WA_SimpleRefresh,   TRUE,
            WA_BackFill,        backFill,
            WA_Activate,        TRUE,
            WA_RMBTrap,         TRUE,
            WA_IDCMP,           IDCMP_MOUSEBUTTONS,
            TAG_DONE))
        {
            *bmptr = bm;
            return(w);
        }
    }
    return(NULL);
}

int main()
{
    struct BitMap *bm[2], *gfx;
    struct ColorMap *cm;
    struct Screen *s;
    struct BitMap *backbm;

    if (allocBitMaps(bm, 320, 256, 5))
    {
        if (gfx = loadBitMap("Dane/Grafika.iff", &cm))
        {
            WORD width = 320, height = 256, depth = 5;
            if (backbm = AllocBitMap(width, height, depth, 0, NULL))
            {
                bltTemplate(gfx, 0, 0, 16, 16, backbm);
                if (s = openScreen(bm[0], cm, backbm))
                {
                    struct Window *bw;

                    if (bw = openBackdropWindow(s, gfx, backbm))
                    {
                        struct Window *w;
                        struct BitMap *auxbm;

                        if (w = openIntroWindow(s, gfx, &auxbm))
                        {
                            struct IntuiMessage *msg;

                            Delay(50);

                            WORD i;

                            for (i = 0; i < 32; i++)
                            {
                                BltBitMap(w->RPort->BitMap, w->LeftEdge, w->TopEdge, w->RPort->BitMap, w->LeftEdge + 1, w->TopEdge + 1, w->Width, w->Height, 0xc0, 0xff, NULL);
                                BltBitMap(backbm, w->LeftEdge, w->TopEdge, w->RPort->BitMap, w->LeftEdge, w->TopEdge, 1, w->Height, 0xc0, 0xff, NULL);
                                BltBitMap(backbm, w->LeftEdge + 1, w->TopEdge, w->RPort->BitMap, w->LeftEdge + 1, w->TopEdge, w->Width - 1, 1, 0xc0, 0xff, NULL);
                                w->LeftEdge++;
                                w->TopEdge++;
                                WaitTOF();
                            }



                            WaitPort(w->UserPort);
                            CloseWindow(w);
                            FreeBitMap(auxbm);
                            WaitPort(bw->UserPort);
                            while (msg = (struct IntuiMessage *)GetMsg(bw->UserPort))
                            {
                                if (msg->Class == IDCMP_REFRESHWINDOW)
                                {
                                    struct Rectangle rect;
                                    BeginRefresh(bw);

                                    GetRPAttrs(bw->RPort, RPTAG_DrawBounds, &rect, TAG_DONE);
                                    SetAPen(bw->RPort, 10);
                                    Move(bw->RPort, rect.MinX, rect.MinY);
                                    Draw(bw->RPort, rect.MaxX, rect.MinY);
                                    Draw(bw->RPort, rect.MaxX, rect.MaxY);
                                    Draw(bw->RPort, rect.MinX, rect.MaxY);
                                    Draw(bw->RPort, rect.MinX, rect.MinY + 1);
                                    EndRefresh(bw, TRUE);
                                }
                                ReplyMsg((struct Message *)msg);
                            }
                            Delay(100);
                        }
                        CloseWindow(bw);
                    }
                    CloseScreen(s);
                }
                FreeBitMap(backbm);
            }
            FreeBitMap(gfx);
            FreeColorMap(cm);
        }
        freeBitMaps(bm);
    }
    return(RETURN_OK);
}
