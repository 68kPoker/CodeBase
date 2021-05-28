
#include "Windows.h"

#include <dos/dos.h>
#include <intuition/intuition.h>

#include <clib/graphics_protos.h>

STRPTR TITLE = "Magazyn";

/* Board window dispatcher */

LONG dispatchBoard(struct Window *w, WORD type, union message *wm)
{
	switch (type)
	{
		case MTYPE_IDCMP:

			ULONG class = wm->msg->Class;
			WORD code = wm->msg->Code;

			if (class == IDCMP_RAWKEY)
			{
				if (code == 0x45)
				{
					return(TRUE);
				}
			}
			break;

		case MTYPE_GAMEPORT:
			break;

		case MTYPE_VBLANK:
			static WORD counter = 0, seconds = 0;
			UBYTE text[5];
			struct RastPort *rp = wm->rp;
			Move(rp, 0, rp->Font->tf_Baseline);
			SetAPen(rp, 1);
			if (++counter == 50)
			{
				counter = 0;
				seconds++;
				sprintf(text, "%4d", seconds);
				Text(rp, text, 4);
			}
			break;
	}
	return(FALSE);
}

int main(void)
{
	struct screenUD sud;
	struct windowUD wud;
	struct Screen *s;
	struct Window *w;

	if (s = openScreen(&sud))
	{
		if (w = openMainWindow(s, &wud))
		{
			wud.dispatch = dispatchBoard;
			mainLoop(w);
			CloseWindow(w);
		}
		closeScreen(s);
	}
	return(RETURN_OK);
}
