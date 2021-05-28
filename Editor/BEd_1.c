
/* Board editor */

#include <assert.h>
#include <intuition/intuition.h>

/* Board gadget */

#define WIDTH     20
#define HEIGHT    16

#define SELWIDTH  4
#define SELHEIGHT 4

enum
{
    BOARDGAD,
    SELECTIONGAD
};

enum
{
    FLOORTILE,
    WALLTILE
};

struct boardInfo
{
    WORD type;
    struct selectionInfo *si;
    BOOL paint;
    WORD cx, cy;
    WORD board[HEIGHT][WIDTH];
};

/* Tile selection */

struct selectionInfo
{
    WORD type;
    WORD selectedTile;
};

/* Board gadget dispatcher */

LONG boardDispatch(struct Gadget *gad, struct IntuiMessage *msg)
{
    WORD mx = msg->MouseX, tx = mx >> 4;
    WORD my = msg->MouseY, ty = my >> 4;

    struct boardInfo *bi = (struct boardInfo *)gad->UserData;

    switch (msg->Class)
    {
        case IDCMP_GADGETDOWN:
            /* User activated the gadget */

            if (!(tx >= 0 && tx < WIDTH && ty >= 0 && ty < HEIGHT))
            {
                break;
            }
            bi->paint = TRUE;
            bi->cx = tx;
            bi->cy = ty;
            bi->board[ty][tx] = bi->si->selectedTile;

            /* Paint here */
            break;

        case IDCMP_MOUSEMOVE:
            if (!bi->paint)
            {
                break;
            }
            /* Paint mode */
            if (!(tx >= 0 && tx < WIDTH && ty >= 0 && ty < HEIGHT))
            {
                break;
            }
            if (!(tx != bi->cx || ty != bi->cy))
            {
                break;
            }
            /* Another cell */
            bi->cx = tx;
            bi->cy = ty;
            bi->board[ty][tx] = bi->si->selectedTile;

            /* Paint here */
            break;

        case IDCMP_MOUSEBUTTONS:
            if (msg->Code == (IECODE_LBUTTON|IECODE_UP_PREFIX))
            {
                /* Paint mode OFF */
                bi->paint = FALSE;
            }
            break;
    }
    return(0);
}

/* Selection gadget dispatcher */

LONG selectionDispatch(struct Gadget *gad, struct IntuiMessage *msg)
{
    WORD mx = msg->MouseX, tx = mx >> 4;
    WORD my = msg->MouseY, ty = my >> 4;

    struct selectionInfo *si = (struct selectionInfo *)gad->UserData;

    switch (msg->Class)
    {
        case IDCMP_GADGETDOWN:
            if ((si->selectedTile % SELWIDTH) != tx || (si->selectedTile / SELWIDTH) != ty)
            {
                /* Highlight tile here */

                si->selectedTile = (ty * SELWIDTH) + tx;
            }
            break;
    }
    return(0);
}

void initBoard(struct Gadget *gad, struct boardInfo *bi, struct selectionInfo *si)
{
    WORD x, y;

    gad->UserData = (APTR)bi;
    gad->LeftEdge = 0;
    gad->TopEdge = 0;
    gad->Width = WIDTH << 4;
    gad->Height = HEIGHT << 4;
    gad->Flags = GFLG_GADGHNONE;
    gad->GadgetType = GTYP_BOOLGADGET;
    gad->GadgetID = 1;
    gad->Activation = GACT_IMMEDIATE|GACT_FOLLOWMOUSE;
    gad->MutualExclude = 0;
    gad->SpecialInfo = NULL;
    gad->NextGadget = NULL;
    gad->GadgetRender = gad->SelectRender = gad->GadgetText = NULL;

    bi->type = BOARDGAD;
    bi->si = si;
    bi->paint = FALSE;
    bi->cx = bi->cy = 0;

    for (y = 0; y < HEIGHT; y++)
    {
        for (x = 0; x < WIDTH; x++)
        {
            if (x == 0 || x == WIDTH - 1 || y == 0 || y == HEIGHT - 1)
                bi->board[y][x] = WALLTILE;
            else
                bi->board[y][x] = FLOORTILE;
        }
    }
}

void initSelection(struct Gadget *gad, struct selectionInfo *si)
{
    si->type = SELECTIONGAD;
    si->selectedTile = WALLTILE;

    gad->UserData = (APTR)si;
    gad->LeftEdge = 0;
    gad->TopEdge = 0;
    gad->Width = SELWIDTH << 4;
    gad->Height = SELHEIGHT << 4;
    gad->Flags = GFLG_GADGHNONE;
    gad->GadgetType = GTYP_BOOLGADGET;
    gad->GadgetID = 2;
    gad->Activation = GACT_IMMEDIATE;
    gad->MutualExclude = 0;
    gad->SpecialInfo = NULL;
    gad->NextGadget = NULL;
    gad->GadgetRender = gad->SelectRender = gad->GadgetText = NULL;
}

void boardEd()
{
    struct Gadget bgad, sgad;
    struct boardInfo bi;
    struct selectionInfo si;
    struct Screen *s;
    struct Window *w;

    initBoard(&bgad, &bi, &si);
    initSelection(&sgad, &si);

    if (s = OpenScreenTags(NULL,
        SA_DClip,   &dclip,
        SA_Depth,   5,
        SA_DisplayID,   LORES_KEY,
        TAG_DONE);


}
