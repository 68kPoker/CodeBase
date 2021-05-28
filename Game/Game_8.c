
#include "IFF.h"
#include "Screen.h"

#include <intuition/screens.h>

#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>

WORD init(void)
{
	struct BitMap *bm;
	struct ColorMap *cm = NULL;

	if (bm = loadBitMap("Data/Magazyn.iff", &cm))
	{
		struct Screen *s;

		if (s = openScreen(cm, GetBitMapAttr(bm, BMA_DEPTH)))
		{
			struct Window *w;

			if (w = openWindow(s))
			{
				Delay(50);
				WaitTOF();
				BltBitMap(bm, 0, 0, w->RPort->BitMap, 0, 0, 320, 256, 0xc0, 0xff, NULL);
				Delay(400);
				CloseWindow(w);
			}
			CloseScreen(s);
		}
		FreeColorMap(cm);
		FreeBitMap(bm);
	}
	return(0);
}

int main(void)
{
	init();
	return(0);
}
