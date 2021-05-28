
#include "Windows.h"

#include <intuition/intuition.h>
#include <graphics/gfxmacros.h>
#include <hardware/intbits.h>
#include <hardware/custom.h>
#include <exec/memory.h>

#include <clib/intuition_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>

extern STRPTR TITLE;

extern void myCopper(void);

__far extern struct Custom custom;

struct Window *openMainWindow(struct Screen *s, struct windowUD *wud)
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
		WA_IDCMP,			IDCMP_RAWKEY|IDCMP_MOUSEMOVE|IDCMP_MOUSEBUTTONS|IDCMP_REFRESHWINDOW,
		WA_ReportMouse,		TRUE,
		WA_SimpleRefresh,	TRUE,
		WA_BackFill,		LAYERS_NOBACKFILL,
		TAG_DONE))
	{
		w->UserData = (APTR)wud;
		return(w);
	}
	return(NULL);
}

struct Screen *openScreen(struct screenUD *sud)
{
	struct Screen *s;
	struct Rectangle dclip = { 0, 0, 319, 255 };

	if (s = OpenScreenTags(NULL,
		SA_DClip,		&dclip,
		SA_Depth,		DEPTH,
		SA_DisplayID,	MODEID,
		SA_Quiet,		TRUE,
		SA_Exclusive,	TRUE,
		SA_ShowTitle,	FALSE,
		SA_BackFill,	LAYERS_NOBACKFILL,
		SA_Title,		TITLE,
		SA_Interleaved,	TRUE,
		TAG_DONE))
	{
		struct Interrupt *is = &sud->is;
		struct copperInfo *ci = &sud->ci;

		is->is_Code = myCopper;
		is->is_Data = ci;
		is->is_Node.ln_Pri = 0;
		is->is_Node.ln_Name = TITLE;

		if ((ci->signal = AllocSignal(-1)) != -1)
		{
			struct UCopList *ucl;

			ci->vp = &s->ViewPort;
			ci->task = FindTask(NULL);

			if (ucl = AllocMem(sizeof(*ucl), MEMF_PUBLIC|MEMF_CLEAR))
			{
				CINIT(ucl, 3);
				CWAIT(ucl, 0, 0);
				CMOVE(ucl, custom.intreq, INTF_SETCLR|INTF_COPER);
				CEND(ucl);

				Forbid();
				ci->vp->UCopIns = ucl;
				Permit();

				RethinkDisplay();

				AddIntServer(INTB_COPER, is);

				s->UserData = (APTR)sud;
				return(s);
			}
			FreeSignal(ci->signal);
		}
		CloseScreen(s);
	}
	return(NULL);
}

void closeScreen(struct Screen *s)
{
	struct screenUD *sud = (struct screenUD *)s->UserData;

	RemIntServer(INTB_COPER, &sud->is);

	FreeSignal(sud->ci.signal);
	CloseScreen(s);
}

LONG mainLoop(struct Window *w)
{
	struct windowUD *wud = (struct windowUD *)w->UserData;
	struct screenUD *sud = (struct screenUD *)w->WScreen->UserData;
	BOOL done = FALSE;
	struct MsgPort *userPort = w->UserPort;
	ULONG signals[MTYPES] =
	{
		1L << userPort->mp_SigBit,
		1L << sud->ci.signal
	};
	union message m;

	while (!done)
	{
		ULONG total = signals[MTYPE_IDCMP]|signals[MTYPE_VBLANK];
		ULONG result = Wait(total);

		if (result & signals[MTYPE_IDCMP])
		{
			struct IntuiMessage *msg;

			if (msg = (struct IntuiMessage *)GetMsg(userPort))
			{
				m.msg = msg;
				done = wud->dispatch(w, MTYPE_IDCMP, &m);

				ReplyMsg((struct Message *)msg);
			}
		}

		if (result & signals[MTYPE_VBLANK])
		{
			struct RastPort *rp = w->RPort;
			m.rp = rp;
			wud->dispatch(w, MTYPE_VBLANK, &m);
		}
	}
}
