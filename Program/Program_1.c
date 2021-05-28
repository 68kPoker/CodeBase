
#include <intuition/intuition.h>
#include <libraries/iffparse.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>

#include "Screen.h"
#include "IFF.h"
/* #include "Blit.h" */

int main(void)
{
	struct Screen *s = openScreen();

	if (s)
	{
		struct IFFHandle *iff;

		if (iff = openIFile("Dane/Magazyn.iff", IFFF_READ))
		{
			if (scanILBM(iff))
			{
				UBYTE *cmap;
				WORD colors;
				if (obtainCMAP(iff, &cmap, &colors))
				{
					UBYTE cmp, depth;
					WORD bpr, rows;

					loadCMAP(s->ViewPort.ColorMap, cmap, colors);
					MakeScreen(s);
					RethinkDisplay();
					if (obtainBMHD(iff, &cmp, &bpr, &rows, &depth))
					{
						struct BitMap *gfx;

						if (gfx = loadILBM(iff, cmp, bpr, rows, depth))
						{
							struct Window *w = openBoardWindow(s, gfx);

							if (w)
							{
								ULONG copmask = 1L << copperData.signal;
								UBYTE text[] = "Naciônij FIRE by rozpoczâê";
								WaitBlit();
								SetSignal(0L, copmask);
								Wait(copmask);

								/* bltTileRastPort(gfx, 0, 0, w->RPort, 0, 0, w->GZZWidth, 16); */

								Move(w->RPort, 100, 128);
								SetAPen(w->RPort, 31);
								Text(w->RPort, text, sizeof(text) - 1);

								WaitPort(w->UserPort);

								WaitBlit();
								SetSignal(0L, copmask);
								Wait(copmask);

								/* bltBoardRastPort(gfx, 0, 16, w->RPort, 0, 16, w->GZZWidth, 240); */

								closeWindow(w);
							}
							FreeBitMap(gfx);
						}
					}
				}
			}
			closeIFile(iff);
		}
		closeScreen(s);
	}
	return(0);
}
