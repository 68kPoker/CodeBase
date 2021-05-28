
#include <intuition/screens.h>
#include <intuition/intuition.h>
#include <dos/dos.h>
#include <libraries/iffparse.h>
#include <exec/interrupts.h>
#include <exec/memory.h>
#include <graphics/gfxmacros.h>
#include <hardware/intbits.h>
#include <hardware/custom.h>

#include <clib/intuition_protos.h>
#include <clib/dos_protos.h>
#include <clib/layers_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>

#include "IFF.h"
#include "Blit.h"

#define DEPTH 5
#define ESC_KEY	0x45

int main(void);

struct copperData
{
	struct ViewPort *vp;
	WORD signal;
	struct Task *task;
};

__far extern struct Custom custom;

extern void myCopper(void);

struct Screen *openScreen(void)
{
	struct Screen *s;
	struct Rectangle dclip =
	{
		0, 0, 319, 255
	};

	if (s = OpenScreenTags(NULL,
		SA_DClip,		&dclip,
		SA_Depth,		DEPTH,
		SA_DisplayID,	LORES_KEY,
		SA_Quiet,		TRUE,
		SA_ShowTitle,	FALSE,
		SA_Exclusive,	TRUE,
		SA_BackFill,	LAYERS_NOBACKFILL,
		SA_Interleaved,	TRUE,
		TAG_DONE))
	{
		/* Make Bar layer 16 pixel height */
		SizeLayer(0, s->BarLayer, 0, 16 - (s->BarHeight + 1));
		return(s);
	}
	return(NULL);
}

BOOL addCopper(struct Screen *s, struct Interrupt *is, struct copperData *cd)
{
	is->is_Code = myCopper;
	is->is_Data = (APTR)cd;
	is->is_Node.ln_Pri = 0;

	cd->vp = &s->ViewPort;

	if ((cd->signal = AllocSignal(-1)) != -1)
	{
		struct UCopList *ucl;

		cd->task = FindTask(NULL);

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

			AddIntServer(INTB_COPER, is);

			return(TRUE);
		}
		FreeSignal(cd->signal);
	}
	return(FALSE);
}

void remCopper(struct Interrupt *is)
{
	struct copperData *cd = (struct copperData *)is->is_Data;

	RemIntServer(INTB_COPER, is);
	FreeSignal(cd->signal);
}

struct Window *openBackdropWindow(struct Screen *s)
{
	struct Window *w;

	if (w = OpenWindowTags(NULL,
		WA_CustomScreen,	s,
		WA_Left,			0,
		WA_Top,				16,
		WA_Width,			320,
		WA_Height,			240,
		WA_AutoAdjust,		FALSE,
		WA_Backdrop,		TRUE,
		WA_Borderless,		TRUE,
		WA_RMBTrap,			TRUE,
		WA_SimpleRefresh,	TRUE,
		WA_BackFill,		LAYERS_NOBACKFILL,
		WA_Activate,		TRUE,
		WA_IDCMP,			IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE,
		WA_ReportMouse,		TRUE,
		TAG_DONE))
	{
		return(w);
	}
	return(NULL);
}

struct BitMap *loadGfx(struct Window *w, STRPTR name)
{
	struct IFFHandle *iff;

	if (iff = openIFile(name, IFFF_READ))
	{
		if (scanILBM(iff))
		{
			UBYTE *cmap;
			WORD colors;
			if (obtainCMAP(iff, &cmap, &colors))
			{
				struct BitMap *gfx;
				UBYTE cmp, depth;
				WORD bpr, rows;

				loadCMAP(w->WScreen->ViewPort.ColorMap, cmap, colors);
				MakeScreen(w->WScreen);
				RethinkDisplay();

				if (obtainBMHD(iff, &cmp, &bpr, &rows, &depth))
				{
					if (gfx = loadILBM(iff, cmp, bpr, rows, depth))
					{
						closeIFile(iff);
						return(gfx);
					}
				}
			}
		}
		closeIFile(iff);
	}
	return(NULL);
}

void waitVBlank(struct copperData *cd)
{
	ULONG mask = 1L << cd->signal;

	SetSignal(0L, mask);
	Wait(mask);
}

void handleWindow(struct Window *w, struct BitMap *gfx)
{
	ULONG signals[] =
	{
		1L << w->UserPort->mp_SigBit
	};
	BOOL done = FALSE;

	do
	{
		struct IntuiMessage *msg;
		WaitPort(w->UserPort);

		while ((!done) && (msg = (struct IntuiMessage *)GetMsg(w->UserPort)))
		{
			ULONG class = msg->Class;
			WORD code = msg->Code;
			WORD mx = msg->MouseX, my = msg->MouseY;

			ReplyMsg((struct Message *)msg);

			if (class == IDCMP_RAWKEY)
			{
				if (code == ESC_KEY)
				{
					done = TRUE;
				}
			}
			else if (class == IDCMP_MOUSEBUTTONS)
			{
				if (code == IECODE_LBUTTON)
				{
					bltTileRastPort(gfx, 3 << 4, 1 << 4, w->RPort, mx & 0xfff0, my & 0xfff0, 16, 16);
				}
			}
		}
	}
	while (!done);
}

int main(void)
{
	struct Screen *s;

	if (s = openScreen())
	{
		struct copperData cd;
		struct Interrupt is;

		if (addCopper(s, &is, &cd))
		{
			struct Window *w;

			if (w = openBackdropWindow(s))
			{
				struct BitMap *gfx;

				if (gfx = loadGfx(w, "Dane/Magazyn.iff"))
				{
					waitVBlank(&cd);
					bltTileRastPort(gfx, 0, 0, s->BarLayer->rp, 0, 0, 320, 16);
					bltBoardRastPort(gfx, 0, 0, w->RPort, 0, 0, 320, 240);

					handleWindow(w, gfx);
					FreeBitMap(gfx);
				}
				CloseWindow(w);
			}
			remCopper(&is);
		}
		CloseScreen(s);
	}
	return(RETURN_OK);
}
