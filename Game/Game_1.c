
#include <dos/dos.h>

#include "Screen.h"

void test(void)
{
    struct screen s;
    struct graphics gfx;

    if (openScreen(&s, 320, 256, 5, &gfx, "Data/Graphics.iff"))
    {
        Delay(400);
        closeScreen(&s);
    }
}

int main(void)
{
    test();
    return(0);
}
