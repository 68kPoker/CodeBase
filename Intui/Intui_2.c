
/*
 * GUI
 */

#include <stdio.h>
#include <intuition/intuition.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>

#define WIDTH  320
#define HEIGHT 256
#define DEPTH  5
#define IDCMP_FLAGS (IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE|IDCMP_GADGETUP)

struct screenData
{
    struct TextAttr *ta;
    struct TextFont *font;
    struct BitMap   *bm[2];
    struct Screen   *s;
    struct Window   *w;
};

/* Open font */
struct TextFont *openFont(struct screenData *sd, struct TextAttr *ta)
{
    if (sd->font = OpenFont(sd->ta = ta))
    {
        return(sd->font);
    }
    return(NULL);
}

void closeFont(struct screenData *sd)
{
    CloseFont(sd->font);
}

/* Alloc bitmaps */
struct BitMap **allocBitMaps(struct screenData *sd, WORD width, WORD height, UBYTE depth)
{
    if (sd->bm[0] = AllocBitMap(width, height, depth, BMF_DISPLAYABLE|BMF_INTERLEAVED|BMF_CLEAR, NULL))
    {
        if (sd->bm[1] = AllocBitMap(width, height, depth, BMF_DISPLAYABLE|BMF_INTERLEAVED|BMF_CLEAR, NULL))
        {
            return(sd->bm);
        }
        FreeBitMap(sd->bm[0]);
    }
    return(NULL);
}

void freeBitMaps(struct screenData *sd)
{
    FreeBitMap(sd->bm[1]);
    FreeBitMap(sd->bm[0]);
}

/* Open screen */
struct Window *openScreen(struct screenData *sd, ULONG idcmp)
{
    WORD depth = GetBitMapAttr(sd->bm[0], BMA_DEPTH);

    ULONG colors[(1 * 3) + 2] = { 1 << 16 };
    ULONG modeID;

    if ((modeID = BestModeID(
        BIDTAG_NominalWidth,    GetBitMapAttr(sd->bm[0], BMA_WIDTH),
        BIDTAG_NominalHeight,   GetBitMapAttr(sd->bm[0], BMA_HEIGHT),
        BIDTAG_Depth,           depth,
        BIDTAG_DIPFMustNotHave, DIPF_IS_LACE,
        TAG_DONE)) != INVALID_ID)
    {
        if (sd->s = OpenScreenTags(NULL,
            SA_DisplayID,   modeID,
            SA_BitMap,      sd->bm[0],
            SA_Colors32,    colors,
            SA_Font,        sd->ta,
            SA_Quiet,       TRUE,
            SA_Exclusive,   TRUE,
            SA_ShowTitle,   FALSE,
            SA_Draggable,   FALSE,
            SA_BackFill,    LAYERS_NOBACKFILL,
            TAG_DONE))
        {
            sd->s->UserData = (APTR)sd;
            if (sd->w = OpenWindowTags(NULL,
                WA_CustomScreen,    sd->s,
                WA_Left,            0,
                WA_Top,             0,
                WA_Width,           sd->s->Width,
                WA_Height,          sd->s->Height,
                WA_Backdrop,        TRUE,
                WA_Borderless,      TRUE,
                WA_Activate,        TRUE,
                WA_RMBTrap,         TRUE,
                WA_SimpleRefresh,   TRUE,
                WA_IDCMP,           idcmp,
                WA_ReportMouse,     TRUE,
                WA_BackFill,        LAYERS_NOBACKFILL,
                TAG_DONE))
            {
                return(sd->w);
            }
            else
                printf("Couldn't open window!\n");
            CloseScreen(sd->s);
        }
        else
            printf("Couldn't open screen!\n");
    }
    else
        printf("Couldn't find screen ModeID!\n");
    return(NULL);
}

void closeScreen(struct screenData *sd)
{
    CloseWindow(sd->w);
    CloseScreen(sd->s);
}

/* Grouping */
BOOL openIntui(struct screenData *sd, struct TextAttr *ta, WORD width, WORD height, UBYTE depth, ULONG idcmp)
{
    if (openFont(sd, ta))
    {
        if (allocBitMaps(sd, width, height, depth))
        {
            if (openScreen(sd, idcmp))
            {
                return(TRUE);
            }
            freeBitMaps(sd);
        }
        closeFont(sd);
    }
    return(FALSE);
}

void closeIntui(struct screenData *sd)
{
    closeScreen(sd);
    freeBitMaps(sd);
    closeFont(sd);
}

int main()
{
    struct screenData sd;
    struct TextAttr ta =
    {
        "topaz.font",
        9,
        FS_NORMAL,
        FPF_ROMFONT|FPF_DESIGNED
    };

    if (openIntui(&sd, &ta, WIDTH, HEIGHT, DEPTH, IDCMP_FLAGS))
    {
        Delay(300);
        closeIntui(&sd);
    }
    return(0);
}
