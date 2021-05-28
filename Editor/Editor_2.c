
#include "Editor.h"

#include <clib/intuition_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>

/* Editor has few variables: */

WORD currentTile = 0; /* Current tile type */
WORD currentX = 0, currentY = 0; /* Current position */

struct map map = { 0 }; /* Currently edited map */

struct Gadget editGads[GID_COUNT] = { 0 }; /* Editor gadgets */

struct Window *editWin; /* Editor window */

/* Editor consists of four main functions: */

/* Place current tile on the board */
void placeTile(WORD x, WORD y)
{
    map.tiles[y][x] = currentTile;
}

/* Select current tile */
void selectTile(WORD tile)
{
    currentTile = tile;
}

/* Load and save map */
void saveMap(STRPTR name)
{
    /* We use IFF here */
}

void loadMap(STRPTR name)
{
}

/* Init editor gadgets */
void initEditGadgets(void)
{
    WORD i;
    struct Gadget *prev = NULL;

    for (i = 0; i < GID_COUNT; i++)
    {
        editGads[i].NextGadget = prev;
        editGads[i].Flags = GFLG_GADGHNONE;
        editGads[i].Activation = GACT_RELVERIFY;
        editGads[i].GadgetType = GTYP_BOOLGADGET;
        editGads[i].GadgetID = i;
        prev = editGads + i;
    }

    editGads[GID_BOARD].LeftEdge = 0;
    editGads[GID_BOARD].TopEdge = 0;
    editGads[GID_BOARD].Width = MAP_WIDTH << 4;
    editGads[GID_BOARD].Height = MAP_HEIGHT << 4;
    editGads[GID_BOARD].Activation |= GACT_IMMEDIATE|GACT_FOLLOWMOUSE;

    editGads[GID_SELECT].LeftEdge = 0;
    editGads[GID_SELECT].TopEdge = MAP_HEIGHT << 4;
    editGads[GID_SELECT].Width = MAP_WIDTH << 4;
    editGads[GID_SELECT].Height = 16;

    editGads[GID_PANEL].LeftEdge = MAP_WIDTH << 4;
    editGads[GID_PANEL].TopEdge = 0;
    editGads[GID_PANEL].Width = 32;
    editGads[GID_PANEL].Height = MAP_HEIGHT << 4;
}

struct Window *openEditWindow(struct Screen *s)
{
    struct Window *w;

    initEditGadgets();

    w = OpenWindowTags(NULL,
        WA_CustomScreen,    s,
        WA_Left,            0,
        WA_Top,             0,
        WA_Width,           (MAP_WIDTH + 2) << 4,
        WA_Height,          (MAP_HEIGHT + 1) << 4,
        WA_Backdrop,        TRUE,
        WA_Borderless,      TRUE,
        WA_Activate,        TRUE,
        WA_RMBTrap,         TRUE,
        WA_Gadgets,         editGads + GID_COUNT - 1,
        WA_SimpleRefresh,   TRUE,
        WA_IDCMP,           IDCMP_GADGETUP|IDCMP_GADGETDOWN|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE|IDCMP_RAWKEY,
        WA_ReportMouse,     TRUE,
        WA_BackFill,        LAYERS_NOBACKFILL,
        TAG_DONE);

    if (w)
    {
        BOOL done = FALSE, paint = FALSE;
        struct Gadget *active = NULL;
        WORD prevx = 0, prevy = 0;
        while (!done)
        {
            struct IntuiMessage *msg;
            struct Gadget *gad;
            WaitPort(w->UserPort);

            while (msg = (struct IntuiMessage *)GetMsg(w->UserPort))
            {
                WORD x = msg->MouseX >> 4, y = msg->MouseY >> 4;
                if (msg->Class == IDCMP_GADGETDOWN)
                {
                    gad = (struct Gadget *)msg->IAddress;
                    active = gad;
                    if (active->GadgetID == GID_BOARD)
                    {
                        RectFill(w->RPort, x << 4, y << 4, (x << 4) + 15, (y << 4) + 15);
                        paint = TRUE;
                        prevx = x;
                        prevy = y;
                    }

                }
                else if (msg->Class == IDCMP_GADGETUP)
                {
                    gad = (struct Gadget *)msg->IAddress;
                    if (gad->GadgetID == GID_BOARD)
                    {
                        printf("Gadget release\n");
                        paint = FALSE;
                    }
                    active = NULL;
                }
                else if (msg->Class == IDCMP_MOUSEBUTTONS)
                {
                    if (msg->Code == (IECODE_LBUTTON|IECODE_UP_PREFIX))
                    {
                        if (active && active->GadgetID == GID_BOARD)
                        {
                            printf("Button release\n");
                            paint = FALSE;
                        }
                        active = NULL;
                    }
                }
                else if (msg->Class == IDCMP_MOUSEMOVE)
                {
                    if (active && active->GadgetID == GID_BOARD && x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT && (x != prevx || y != prevy))
                    {
                        RectFill(w->RPort, x << 4, y << 4, (x << 4) + 15, (y << 4) + 15);
                        prevx = x;
                        prevy = y;
                        printf("%2d %2d\n", x, y);
                    }
                }
                else if (msg->Class == IDCMP_RAWKEY)
                {
                    if (msg->Code == 0x45)
                    {
                        done = TRUE;
                    }
                }
                ReplyMsg((struct Message *)msg);
            }
        }
        return(w);
    }
    return(NULL);
}

int main(void)
{
    struct Screen *s;

    if (s = OpenScreenTags(NULL,
        SA_Width, 320,
        SA_Height, 256,
        SA_Depth, 5,
        SA_DisplayID, LORES_KEY,
        SA_ShowTitle, FALSE,
        SA_Quiet, TRUE,
        TAG_DONE))
    {
        struct Window *w;

        if (w = openEditWindow(s))
        {
            CloseWindow(w);
        }
        CloseScreen(s);
    }
    return(0);
}
