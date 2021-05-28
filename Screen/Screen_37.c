
/* Screen.c:
 * Kod ekranu.
 */

#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <intuition/gadgetclass.h>
#include <libraries/asl.h>

#include <clib/intuition_protos.h>
#include <clib/exec_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include <clib/asl_protos.h>

#include "iffp/ilbmapp.h"

#include "Engine.h"

#define ID_MGZN MAKE_ID('M','G','Z','N')
#define ID_PLNS MAKE_ID('P','L','N','S')

/* Rozdzielczoôê */
struct Resolution
{
    struct Rectangle dclip;
    ULONG modeID;
    UBYTE depth;
};

struct ScreenInfo
{
    struct ScreenBuffer *sb[2];
    WORD frame;
};

struct WindowInfo
{
    Object *panel;
    struct editBoard eb;
};

UWORD pens[] = { ~0 };

/* Ustalenie rozdzielczoôci gry */
BOOL getResolution(struct Resolution *res)
{
    /* Pobranie monitora domyôlnego ekranu publicznego */
    UWORD width = 320, height = 256;
    UBYTE depth = 5;
    struct Screen *s = LockPubScreen(NULL);

    if (s)
    {
        struct ViewPort *vp = &s->ViewPort;
        ULONG pubID = GetVPModeID(vp);

        if (pubID != INVALID_ID)
        {
            ULONG monitorID = pubID & MONITOR_ID_MASK;

            /* Ustalenie trybu ekranu gry */
            ULONG modeID = BestModeID(
                BIDTAG_MonitorID,       monitorID,
                BIDTAG_NominalWidth,    width,
                BIDTAG_NominalHeight,   height,
                BIDTAG_Depth,           depth,
                TAG_DONE);

            if (modeID != INVALID_ID)
            {
                struct NameInfo name;
                if (GetDisplayInfoData(NULL, (UBYTE *)&name, sizeof(name), DTAG_NAME, modeID) > 0)
                {
                    struct DimensionInfo dims;
                    printf("Display ID name is: %s.\n", name.Name);
                    if (GetDisplayInfoData(NULL, (UBYTE *)&dims, sizeof(dims), DTAG_DIMS, modeID) > 0)
                    {
                        struct DisplayInfo disp;
                        if (GetDisplayInfoData(NULL, (UBYTE *)&disp, sizeof(disp), DTAG_DISP, modeID) > 0)
                        {
                            printf("Aspect ratio: %d/%d\n", disp.Resolution.x, disp.Resolution.y);
                            printf("Color bits: %d\n", disp.RedBits + disp.GreenBits + disp.BlueBits);
                            printf("Dimension: %d x %d\n", dims.Nominal.MaxX - dims.Nominal.MinX + 1, dims.Nominal.MaxY - dims.Nominal.MinY + 1);
                            res->modeID = modeID;
                            res->depth  = depth;
                            res->dclip  = dims.Nominal;
                            UnlockPubScreen(NULL, s);
                            return(TRUE);
                        }
                    }
                }
            }
        }
        UnlockPubScreen(NULL, s);
    }
    return(FALSE);
}

/* Otwarcie ekranu w danej rozdzielczoôci i palecie */
struct Screen *openScreen(struct Resolution *res, ULONG *palette, struct ScreenInfo *si)
{
    struct Screen *s;

    if (s = OpenScreenTags(NULL,
        SA_DClip,       &res->dclip,
        SA_Depth,       res->depth,
        palette ? SA_Colors32 : TAG_IGNORE,    palette,
        SA_DisplayID,   res->modeID,
        SA_Quiet,       TRUE,
        SA_Exclusive,   TRUE,
        SA_ShowTitle,   FALSE,
        SA_Title,       "Magazyn",
        SA_BackFill,    LAYERS_NOBACKFILL,
        SA_Interleaved, TRUE,
        SA_Pens, pens,
        TAG_DONE))
    {
        s->UserData = (APTR)si;
        if (si->sb[0] = AllocScreenBuffer(s, NULL, SB_SCREEN_BITMAP))
        {
            if (si->sb[1] = AllocScreenBuffer(s, NULL, 0))
            {
                return(s);
            }
            FreeScreenBuffer(s, si->sb[0]);
        }
        CloseScreen(s);
    }
    return(NULL);
}

/* Zaîadowane grafiki */
BOOL loadGraphics(struct ILBMInfo *ii, STRPTR name)
{
    ULONG props[] = { ID_ILBM, ID_BMHD, ID_ILBM, ID_CMAP, 0 };
    ULONG stops[] = { ID_ILBM, ID_BODY, 0 };

    if (ii->ParseInfo.iff = AllocIFF())
    {
        ii->ParseInfo.propchks = props;
        ii->ParseInfo.collectchks = NULL;
        ii->ParseInfo.stopchks = stops;

        if (loadbrush(ii, name) == 0)
        {
            return(TRUE);
        }
        FreeIFF(ii->ParseInfo.iff);
    }
    return(FALSE);
}

void freeGraphics(struct ILBMInfo *ii)
{
    unloadbrush(ii);
    FreeIFF(ii->ParseInfo.iff);
}

struct Window *openWindow(struct Screen *s, struct WindowInfo *wi)
{
    struct Window *w;

    if (w = OpenWindowTags(NULL,
        WA_CustomScreen,    s,
        WA_Left,            0,
        WA_Top,             0,
        WA_Width,           s->Width,
        WA_Height,          s->Height,
        WA_Backdrop,        TRUE,
        WA_Borderless,      TRUE,
        WA_Activate,        TRUE,
        WA_RMBTrap,         TRUE,
        WA_IDCMP,           IDCMP_RAWKEY|IDCMP_GADGETUP|IDCMP_GADGETDOWN|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE,
        WA_ReportMouse,     TRUE,
        WA_SmartRefresh,    TRUE,
        WA_BackFill,        LAYERS_NOBACKFILL,
        TAG_DONE))
    {
        w->UserData = (APTR)wi;
        if (wi->panel = NewObject(NULL, "buttongclass",
            GA_Left, 0,
            GA_Top, 0,
            GA_Width, w->Width,
            GA_Height, 16,
            GA_ID, 1,
            GA_RelVerify, TRUE,
            GA_Immediate, TRUE,
            TAG_DONE))
        {
            AddGadget(w, (struct Gadget *)wi->panel, ~0);
            return(w);
        }
        CloseWindow(w);
    }
    return(NULL);
}

void closeWindow(struct Window *w)
{
    struct WindowInfo *wi = (struct WindowInfo *)w->UserData;
    RemoveGadget(w, (struct Gadget *)wi->panel);
    DisposeObject(wi->panel);
    CloseWindow(w);
}

/* Zamknij ekran z przyîâczonymi zasobami */
void closeScreen(struct Screen *s)
{
    struct ScreenInfo *si = (struct ScreenInfo *)s->UserData;
    FreeScreenBuffer(s, si->sb[1]);
    FreeScreenBuffer(s, si->sb[0]);
    CloseScreen(s);
}

void setBoard(struct ILBMInfo *ii, struct Window *w)
{
    WORD x, y;
    struct WindowInfo *wi = (struct WindowInfo *)w->UserData;

    for (y = 1; y < 16; y++)
    {
        for (x = 0; x < 20; x++)
        {
            if (x == 0 || y == 1 || x == 19 || y == 15)
            {
                wi->eb.kinds[y][x] = WALL_KIND;
            }
            else
            {
                wi->eb.kinds[y][x] = FLOOR_KIND;
            }
        }
    }
}

void drawBoard(struct ILBMInfo *ii, struct Window *w)
{
    WORD x, y;
    struct WindowInfo *wi = (struct WindowInfo *)w->UserData;

    for (y = 1; y < 16; y++)
    {
        for (x = 0; x < 20; x++)
        {
            BltBitMapRastPort(ii->brbitmap, (2 + wi->eb.kinds[y][x]) << 4, 0, w->RPort, x << 4, y << 4, 16, 16, 0xc0);
        }
    }
}

BOOL loadBoard(struct Window *w)
{
    struct FileRequester *fr;
    struct WindowInfo *wi = (struct WindowInfo *)w->UserData;
    ULONG stops[] = { ID_MGZN, ID_PLNS, 0 };
    static UBYTE name[256];
    BOOL result = FALSE;

    if (fr = AllocAslRequestTags(ASL_FileRequest, TAG_DONE))
    {
        if (AslRequestTags(fr,
            ASLFR_Window, w,
            ASLFR_SleepWindow, TRUE,
            TAG_DONE))
        {
            struct ParseInfo pi;

            strncpy(name, fr->fr_Drawer, 255);
            AddPart(name, fr->fr_File, 255);

            if (pi.iff = AllocIFF())
            {
                if (openifile(&pi, name, IFFF_READ) == 0)
                {
                    pi.propchks = NULL;
                    pi.stopchks = stops;
                    pi.collectchks = NULL;
                    LONG err;

                    if ((err = parseifile(&pi, ID_MGZN, ID_FORM, NULL, NULL, stops)) == 0 || err == IFFERR_EOC || err == IFFERR_EOF)
                    {
                        if (ReadChunkBytes(pi.iff, &wi->eb, sizeof(wi->eb)) == sizeof(wi->eb))
                        {
                            result = TRUE;
                        }
                    }
                    else
                        printf("Error %ld\n", err);
                    closeifile(&pi);
                }
                FreeIFF(pi.iff);
            }
        }
        FreeAslRequest(fr);
    }
    return(result);
}

BOOL saveBoard(struct Window *w)
{
    struct FileRequester *fr;
    struct WindowInfo *wi = (struct WindowInfo *)w->UserData;
    static UBYTE name[256];

    if (fr = AllocAslRequestTags(ASL_FileRequest, TAG_DONE))
    {
        if (AslRequestTags(fr,
            ASLFR_Window, w,
            ASLFR_SleepWindow, TRUE,
            ASLFR_DoSaveMode, TRUE,
            TAG_DONE))
        {
            struct ParseInfo pi;

            strncpy(name, fr->fr_Drawer, 255);
            AddPart(name, fr->fr_File, 255);

            if (pi.iff = AllocIFF())
            {
                if (openifile(&pi, name, IFFF_WRITE) == 0)
                {
                    if (PushChunk(pi.iff, ID_MGZN, ID_FORM, IFFSIZE_UNKNOWN) == 0)
                    {
                        if (PutCk(pi.iff, ID_PLNS, sizeof(struct editBoard), &wi->eb) == 0)
                        {
                            PopChunk(pi.iff);
                        }
                    }
                    closeifile(&pi);
                }
                FreeIFF(pi.iff);
            }
        }
        FreeAslRequest(fr);
    }
}

void loop(struct Window *w, struct ILBMInfo *ii)
{
    WORD active = -1;
    BOOL done = FALSE;
    WORD tile = WALL_KIND;
    WORD prevx = -1, prevy = -1;
    BOOL paint = FALSE;
    struct WindowInfo *wi = (struct WindowInfo *)w->UserData;

    while (!done)
    {
        struct IntuiMessage *msg;
        WaitPort(w->UserPort);

        while (msg = (struct IntuiMessage *)GetMsg(w->UserPort))
        {
            if (msg->Class == IDCMP_GADGETDOWN)
            {
                WORD x = msg->MouseX >> 4;

                BltBitMapRastPort(ii->brbitmap, x << 4, 16, w->RPort, x << 4, 0, 16, 16, 0xc0);
                active = x;
            }
            else if (msg->Class == IDCMP_GADGETUP)
            {
                WORD x = msg->MouseX >> 4;

                if (active != -1)
                {
                    BltBitMapRastPort(ii->brbitmap, active << 4, 0, w->RPort, active << 4, 0, 16, 16, 0xc0);
                    if (active == 0)
                    {
                        done = TRUE;
                    }
                    else if (active >= 2 && active < 2 + KINDS)
                    {
                        tile = active - 2;
                    }
                    else if (active == 15)
                    {
                        if (loadBoard(w))
                        {
                            drawBoard(ii, w);
                        }
                    }
                    else if (active == 16)
                    {
                        saveBoard(w);
                    }
                    active = -1;
                }
            }
            else if (msg->Class == IDCMP_MOUSEBUTTONS)
            {
                WORD x = msg->MouseX >> 4;
                WORD y = msg->MouseY >> 4;

                if (msg->Code == IECODE_LBUTTON)
                {
                    BltBitMapRastPort(ii->brbitmap, (tile + 2) << 4, 0, w->RPort, x << 4, y << 4, 16, 16, 0xc0);
                    paint = TRUE;
                    wi->eb.kinds[y][x] = tile;
                }
                else if (msg->Code == (IECODE_LBUTTON|IECODE_UP_PREFIX))
                {
                    paint = FALSE;
                }
            }
            else if (msg->Class == IDCMP_MOUSEMOVE)
            {
                WORD x = msg->MouseX >> 4;
                WORD y = msg->MouseY >> 4;

                if (paint && x >= 0 && x < 20 && y >= 1 && y < 16)
                {
                    if (x != prevx || y != prevy)
                    {
                        BltBitMapRastPort(ii->brbitmap, (tile + 2) << 4, 0, w->RPort, x << 4, y << 4, 16, 16, 0xc0);
                        prevx = x;
                        prevy = y;
                        wi->eb.kinds[y][x] = tile;
                    }
                }
            }
            ReplyMsg((struct Message *)msg);
        }
    }
}

int main(int argc, char **argv)
{
    static struct Resolution res;
    static struct ScreenInfo si;
    static struct WindowInfo wi;
    static struct ILBMInfo ii;
    static UBYTE name[32] = "Dane/Sceneria1.iff";

    if (argc >= 2)
    {
        strncpy(name, "Dane", 31);
        AddPart(name, argv[1], 31);
    }

    if (getResolution(&res))
    {
        if (loadGraphics(&ii, name))
        {
            struct Screen *s;
            if (s = openScreen(&res, (ULONG *)ii.colorrecord, &si))
            {
                struct Window *w;
                if (w = openWindow(s, &wi))
                {
                    w->RPort->BitMap = si.sb[1]->sb_BitMap;

                    BltBitMapRastPort(ii.brbitmap, 0, 0, w->RPort, 0, 0, 320, 16, 0xc0);

                    setBoard(&ii, w);
                    drawBoard(&ii, w);

                    while (!ChangeScreenBuffer(s, si.sb[1]))
                    {
                        WaitTOF();
                    }

                    loop(w, &ii);

                    closeWindow(w);
                }
                closeScreen(s);
            }
            freeGraphics(&ii);
        }
    }
    return(0);
}
