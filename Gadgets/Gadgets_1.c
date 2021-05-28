
#include <intuition/intuition.h>
#include <clib/intuition_protos.h>

WORD added = 0; /* Added gadgets */

/* Add gadget */
void addGadget( struct Gadget *glist, WORD left, WORD top, WORD width, WORD height )
{
    glist += added;

    if( added >= 1 )
        /* Link previous with this */
        glist[ -1 ].NextGadget = glist;

    glist->LeftEdge     = left;
    glist->TopEdge      = top;
    glist->Width        = width;
    glist->Height       = height;

    glist->Flags        = GFLG_GADGHNONE;
    glist->Activation   = GACT_IMMEDIATE|GACT_FOLLOWMOUSE;
    glist->GadgetType   = GTYP_BOOLGADGET;

    glist->GadgetRender = glist->SelectRender = NULL;
    glist->GadgetText   = NULL;

    glist->MutualExclude = 0;
    glist->SpecialInfo  = NULL;

    glist->GadgetID     = added;
    glist->UserData     = NULL;

    glist->NextGadget   = NULL;

    added++;
}

/* Open window */
struct Window *openWindow( WORD x, WORD y, WORD w, WORD h, struct Screen *s, struct Gadget *glist, LONG idcmp )
{
    struct Window *win;

    if( win = OpenWindowTags( NULL,
        WA_CustomScreen,    s,
        WA_Gadgets,         glist,
        WA_Left,            x,
        WA_Top,             y,
        WA_Width,           w,
        WA_Height,          h,
        WA_Backdrop,        TRUE,
        WA_Borderless,      TRUE,
        WA_Activate,        TRUE,
        WA_RMBTrap,         TRUE,
        WA_SimpleRefresh,   TRUE,
        WA_BackFill,        LAYERS_NOBACKFILL,
        WA_IDCMP,           idcmp,
        WA_ReportMouse,     TRUE,
        TAG_DONE))
    {
        return( win );
    }
    return( NULL );
}

struct Screen *openScreen( STRPTR title, UBYTE depth, ULONG *colors )
{
    struct Screen *s;
    struct Rectangle dclip = { 0, 0, 319, 255 };

    static struct ScreenBuffer *sb[2];

    if( s = OpenScreenTags( NULL,
        SA_DClip,           &dclip,
        SA_Depth,           depth,
        SA_DisplayID,       LORES_KEY|EXTRAHALFBRITE_KEY,
        SA_Exclusive,       TRUE,
        SA_Quiet,           TRUE,
        SA_ShowTitle,       FALSE,
        SA_BackFill,        LAYERS_NOBACKFILL,
        SA_Title,           title,
        SA_Colors32,        colors,
        SA_Interleaved,     TRUE,
        TAG_DONE ) )
    {
        if( sb[0] = AllocScreenBuffer(s, NULL, SB_SCREEN_BITMAP))
        {
            if( sb[1] = AllocScreenBuffer(s, NULL, 0))
            {
                s->UserData = (APTR)sb;
                return( s );
            }
            FreeScreenBuffer(s, sb[0]);
        }
        CloseScreen(s);
    }
    return( NULL );
}

void closeScreen(struct Screen *s)
{
    struct ScreenBuffer **sb = (struct ScreenBuffer **)s->UserData;

    FreeScreenBuffer(s, sb[1]);
    FreeScreenBuffer(s, sb[0]);
    CloseScreen(s);
}
