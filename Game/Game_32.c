
#include <dos/dos.h>
#include <libraries/iffparse.h>
#include <clib/iffparse_protos.h>
#include <clib/graphics_protos.h>
#include "Game.h"
#include "Init.h"
#include "IFF.h"

#include "debug.h"

BOOL loadBoard(STRPTR name, struct boardData *bd)
{
    struct IFFHandle *iff;
    BOOL result = FALSE;

    if (iff = openIFF(name, IFFF_READ))
    {
        if (!PropChunk(iff, ID_MAGA, ID_NAGL))
        {
            if (!PropChunk(iff, ID_MAGA, ID_STAN))
            {
                if (!StopChunk(iff, ID_MAGA, ID_PLAN))
                {
                    if (!StopOnExit(iff, ID_MAGA, ID_FORM))
                    {
                        LONG err = ParseIFF(iff, IFFPARSE_SCAN);
                        if (err == 0 || err == IFFERR_EOC || err == IFFERR_EOF)
                        {
                            struct StoredProperty *sp;
                            if (sp = FindProp(iff, ID_MAGA, ID_STAN))
                            {
                                D(bug("STAN chunk found.\n"));
                            }
                            if (sp = FindProp(iff, ID_MAGA, ID_NAGL))
                            {
                                ULONG version = *(ULONG *)sp->sp_Data;
                                D(bug("NAGL chunk found.\n"));
                                D(bug("Game version: %d\n", version));

                                if (version == 1)
                                {
                                    ULONG actual;
                                    actual = ReadChunkBytes(iff, bd->board, sizeof(bd->board));
                                    D(bug("Read %d bytes.\n", actual));
                                    result = TRUE;
                                }
                            }
                        }
                    }
                }
            }
        }
        closeIFF(iff);
    }
    return(result);
}

int main(void)
{
    struct initData id;
    ULONG err;

    if ((err = initAll(&id)) == NO_ERRORS)
    {
        struct IFFHandle *iff;
        if (iff = openIFF("Data1/Gfx/Template.iff", IFFF_READ))
        {
            if (scanILBM(iff))
            {
                if (loadCMAP(iff, id.s))
                {
                    struct BitMap *gfx;
                    if (id.gfx = gfx = loadBitMap(iff))
                    {
                        closeIFF(iff);
                        iff = NULL;
                        err = mainLoop(&id);
                        FreeBitMap(gfx);
                    }
                }
            }
            if (iff)
                closeIFF(iff);
        }
        cleanAll(&id);
    }
    printError(err);
    return(RETURN_OK);
}
