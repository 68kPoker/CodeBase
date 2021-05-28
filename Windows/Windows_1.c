
/*
** Magazyn (Warehouse)
** Windows.c
*/

#include <intuition/intuition.h>

#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>

#include "Screen.h"
#include "Windows.h"

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
		WA_IDCMP,			IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE,
		WA_ReportMouse,		TRUE,
		TAG_DONE))
	{
		return(w);
	}
	return(NULL);	
}

LONG mainLoop(struct Window *w, struct screenInfo *si, struct BitMap *gfx)
{	
	BOOL done = FALSE;
	ULONG signals[] = 
	{ 
		1L << w->UserPort->mp_SigBit,
		1L << si->signal,
		1L << si->safeport->mp_SigBit
	}, total = signals[0] | signals[1] | signals[2];
	BOOL drawn[2] = { FALSE };

	while (!done)
	{
		ULONG result = Wait(total);
		
		if (result & signals[SIGBIT_USERPORT])
		{
			struct IntuiMessage *msg;
			/* User Port */
			while (msg = (struct IntuiMessage *)GetMsg(w->UserPort))
			{	
				ULONG class = msg->Class;
				WORD code = msg->Code;
				WORD mx = msg->MouseX, my = msg->MouseY;
				APTR iaddr = msg->IAddress;
				
				ReplyMsg((struct Message *)msg);
				
				if (class == IDCMP_RAWKEY)
				{
					if (code == ESC_KEY)
					{
						done = TRUE;
						break;
					}	
				}	
			}
			if (done)
				break;
		}
		
		if (result & signals[SIGBIT_COPPER])
		{
			/* Copper interrupt */
			UWORD frame = si->frame;
			
			if (si->safe)
			{
				WaitBlit();
				ChangeVPBitMap(&si->s->ViewPort, si->bm[frame], si->dbi);
				si->safe = FALSE;
				si->frame ^= 1;
			}	
		}
		
		if (result & signals[SIGBIT_SAFE])
		{
			/* SafeToDraw */
			struct RastPort *rp = w->RPort;
			UWORD frame = si->frame;
			
			if (!si->safe)
			{
				while (!GetMsg(si->safeport))
				{
					WaitPort(si->safeport);
				}	
				si->safe = TRUE;
			}	
			rp->BitMap = si->bm[frame];			
			
			if (!drawn[frame])
			{
				BltBitMapRastPort(gfx, 0, 0, w->RPort, 0, 0, 320, 16, 0xc0);
				drawn[frame] = TRUE;
			}	
			
		}
	}	
}
