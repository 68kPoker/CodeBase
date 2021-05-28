
#include <intuition/screens.h>
#include <graphics/rpattr.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>

#include "Init.h"
#include "Screen.h"
#include "Blit.h"

enum
{
	BACKWIN,
	MENUWIN,
	WINDOWS
};

enum
{
	BACKSIG,
	MENUSIG,
	SIGNALS
};

void refreshBoard(struct Window *w, struct BitMap *gfx, ULONG sigmask)
{
	struct Rectangle rect;
	WORD width, height;

	BeginRefresh(w);
	GetRPAttrs(w->RPort, RPTAG_DrawBounds, &rect, TAG_DONE);

	SetSignal(0L, sigmask);
	Wait(sigmask);

/*
	for (y = rect.MinY >> 4; y < (rect.MaxY + 15) >> 4; y++)
	{
		for (x = rect.MinX >> 4; x < (rect.MaxX + 15) >> 4; x++)
		{
			WORD src = 1;
			if (x == 0 || y == 1 || x == 19 || y == 15)
			src = 0;
			bltTileRastPort(gfx, src << 4, 1 << 4, w->RPort, x << 4, y << 4, 16, 16);
		}
	}
*/
	width = rect.MaxX - rect.MinX + 1;
	height = rect.MaxY - rect.MinY + 1;

	bltBoardRastPort(gfx, rect.MinX, rect.MinY, w->RPort, rect.MinX, rect.MinY, width, height);

	EndRefresh(w, TRUE);
}

int main(void)
{
	struct screenAux sa;
	struct BitMap *gfx;
	struct Window *w[WINDOWS];

	if (gfx = initScreen(&sa, "Dane/Grafika.iff"))
	{
		if (initWindows(sa.screen, gfx, w))
		{
			BOOL done = FALSE;

			SetSignal(0L, 1L << sa.copper.signal);
			Wait(1L << sa.copper.signal);

			/* Setup title/menu bar */
			bltTileRastPort(gfx, 0, 0, w[BACKWIN]->RPort, 0, 0, 320, 16);
			bltTileRastPort(gfx, 16, 0, w[MENUWIN]->RPort, 0, 0, 64, 16);


			bltBoardRastPort(gfx, 0, 0, w[BACKWIN]->RPort, 0, 16, 320, 240);

			while (!done)
			{
				ULONG signals[SIGNALS] =
				{
					1L << w[BACKWIN]->UserPort->mp_SigBit,
					1L << w[MENUWIN]->UserPort->mp_SigBit
				}, total = signals[0] | signals[1], result;

				result = Wait(total);

				if (result & signals[BACKSIG])
				{
					struct MsgPort *mp = w[BACKWIN]->UserPort;
					struct IntuiMessage *msg;
					while (msg = (struct IntuiMessage *)GetMsg(mp))
					{
						ULONG class = msg->Class;
						WORD code = msg->Code;

						ReplyMsg((struct Message *)msg);

						if (class == IDCMP_RAWKEY)
						{
							if (code == 0x45)
							{
								done = TRUE;
							}
						}
						else if (class == IDCMP_REFRESHWINDOW)
						{
							refreshBoard(w[BACKWIN], gfx, 1L << sa.copper.signal);
						}
					}
				}

				if (result & signals[MENUSIG])
				{
					struct MsgPort *mp = w[MENUWIN]->UserPort;
					struct IntuiMessage *msg;
					while (msg = (struct IntuiMessage *)GetMsg(mp))
					{
						ULONG class = msg->Class;
						WORD code = msg->Code;

						ReplyMsg((struct Message *)msg);

						if (class == IDCMP_ACTIVEWINDOW)
						{
							ChangeWindowBox(w[MENUWIN], 16, 0, 64, 256);
						}
						else if (class == IDCMP_INACTIVEWINDOW)
						{
							ChangeWindowBox(w[MENUWIN], 16, 0, 64, 16);
						}
						else if (class == IDCMP_NEWSIZE)
						{
							if (w[MENUWIN]->Height > 16)
							{
								SetSignal(0L, 1L << sa.copper.signal);
								Wait(1L << sa.copper.signal);

								bltTileRastPort(gfx, 16, 32, w[MENUWIN]->RPort, 0, 0, 64, 16);

								bltTileRastPort(gfx, 256, 16, w[MENUWIN]->RPort, 0, 16, 64, 240);
							}
							else
							{
								bltTileRastPort(gfx, 16, 0, w[MENUWIN]->RPort, 0, 0, 64, 16);
							}
						}
					}
				}
			}


			freeWindows(w);
		}
		freeScreen(&sa, gfx);
	}
	return(0);
}
