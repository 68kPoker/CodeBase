
#include <intuition/screens.h>
#include <intuition/intuition.h>

#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>

#include "Screen.h"

/*
** Open new screen.
*/
struct Screen *openScreen(struct ColorMap *cm, UBYTE depth)
{
	struct Screen *s;
	struct Rectangle dclip = { 0, 0, 319, 255 };

	if (s = OpenScreenTags(NULL,
		SA_DClip,		&dclip,
		SA_Depth,		depth,
		SA_DisplayID,	LORES_KEY,
		SA_Quiet,		TRUE,
		SA_Exclusive,	TRUE,
		SA_ShowTitle,	FALSE,
		SA_BackFill,	LAYERS_NOBACKFILL,
		SA_Title,		"Magazyn",
		SA_Interleaved,	TRUE,
		TAG_DONE))
	{
		ULONG rgb[3];
		WORD i, colors = 1 << depth;

		for (i = 0; i < colors; i++)
		{
			GetRGB32(cm, i, 1, rgb);
			SetRGB32CM(s->ViewPort.ColorMap, i, rgb[0], rgb[1], rgb[2]);
		}
		MakeScreen(s);
		RethinkDisplay();

		return(s);
	}
	return(NULL);
}

struct Window *openWindow(struct Screen *s)
{
	struct Window *w;

	if (w = OpenWindowTags(NULL,
		WA_CustomScreen,	s,
		WA_Left,			0,
		WA_Top,				0,
		WA_Width,			s->Width,
		WA_Height,			s->Height,
		WA_Backdrop,		TRUE,
		WA_Borderless,		TRUE,
		WA_Activate,		TRUE,
		WA_RMBTrap,			TRUE,
		WA_SimpleRefresh,	TRUE,
		WA_BackFill,		LAYERS_NOBACKFILL,
		WA_IDCMP,			IDCMP_MOUSEBUTTONS|IDCMP_RAWKEY,
		TAG_DONE))
	{
		return(w);
	}
	return(NULL);
}
