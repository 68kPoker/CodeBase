
/* Warehouse GUI */

#include "Screen.h"

#include <intuition/classusr.h>
#include <intuition/gadgetclass.h>
#include <intuition/imageclass.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include <clib/diskfont_protos.h>

#define BOARDIDCMP IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE
#define BARIDCMP   IDCMP_GADGETUP

enum gadgets
{
    BARGAD_TILE,
    BARGAD_BOARD,
    BARGADS
};

enum
{
    TILEGAD_WALL,
    TILEGAD_FLOOR,
    TILEGAD_BOX,
    TILEGAD_PLACE,
    TILEGADS
};

enum
{
    BOARDGAD_RESTORE,
    BOARDGAD_SAVE,
    BOARDGAD_NEXT,
    BOARDGAD_PREV,
    BOARDGADS
};

struct wareScreen
{
    struct TextFont *font;
    struct screen s;
    struct window back, bar, menu; /* Board and menu windows */
    Object *bargads[BARGADS];
    union
    {
        Object *tilegads[TILEGADS];
        Object *boardgads[BOARDGADS];
    } menuItems;
    Object *frameimg;
};

struct TextAttr ta =
{
    "ld.font", 8,
    FS_NORMAL,
    FPF_DISKFONT | FPF_DESIGNED
};

struct IntuiText it[] = {
{
    1, 0,JAM1, 0, 0, &ta, "Tile", NULL
},
{
    1, 0,JAM1, 0, 0, &ta, "Board",  NULL
},
{
    1, 0,JAM1, 0, 0, &ta, "Wall",  NULL
},
{
    1, 0,JAM1, 0, 0, &ta, "Floor",  NULL
},
{
    1, 0,JAM1, 0, 0, &ta, "Box",  NULL
},
{
    1, 0,JAM1, 0, 0, &ta, "Place",  NULL
} };

BOOL prepBarGadgets(Object *gads[], struct screen *s, Object *img)
{
    if (gads[BARGAD_TILE] = NewObject(NULL, "frbuttonclass",
        GA_Left,    0,
        GA_Top,     0,
        GA_IntuiText, &it[0],
        GA_DrawInfo, s->dri,
        GA_Image,   img,
        TAG_DONE))
    {
        if (gads[BARGAD_BOARD] = NewObject(NULL, "frbuttonclass",
            GA_Left,    80,
            GA_Top,     0,
            GA_IntuiText, &it[1],
            GA_DrawInfo, s->dri,
            GA_Image,   img,
            GA_Previous,    gads[BARGAD_TILE],
            TAG_DONE))
        {
            return(TRUE);
        }
        DisposeObject(gads[BARGAD_TILE]);
    }
    return(FALSE);
}

BOOL prepTileGadgets(Object *gads[], struct screen *s, Object *img)
{
    if (gads[TILEGAD_WALL] = NewObject(NULL, "frbuttonclass",
        GA_Left,    0,
        GA_Top,     0,
        GA_IntuiText,    &it[2],
        GA_DrawInfo, s->dri,
        GA_Image,   img,
        GA_Selected, TRUE,
        TAG_DONE))
    {
        if (gads[TILEGAD_FLOOR] = NewObject(NULL, "frbuttonclass",
            GA_Left,    0,
            GA_Top,     16,
            GA_IntuiText,    &it[3],
            GA_DrawInfo, s->dri,
            GA_Image,   img,
            GA_Previous,    gads[TILEGAD_WALL],
            TAG_DONE))
        {
            if (gads[TILEGAD_BOX] = NewObject(NULL, "frbuttonclass",
                GA_Left,    0,
                GA_Top,     32,
                GA_IntuiText,    &it[4],
                GA_DrawInfo, s->dri,
                GA_Image,   img,
                GA_Previous,    gads[TILEGAD_FLOOR],
                TAG_DONE))
            {
                if (gads[TILEGAD_PLACE] = NewObject(NULL, "frbuttonclass",
                    GA_Left,    0,
                    GA_Top,     48,
                    GA_IntuiText,    &it[5],
                    GA_DrawInfo, s->dri,
                    GA_Image,   img,
                    GA_Previous,    gads[TILEGAD_BOX],
                    TAG_DONE))
                {
                    return(TRUE);
                }
                DisposeObject(gads[TILEGAD_BOX]);
            }
            DisposeObject(gads[TILEGAD_FLOOR]);
        }
        DisposeObject(gads[TILEGAD_WALL]);
    }
    return(FALSE);
}

void disposeGadgets(Object *gads[], WORD count)
{
    WORD i;

    for (i = 0; i < count; i++)
    {
        DisposeObject(gads[i]);
    }
}

BOOL openWareScreen(struct wareScreen *ws)
{
    struct screen *s = &ws->s;

    if (s->bm[0] = AllocBitMap(320, 256, 5, BMF_DISPLAYABLE|BMF_CLEAR, NULL))
    {
        if (s->bm[1] = AllocBitMap(320, 256, 5, BMF_DISPLAYABLE|BMF_CLEAR, NULL))
        {
            if (openScreen(s, 0, 0, 319, 255, LORES_KEY, NULL))
            {
                if (openWindow(&ws->back, s->s, 0, 0, 319, 255, TRUE, TRUE, BOARDIDCMP, NULL))
                {
                    if (ws->frameimg = NewObject(NULL, "frameiclass",
                        TAG_DONE))
                    {
                        if (prepBarGadgets(ws->bargads, s, ws->frameimg))
                        {
                            if (openWindow(&ws->bar, s->s, 0, 0, 319, 15, FALSE, FALSE, BARIDCMP, NULL))
                            {
                                WORD i;
                                AddGList(ws->bar.w, (struct Gadget *)ws->bargads[0], -1, BARGADS, NULL);
                                for (i = 0; i < BARGADS; i++)
                                {
                                    SetGadgetAttrs((struct Gadget *)ws->bargads[i], ws->bar.w, NULL,
                                        GA_Width,   80,
                                        GA_Height,  16,
                                        TAG_DONE);
                                }
                                return(TRUE);
                            }
                            disposeGadgets(ws->bargads, BARGADS);
                        }
                        DisposeObject(ws->frameimg);
                    }
                    closeWindow(&ws->back);
                }
                closeScreen(s);
            }
            FreeBitMap(s->bm[1]);
        }
        FreeBitMap(s->bm[0]);
    }
    return(FALSE);
}

BOOL openMenu(struct wareScreen *ws)
{
    if (prepTileGadgets(ws->menuItems.tilegads, &ws->s, ws->frameimg))
    {
        if (openWindow(&ws->menu, ws->s.s, 0, 16, 79, 16 + (16 * TILEGADS) - 1, FALSE, FALSE, BARIDCMP, NULL))
        {
            WORD i;
            AddGList(ws->menu.w, (struct Gadget *)ws->menuItems.tilegads[0], -1, TILEGADS, NULL);
            for (i = 0; i < TILEGADS; i++)
            {
                SetGadgetAttrs((struct Gadget *)ws->menuItems.tilegads[i], ws->menu.w, NULL,
                    GA_Width,   80,
                    GA_Height,  16,
                    TAG_DONE);
            }
            return(TRUE);
        }
        disposeGadgets(ws->menuItems.tilegads, TILEGADS);
    }
    return(FALSE);
}

void closeMenu(struct wareScreen *ws)
{
    closeWindow(&ws->menu);
    disposeGadgets(ws->menuItems.tilegads, TILEGADS);
}

void closeWareScreen(struct wareScreen *ws)
{
    closeWindow(&ws->bar);
    disposeGadgets(ws->bargads, BARGADS);
    DisposeObject(ws->frameimg);
    closeWindow(&ws->back);
    closeScreen(&ws->s);
    FreeBitMap(ws->s.bm[1]);
    FreeBitMap(ws->s.bm[0]);
}

int main()
{
    struct wareScreen ws;

    if (ws.font = OpenDiskFont(&ta))
    {
        if (openWareScreen(&ws))
        {
            if (openMenu(&ws))
            {
                Delay(400);
                closeMenu(&ws);
            }
            closeWareScreen(&ws);
        }
        CloseFont(ws.font);
    }
    return(0);
}
