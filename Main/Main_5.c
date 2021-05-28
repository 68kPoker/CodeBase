
#include "GUI.h"
#include "GUI_protos.h"

#include <clib/dos_protos.h>

int main()
{
    struct ekranInfo ei;
    struct oknoInfo oi;

    ei.ta.ta_Name = "topazpl.font";
    ei.ta.ta_YSize = 9;
    ei.ta.ta_Style = FS_NORMAL;
    ei.ta.ta_Flags = FPF_DISKFONT|FPF_DESIGNED;

    if (otworzEkran(&ei))
    {
        if (otworzOkno(&oi, OKNO_GLOWNE, &ei))
        {
            Delay(600);
            zamknijOkno(&oi);
        }
        zamknijEkran(&ei);
    }
    return(0);
}
