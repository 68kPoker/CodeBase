
/* Board editor */

#include <assert.h>
#include <intuition/intuition.h>

/* Board gadget */

#define WIDTH     20
#define HEIGHT    16

#define SELWIDTH  4
#define SELHEIGHT 4

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
