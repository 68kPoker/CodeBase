
#include <intuition/screens.h>
#include <hardware/custom.h>
#include <hardware/intbits.h>
#include <exec/memory.h>
#include <exec/interrupts.h>
#include <datatypes/pictureclass.h>
#include <graphics/gfxmacros.h>

#include <clib/iffparse_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/intuition_protos.h>

#include "Screen.h"

#define RGB(c) ((c)|((c)<<8)|((c)<<16)|((c)<<24))

extern __far struct Custom custom;
extern void myCopper(void);

/* Przed otwarciem ekranu potrzebujë:
 * - Zaalokowaê bitmapë ekranowâ,
 * - Otworzyê czcionkë,
 * - Wczytaê paletë kolorów z pliku IFF.
 *
 * Po otwarciu ekranu potrzebujë:
 * - Zainstalowaê przerwanie Coppera.
 */

BOOL preOpenScreen(struct screenAux *sa, UWORD width, UWORD height, UBYTE depth, struct TextAttr *ta, struct IFFHandle *iff)
{
	/* Alokujë bitmapë o wybranym rozmiarze */

	if (sa->bitmap = AllocBitMap(width, height, depth, BMF_DISPLAYABLE|BMF_INTERLEAVED|BMF_CLEAR, NULL))
	{
		/* Otwieram czcionkë */

		sa->ta = ta;
		if (sa->font = OpenDiskFont(ta))
		{
			/* Wczytujë paletë kolorów ze strumienia IFF */

			struct StoredProperty *sp;
			if (sp = FindProp(iff, ID_ILBM, ID_CMAP))
			{
				UBYTE *cmap = sp->sp_Data;
				LONG size = sp->sp_Size;
				WORD count = size / 3, i;

				if (sa->colors = AllocVec((size + 2) * sizeof(ULONG), MEMF_PUBLIC))
				{
					sa->colors[0] = count << 16;
					for (i = 0; i < size; i++)
					{
						UBYTE data = *cmap++;
						sa->colors[i + 1] = RGB(data);
					}
					sa->colors[size + 1] = 0L;
					return(TRUE);
				}
			}
			CloseFont(sa->font);
		}
		FreeBitMap(sa->bitmap);
	}
	return(FALSE);
}

struct Screen *openScreen(struct screenAux *sa)
{
	struct Rectangle dclip = { 0, 0, 319, 255 };
	struct Screen *s;

	/*

	if (s = OpenScreenTags(NULL,
		SA_BitMap,		sa->bitmap,
		SA_Font,		sa->ta,
		SA_Colors32,	sa->colors,
		SA_DClip,		&dclip,
		SA_Exclusive,	TRUE,
		SA_ShowTitle,	FALSE,
		SA_Quiet,		TRUE,
		SA_BackFill,	LAYERS_NOBACKFILL,
		TAG_DONE))
	{
	*/

	if (s = LockPubScreen("DESKTOP.1"))
	{
		LoadRGB32(&s->ViewPort, sa->colors);
		return(sa->screen = s);
	}
	return(NULL);
}

BOOL postOpenScreen(struct screenAux *sa)
{
	struct copperAux *ca = &sa->copper;
	struct Interrupt *is = &sa->is;
	struct UCopList *ucl;

	/* Alokujë sygnaî */

	if ((ca->signal = AllocSignal(-1)) != -1)
	{
		ca->vp = &sa->screen->ViewPort;
		ca->task = FindTask(NULL);

		is->is_Data = (APTR)ca;
		is->is_Code = myCopper;
		is->is_Node.ln_Pri = 0;

		/* Alokujë copperlistë uûytkownika */

		if (ucl = AllocMem(sizeof(*ucl), MEMF_PUBLIC|MEMF_CLEAR))
		{
			CINIT(ucl, 3);
			CWAIT(ucl, 0, 0);
			CMOVE(ucl, custom.intreq, INTF_SETCLR|INTF_COPER);
			CEND(ucl);

			Forbid();
			ca->vp->UCopIns = ucl;
			Permit();

			RethinkDisplay();

			AddIntServer(INTB_COPER, is);
			return(TRUE);
		}
		FreeSignal(ca->signal);
	}
	return(FALSE);
}

void preCloseScreen(struct screenAux *sa)
{
	RemIntServer(INTB_COPER, &sa->is);

	FreeSignal(sa->copper.signal);
}

void postCloseScreen(struct screenAux *sa)
{
	FreeVec(sa->colors);
	CloseFont(sa->font);
	FreeBitMap(sa->bitmap);
}
