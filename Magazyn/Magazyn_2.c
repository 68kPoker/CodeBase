
#include "GWLibs.h"
#include "GWScreen.h"

#include <dos/dos.h>
#include <clib/dos_protos.h>

int main(void)
{
    if (GWopenLibs(39L))
    {
        struct GWscreen *s;
        if (s = GWgetScreen(FALSE))
        {
            GWprint("Waiting 6 seconds...\n");
            Delay(300);
            GWprint("Game terminated OK. Cleaning up.\n");
            return(RETURN_OK);
        }
    }
    return(RETURN_FAIL);
}
