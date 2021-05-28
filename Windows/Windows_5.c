
#include "Okna.h"
#include "Ekran.h"
#include "Gadzety.h"

#include <clib/intuition_protos.h>

void initSysGadgets(window *w)
{
	struct Gadget *gad = &w->closegad;

	gad->LeftEdge = 0;
	gad->TopEdge = 0;
	gad->Width = 16;
	gad->Height = 16;
	gad->Flags = GFLG_GADGIMAGE | GFLG_GADGHIMAGE;
	gad->Activation = GACT_RELVERIFY;
	gad->GadgetType = GTYP_CLOSE;
	gad->GadgetRender = &Gadzet1;
	gad->SelectRender = &Gadzet2;
	gad->NextGadget = NULL;
}

BOOL openBackWindow(screen *s, window *w)
{
	initSysGadgets(w);

	if (w->w = OpenWindowTags(NULL,
		WA_CustomScreen,	s->s,
		WA_Left,			0,
		WA_Top,				0,
		WA_Width,			s->s->Width,
		WA_Height,			s->s->Height,
		WA_Backdrop,		TRUE,
		WA_Borderless,		TRUE,
		WA_Activate,		TRUE,
		WA_RMBTrap,			TRUE,
		WA_IDCMP,			IDCMP_RAWKEY | IDCMP_CLOSEWINDOW,
		WA_Gadgets,			&w->closegad,
		TAG_DONE))
	{
		return(TRUE);
	}
	return(FALSE);
}

void closeWindow(window *w)
{
	CloseWindow(w->w);
}
