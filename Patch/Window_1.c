
#include <intuition/intuition.h>
#include <graphics/gfxmacros.h>
#include <hardware/intbits.h>
#include <hardware/custom.h>
#include <exec/memory.h>
#include <exec/interrupts.h>

#include <clib/intuition_protos.h>
#include <clib/exec_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>

__chip UWORD pat[] = { 0x5555, 0xaaaa };

__far extern struct Custom custom;

extern void myCopper(void);

struct copInfo
{
	struct ViewPort *vp;
	WORD signal;
	struct Task *task;
};

struct BitMap *loadGfx(STRPTR name, struct Window *w);

struct UCopList *addCopper(struct Interrupt *is, struct copInfo *cop)
{
	struct UCopList *ucl;

	is->is_Code = myCopper;
	is->is_Data = (APTR)cop;
	is->is_Node.ln_Pri = 0;

	if (ucl = AllocMem(sizeof(*ucl), MEMF_PUBLIC|MEMF_CLEAR))
	{
		CINIT(ucl, 3);
		CWAIT(ucl, 255, 0);
		CMOVE(ucl, custom.intreq, INTF_SETCLR|INTF_COPER);
		CEND(ucl);

		Forbid();
		cop->vp->UCopIns = ucl;
		Permit();

		RethinkDisplay();

		AddIntServer(INTB_COPER, is);

		return(ucl);
	}
	return(NULL);
}

struct Window *openWindow(struct BitMap **gfx)
{
	struct Window *w;
	static UWORD zoom[] = { 96, 16, 448, 11 };

	if (w = OpenWindowTags(NULL,
		WA_Left,		96,
		WA_Top,			16,
		WA_InnerWidth,	448,
		WA_InnerHeight,	224,
		WA_Title,		"Magazyn",
		WA_ScreenTitle,	"Magazyn ®2021 Robert Szacki",
		WA_CloseGadget,	TRUE,
		WA_DepthGadget,	TRUE,
		WA_DragBar,		TRUE,
		WA_Activate,	TRUE,
		WA_IDCMP,		IDCMP_CLOSEWINDOW|IDCMP_REFRESHWINDOW|IDCMP_ACTIVEWINDOW|IDCMP_INACTIVEWINDOW,
		WA_BackFill,	LAYERS_NOBACKFILL,
		WA_SimpleRefresh,	TRUE,
		WA_Zoom,		zoom,
		TAG_DONE))
	{
		SetAPen(w->RPort, 3);
		SetAfPt(w->RPort, pat, 1);

		RectFill(w->RPort, w->BorderLeft, w->BorderTop, w->BorderLeft + w->GZZWidth - 1, w->BorderTop + w->GZZHeight - 1);

		if (*gfx = loadGfx("Dane/Magazyn.iff", w))
		{
			return(w);
		}
		CloseWindow(w);
	}
	return(NULL);
}


void drawWindow(struct Window *w, struct BitMap *gfx, ULONG copmask)
{
	SetSignal(0L, copmask);
	Wait(copmask);

	BltBitMapRastPort(gfx, 0, 0, w->RPort, w->BorderLeft, w->BorderTop, w->GZZWidth, w->GZZHeight, 0xc0);
}

int processWindow(struct Window *w, struct BitMap *gfx, ULONG copmask)
{
	BOOL done = FALSE;
	BOOL installed = FALSE;
	struct IntuiMessage *msg;
	struct RastPort *rp = w->RPort;

	drawWindow(w, gfx, copmask);

	while (!done)
	{
		WaitPort(w->UserPort);

		while (msg = GT_GetIMsg(w->UserPort))
		{
			if (msg->Class == IDCMP_CLOSEWINDOW)
			{
				done = TRUE;
			}
			else if (msg->Class == IDCMP_ACTIVEWINDOW)
			{
				installPatch();
				installed = TRUE;
			}
			else if (msg->Class == IDCMP_INACTIVEWINDOW)
			{
				removePatch();
				installed = FALSE;
			}
			else if (msg->Class == IDCMP_REFRESHWINDOW)
			{
				drawWindow(w, gfx, copmask);
			}

			GT_ReplyIMsg(msg);
		}
	}

	if (installed)
	{
		removePatch();
	}
}

int main(void)
{
	struct Window *w;
	struct Interrupt is;
	struct copInfo cop;
	struct BitMap *gfx;
	struct UCopList *ucl;

	if (w = openWindow(&gfx))
	{
		if ((cop.signal = AllocSignal(-1)) != -1)
		{
			cop.vp = &w->WScreen->ViewPort;
			cop.task = FindTask(NULL);
			if (ucl = addCopper(&is, &cop))
			{
				processWindow(w, gfx, 1L << cop.signal);
				RemIntServer(INTB_COPER, &is);

				/* FreeMem(ucl, sizeof(*ucl)); */
				FreeVPortCopLists(cop.vp);
			}
			FreeSignal(cop.signal);
		}
		FreeBitMap(gfx);
		CloseWindow(w);
	}
	return(0);
}
