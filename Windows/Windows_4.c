
#include <intuition/intuition.h>
#include <clib/intuition_protos.h>

/* Gra posiada okno gîówne w tle oraz rozwijane okna menu */

struct Window *openBackWindow(struct Screen *s)
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
		WA_IDCMP,			IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_REFRESHWINDOW,
		WA_SimpleRefresh,	TRUE,
		WA_BackFill,		LAYERS_NOBACKFILL,
		TAG_DONE))
	{
		return(w);
	}
	return(NULL);
}

struct Window *openMenuWindow(struct Screen *s, WORD left, WORD width, WORD height)
{
	struct Window *w;

	if (w = OpenWindowTags(NULL,
		WA_CustomScreen,	s,
		WA_Left,			left,
		WA_Top,				0,
		WA_Width,			width,
		WA_Height,			height,
		WA_Borderless,		TRUE,
		WA_RMBTrap,			TRUE,
		WA_IDCMP,			IDCMP_ACTIVEWINDOW|IDCMP_INACTIVEWINDOW|IDCMP_NEWSIZE|IDCMP_REFRESHWINDOW|IDCMP_MOUSEBUTTONS,
		WA_SimpleRefresh,	TRUE,
		WA_BackFill,		LAYERS_NOBACKFILL,
		TAG_DONE))
	{
		return(w);
	}
	return(NULL);
}
