
#include <clib/intuition_protos.h>

#include "IFF.h"
#include "Screen.h"
#include "Windows.h"

static struct TextAttr ta =
{
	"centurion.font",
	9,
	FS_NORMAL,
	FPF_DISKFONT|FPF_DESIGNED
};

struct BitMap *initScreen(struct screenAux *sa, STRPTR name)
{
	struct IFFHandle *iff;

	if (iff = openIFF(name))
	{
		if (scanILBM(iff))
		{
			if (preOpenScreen(sa, 320, 256, 5, &ta, iff))
			{
				if (openScreen(sa))
				{
					if (postOpenScreen(sa))
					{
						struct BitMap *gfx;

						if (gfx = readILBM(iff))
						{
							closeIFF(iff);
							return(gfx);
						}
						preCloseScreen(sa);
					}
					UnlockPubScreen(NULL, sa->screen);
				}
				postCloseScreen(sa);
			}
		}
		closeIFF(iff);
	}
	return(NULL);
}

void freeScreen(struct screenAux *sa, struct BitMap *gfx)
{
	FreeBitMap(gfx);
	preCloseScreen(sa);
	UnlockPubScreen(NULL, sa->screen);
	postCloseScreen(sa);
}

BOOL initWindows(struct Screen *s, struct BitMap *gfx, struct Window *w[])
{
	if (w[0] = openBackWindow(s))
	{
		if (w[1] = openMenuWindow(s, 16, 64, 16))
		{
			return(TRUE);
		}
		CloseWindow(w[0]);
	}
	return(FALSE);
}

void freeWindows(struct Window *w[])
{
	CloseWindow(w[1]);
	CloseWindow(w[0]);
}
