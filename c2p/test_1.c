
#include <stdio.h>
#include <graphics/view.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>

extern void writePixelLine8(register __a0 UBYTE *pixels, register __a1 struct BitMap *bm, register __d0 UWORD line, register __d1 UWORD wordPos, register __d2 UWORD wordWidth);

int main()
{
    struct BitMap *bm;

    if (bm = AllocBitMap(320, 256, 8, BMF_DISPLAYABLE|BMF_INTERLEAVED|BMF_CLEAR, NULL))
    {
        struct View view;
        struct ViewPort vp;
        struct RasInfo ri;

        InitView(&view);
        view.ViewPort = &vp;
        InitVPort(&vp);
        vp.DxOffset = 0;
        vp.DyOffset = 0;
        vp.DWidth = 320;
        vp.DHeight = 256;
        vp.RasInfo = &ri;
        if (vp.ColorMap = GetColorMap(256))
        {
            WORD i;
            static UBYTE pixels[320];

            vp.Next = NULL;
            ri.RxOffset = 0;
            ri.RyOffset = 0;
            ri.BitMap = bm;
            ri.Next = NULL;
            MakeVPort(&view, &vp);
            MrgCop(&view);
            LoadView(&view);

            for (i = 0; i < 320; i++)
            {
                pixels[i] = 1;
            }

            for (i = 0; i < 256; i++)
            {
                writePixelLine8(pixels, bm, i, 4, 1);
            }
            Delay(400);

            LoadView(ViewAddress());
            WaitTOF();
            WaitTOF();
            FreeCprList(view.LOFCprList);
            FreeVPortCopLists(&vp);
            FreeColorMap(vp.ColorMap);
        }
        FreeBitMap(bm);
    }
    return(0);
}
