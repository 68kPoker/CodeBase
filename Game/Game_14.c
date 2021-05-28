
#include <dos/dos.h>
#include <exec/memory.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include "GameRoot.h"

int main()
{
    struct gameRoot *gr;

    if (gr = AllocMem(sizeof(*gr), MEMF_PUBLIC))
    {
        if (initGame(gr))
        {
            gr->tile = 0;
            playGame(gr);
            closeGame(gr);
        }
        FreeMem(gr, sizeof(*gr));
    }
    return(RETURN_OK);
}
