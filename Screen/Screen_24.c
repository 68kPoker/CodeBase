
#include <intuition/intuition.h>

#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>

#include "Screen.h"

/* bestModeID() - Calculate Mode ID */

ULONG bestModeID()
{
    struct Screen *s;

    if ((s = LockPubScreen(NULL)) != NULL)
    {
        ULONG modeID = BestModeID(
            BIDTAG_ViewPort,        &s->ViewPort,
            BIDTAG_NominalWidth,    WIDTH,
            BIDTAG_NominalHeight,   HEIGHT,
            BIDTAG_Depth,           DEPTH,
            TAG_DONE);

        UnlockPubScreen(NULL, s);
        return(modeID);
    }
    return(INVALID_ID);
}

/* openScreen() - Open game screen */

struct Screen *openScreen()
{
    ULONG modeID;

    if ((modeID = bestModeID()) != INVALID_ID)
    {
        struct Screen *s;

        if (s = OpenScreenTags(NULL,
            SA_Left,        0,
            SA_Top,         0,
            SA_Width,       WIDTH,
            SA_Height,      HEIGHT,
            SA_Depth,       DEPTH,
            SA_DisplayID,   modeID,
            SA_Quiet,       TRUE,
            SA_Exclusive,   TRUE,
            SA_BackFill,    LAYERS_NOBACKFILL,
            SA_Draggable,   FALSE,
            SA_ShowTitle,   FALSE,
            SA_Title,       "Magazyn",
            TAG_DONE))
        {
            return(s);
        }
        else
            printf("Couldn't open screen!\n");
    }
    else
        printf("Couldn't get screen mode!\n");
    return(NULL);
}

VOID closeScreen( struct Screen *s )
{
    CloseScreen( s );
}
