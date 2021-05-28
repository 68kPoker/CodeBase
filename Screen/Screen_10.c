
/* Screen.c - Ekran, video */

/* $Log$ */

#include <intuition/screens.h>
#include <intuition/intuition.h>
#include <graphics/gfxmacros.h>
#include <hardware/intbits.h>
#include <hardware/custom.h>
#include <exec/interrupts.h>
#include <exec/memory.h>

#include <clib/intuition_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/diskfont_protos.h>

#include "Image.h"
#include "Screen.h"

extern void myCopper(void);
__far extern struct Custom custom;

struct IntuiText menuit =
{
	0, 1,
	JAM1,
	10, 4,
	NULL,
	"Menu gry",
	NULL
};

struct IntuiText menu2it =
{
	0, 1,
	JAM1,
	4, 4,
	NULL,
	"Konstruktor",
	NULL
};

struct Image menuimg, depthimg, closeimg;

struct Gadget menu2gad =
{
	NULL,
	80, 0,
	64, 16,
	GFLG_GADGIMAGE | GFLG_GADGHCOMP,
	GACT_RELVERIFY,
	GTYP_BOOLGADGET,
	&menuimg,
	NULL,
	&menu2it,
	0,
	NULL,
	0,
	NULL
};

struct Gadget menugad =
{
	&menu2gad,
	16, 0,
	64, 16,
	GFLG_GADGIMAGE | GFLG_GADGHCOMP,
	GACT_RELVERIFY,
	GTYP_BOOLGADGET,
	&menuimg,
	NULL,
	&menuit,
	0,
	NULL,
	0,
	NULL
};

struct Gadget depthgad =
{
	&menugad,
	304, 0,
	16, 16,
	GFLG_GADGIMAGE | GFLG_GADGHCOMP,
	GACT_RELVERIFY,
	GTYP_SDEPTH,
	&depthimg,
	NULL,
	NULL,
	0,
	NULL,
	0,
	NULL
};

struct Gadget closegad =
{
	&depthgad,
	0, 0,
	16, 16,
	GFLG_GADGIMAGE | GFLG_GADGHCOMP,
	GACT_RELVERIFY,
	GTYP_CLOSE,
	&closeimg,
	NULL,
	NULL,
	0,
	NULL,
	0,
	NULL
};

struct TextAttr ta = { "centurion.font", 9, FS_NORMAL, FPF_DISKFONT|FPF_DESIGNED };
struct TextFont *tf;

struct Interrupt is;
struct copperData copperData;

/* openScreen() - Otwórz ekran */

struct Screen *openScreen(void)
{
	struct Rectangle dclip = { 0, 0, 319, 255 };
	UWORD pens[] = { ~0 };
	struct ColorSpec colspec[] = { { 0, 0, 0, 0 }, { -1 } };

	if ((tf = OpenDiskFont(&ta)) == NULL)
	{
		return(NULL);
	}

	struct Screen *s = OpenScreenTags(NULL,
		SA_Title,		"Magazyn",
		SA_DisplayID,	LORES_KEY,
		SA_Depth,		5,
		SA_DClip,		&dclip,
		SA_Pens,		pens,
		SA_SharePens,	TRUE,
		SA_Interleaved,	TRUE,
		SA_BackFill,	LAYERS_NOBACKFILL,
		SA_Exclusive,	TRUE,
		SA_ShowTitle,	TRUE,
		SA_Quiet,		TRUE,
		SA_Font,		&ta,
		SA_Colors,		colspec,
		TAG_DONE);

	if (s)
	{
		is.is_Code = myCopper;
		is.is_Data = (APTR)&copperData;
		is.is_Node.ln_Pri = 0;

		copperData.vp = &s->ViewPort;

		if ((copperData.signal = AllocSignal(-1)) != -1)
		{
			struct UCopList *ucl;

			copperData.task = FindTask(NULL);

			if (ucl = AllocMem(sizeof(*ucl), MEMF_PUBLIC|MEMF_CLEAR))
			{
				CINIT(ucl, 3);
				CWAIT(ucl, 128, 0);
				CMOVE(ucl, custom.intreq, INTF_SETCLR|INTF_COPER);
				CEND(ucl);

				Forbid();
				s->ViewPort.UCopIns = ucl;
				Permit();

				RethinkDisplay();

				AddIntServer(INTB_COPER, &is);
				return(s);
			}
			FreeSignal(copperData.signal);
		}
		CloseScreen(s);
	}

	CloseFont(tf);
	return(NULL);
}

/* closeScreen() - Zamknij ekran */

void closeScreen(struct Screen *s)
{
	RemIntServer(INTB_COPER, &is);
	FreeSignal(copperData.signal);
	CloseScreen(s);
}

BOOL prepImages(struct BitMap *gfx)
{
	UWORD *data;

	if (data = allocImageData(16, 16, 5))
	{
		cutImageFromBitMap(data, gfx, 0, 0, 16, 16, 5);
		initImage(&closeimg, data, 16, 16, 5, 0xff, 0x00);

		if (data = allocImageData(16, 16, 5))
		{
			cutImageFromBitMap(data, gfx, 304, 0, 16, 16, 5);
			initImage(&depthimg, data, 16, 16, 5, 0xff, 0x00);

			if (data = allocImageData(64, 16, 5))
			{
				cutImageFromBitMap(data, gfx, 16, 0, 64, 16, 5);
				initImage(&menuimg, data, 64, 16, 5, 0xff, 0x00);
				return(TRUE);
			}
			FreeVec(depthimg.ImageData);
		}
		FreeVec(closeimg.ImageData);
	}
	return(FALSE);
}

void freeImages(void)
{
	FreeVec(menuimg.ImageData);
	FreeVec(depthimg.ImageData);
	FreeVec(closeimg.ImageData);
}

/* openBoardWindow() - Otwórz glowne okienko */

struct Window *openBoardWindow(struct Screen *s, struct BitMap *gfx)
{
	if (prepImages(gfx))
	{
		struct Window *w = OpenWindowTags(NULL,
			WA_CustomScreen,	s,
			WA_Top,				0,
			WA_InnerWidth,		320,
			WA_InnerHeight,		256,
			WA_SimpleRefresh,	TRUE,
			WA_IDCMP,			IDCMP_CLOSEWINDOW,
			WA_Borderless,		TRUE,
			WA_MinWidth,		320,
			WA_MinHeight,		224,
			WA_MaxWidth,		640,
			WA_MaxHeight,		448,
			WA_Gadgets,			&closegad,
			WA_BackFill,		LAYERS_NOBACKFILL,
			WA_RMBTrap,			TRUE,
			WA_Activate,		TRUE,
			TAG_DONE);

		if (w)
		{
			return(w);
		}
		freeImages();
	}

	return(NULL);
}

void closeWindow(struct Window *w)
{
	CloseWindow(w);
	freeImages();
}
