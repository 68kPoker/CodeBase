
#include "System.h"

#include <stdio.h>

#include <intuition/screens.h>

#include <clib/exec_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/graphics_protos.h>

struct TextAttr ta =
{
    "centurion.font",
    9,
    FS_NORMAL,
    FPF_DISKFONT|FPF_DESIGNED
};

BOOL initAll(struct systemInfo *si)
{
    if (si->tf = OpenDiskFont(&ta))
    {
        if (openScreen(si, SA_Font, &ta, TAG_DONE))
        {
            return(TRUE);
        }
        CloseFont(si->tf);
    }
    return(FALSE);
}

void closeAll(struct systemInfo *si)
{
    closeScreen(si);
    CloseFont(si->tf);
}

void play(struct systemInfo *si)
{
    Wait(1L << si->ci.signal);
    printf("Done.\n");
}

int main(void)
{
    struct systemInfo si;

    if (initAll(&si))
    {
        play(&si);
        closeAll(&si);
    }
    return(0);
}
