
/* Magazyn */

/* Wlasne View */

#include <dos/dos.h>
#include <graphics/view.h>
#include <exec/interrupts.h>
#include <exec/memory.h>
#include <graphics/gfxmacros.h>
#include <graphics/gfxbase.h>
#include <hardware/intbits.h>
#include <hardware/custom.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>

#include "Input.h"

__far extern struct Custom custom;

extern struct GfxBase *GfxBase;

void myCopper(void);

struct copper
{
	struct ViewPort	*vp;
	WORD signal;
	struct Task *task;
} cop;

struct View 	view, *oldview;
struct ViewPort vp;
struct RasInfo 	ri;
struct BitMap	*bm[2];
struct ColorMap	*cm;
struct DBufInfo	*dbi;
struct MsgPort	*safeport;
struct Interrupt	is;
BOOL			safeToDraw = TRUE;
UWORD			frame = 1;

BOOL initView(void)
{
	struct UCopList *ucl;

	InitView(&view);
	view.ViewPort = &vp;

	InitVPort(&vp);
	vp.DxOffset = vp.DyOffset = 0;
	vp.DWidth	= 320;
	vp.DHeight 	= 256;
	vp.RasInfo 	= &ri;
	if (vp.ColorMap = cm = GetColorMap(32L))
	{
		vp.Next = NULL;
		if (ri.BitMap = bm[0] = AllocBitMap(320, 256, 5, BMF_DISPLAYABLE|BMF_INTERLEAVED|BMF_CLEAR, NULL))
		{
			if (bm[1] = AllocBitMap(320, 256, 5, BMF_DISPLAYABLE|BMF_INTERLEAVED|BMF_CLEAR, NULL))
			{
				if (dbi = AllocDBufInfo(&vp))
				{
					if (dbi->dbi_SafeMessage.mn_ReplyPort = safeport = CreateMsgPort())
					{
						if ((cop.signal = AllocSignal(-1)) != -1)
						{
							cop.vp = &vp;
							cop.task = FindTask(NULL);
							if (ucl = AllocMem(sizeof(*ucl), MEMF_PUBLIC|MEMF_CLEAR))
							{
								CINIT(ucl, 3);
								CWAIT(ucl, 0, 0);
								CMOVE(ucl, custom.intreq, INTF_SETCLR|INTF_COPER);
								CEND(ucl);

								Forbid();
								vp.UCopIns = ucl;
								Permit();

								is.is_Code = myCopper;
								is.is_Data = (APTR)&cop;
								is.is_Node.ln_Pri = 0;
								is.is_Node.ln_Name = "Magazyn";

								ri.RxOffset = ri.RyOffset = 0;
								ri.Next = NULL;

								AddIntServer(INTB_COPER, &is);

								MakeVPort(&view, &vp);
								MrgCop(&view);

								oldview = GfxBase->ActiView;

								LoadView(&view);

								return(TRUE);
							}
							FreeSignal(cop.signal);
						}
						DeleteMsgPort(safeport);
					}
					FreeDBufInfo(dbi);
				}
				FreeBitMap(bm[1]);
			}
			FreeBitMap(bm[0]);
		}
		FreeColorMap(cm);
	}
	return(FALSE);
}

void freeView(void)
{
	LoadView(oldview);
	WaitTOF();
	WaitTOF();

	FreeCprList(view.LOFCprList);
	if (view.SHFCprList)
		FreeCprList(view.SHFCprList);

	FreeVPortCopLists(&vp);

	RemIntServer(INTB_COPER, &is);
	FreeSignal(cop.signal);

	if (!safeToDraw)
		while (!GetMsg(safeport))
			WaitPort(safeport);

	DeleteMsgPort(safeport);
	FreeDBufInfo(dbi);
	FreeBitMap(bm[1]);
	FreeBitMap(bm[0]);
	FreeColorMap(cm);
}

int main(void)
{
	if (openInput())
	{
		if (initView())
		{
			WORD i;

			for (i = 0; i < 100; i++)
			{
				SetSignal(0L, 1L << cop.signal);
				ULONG result = Wait((1L << cop.signal) | SIGBREAKF_CTRL_C);

				if (result & SIGBREAKF_CTRL_C)
					break;
			}
			freeView();
		}
		closeInput();
	}
	return(0);
}
