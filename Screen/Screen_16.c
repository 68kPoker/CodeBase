
#include <intuition/screens.h>
#include <exec/memory.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>

#include "Screen.h"

BOOL openScreen(struct screenInfo *si, ULONG modeID, UBYTE depth)
{
    /* Find dimension info */
    if (GetDisplayInfoData(0, (UBYTE *)&si->dims, sizeof(si->dims), DTAG_DIMS, modeID) <= 0)
    {
        printf("Couldn't get dimension info!\n");
    }
    else
    {
        struct Rectangle *rect = &si->dims.Nominal;
        WORD width  = rect->MaxX - rect->MinX + 1;
        WORD height = rect->MaxY - rect->MinY + 1;

        printf("Screen Information:\n");
        printf("Width  = %d\n", width);
        printf("Height = %d\n", height);
        printf("Depth  = %d\n", depth);
        printf("ModeID = 0x%x\n", modeID);

        if (!(si->bm[ 0 ] = AllocBitMap(width, height, depth, BMF_DISPLAYABLE | BMF_CLEAR, NULL)))
        {
            printf("Couldn't alloc screen bitmap!\n");
        }
        else
        {
            if (!(si->bm[ 1 ] = AllocBitMap(width, height, depth, BMF_DISPLAYABLE | BMF_CLEAR, NULL)))
            {
                printf("Couldn't alloc screen bitmap!\n");
            }
            else
            {
                WORD colors = 1 << depth;
                LONG size = (colors * 3) + 2;
                if (!(si->palette = AllocMem(size * sizeof(ULONG), MEMF_PUBLIC | MEMF_CLEAR)))
                {
                    printf("Couldn't alloc memory for palette!\n");
                }
                else
                {
                    si->palette[0] = colors << 16;
                    if (!(si->s = OpenScreenTags(NULL,
                        SA_Left,        0,
                        SA_Top,         0,
                        SA_BitMap,      si->bm[ 0 ],
                        SA_Colors32,    si->palette,
                        SA_DisplayID,   modeID,
                        SA_Quiet,       TRUE,
                        SA_Exclusive,   TRUE,
                        SA_ShowTitle,   FALSE,
                        SA_Draggable,   FALSE,
                        SA_Title,       "Game Screen",
                        SA_PubName,     "GAME.1",
                        SA_SharePens,   TRUE,
                        SA_BackFill,    LAYERS_NOBACKFILL,
                        TAG_DONE)))
                    {
                        printf("Couldn't open screen!\n");
                    }
                    else
                    {
                        if (!(si->dbi = AllocDBufInfo(&si->s->ViewPort)))
                        {
                            printf("Couldn't alloc dbuf info!\n");
                        }
                        else
                        {
                            si->s->UserData = (APTR) si;
                            return (TRUE);
                        }
                        CloseScreen(si->s);
                    }
                    FreeMem(si->palette, size * sizeof(ULONG));
                }
                FreeBitMap(si->bm[ 1 ]);
            }
            FreeBitMap(si->bm[ 0 ]);
        }
    }
    return (FALSE);
}

void closeScreen(struct screenInfo *si)
{
    WORD colors = 1 << GetBitMapAttr(si->bm[ 0 ], BMA_DEPTH);
    LONG size = (colors * 3) + 2;

    FreeDBufInfo(si->dbi);
    CloseScreen(si->s);
    FreeMem(si->palette, size * sizeof(ULONG));
    FreeBitMap(si->bm[ 1 ]);
    FreeBitMap(si->bm[ 0 ]);
}
