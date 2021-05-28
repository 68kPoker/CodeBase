
#include <intuition/intuition.h>

#include "IFF.h"

void test();
struct Screen *openScreen(Tag tag1, ...);
struct Window *openWindow(Tag tag1, ...);
struct BitMap *readBitMap(STRPTR name, struct Screen *s);

int main(void)
{
    struct Screen *s;

    if (s = openScreen(SA_Interleaved, TRUE, TAG_DONE))
    {
        struct Window *w;

        if (w = openWindow(WA_CustomScreen, s, TAG_DONE))
        {
            struct BitMap *gfx;
            if (gfx = readBitMap("Data/Graphics.iff", s))
            {
                test(w, gfx);
                FreeBitMap(gfx);
            }
            CloseWindow(w);
        }
        CloseScreen(s);
    }
    return 0;
}
