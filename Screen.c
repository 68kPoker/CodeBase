
/*
** Magazyn (Warehouse)
** Screen.c
*/

#include <intuition/screens.h>
#include <exec/interrupts.h>
#include <exec/memory.h>
#include <graphics/gfxmacros.h>
#include <hardware/intbits.h>
#include <hardware/custom.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>

#include "Screen.h"

__far extern struct Custom custom;

extern VOID myCopper(VOID);

static struct Interrupt is;
static struct copperInfo ci;

static struct Rectangle dclip = { 0, 0, 319, 255 };

/*
** openScreen opens double-buffered screen.
*/

struct Screen *openScreen(struct screenInfo *si)
{
	if (si->bm[0] = AllocBitMap(320, 256, 5, BMF_DISPLAYABLE|BMF_INTERLEAVED|BMF_CLEAR, NULL))
	{
		if (si->bm[1] = AllocBitMap(320, 256, 5, BMF_DISPLAYABLE|BMF_INTERLEAVED|BMF_CLEAR, NULL))
		{
			if (si->s = OpenScreenTags(NULL,
				SA_DClip,		&dclip,
				SA_BitMap,		si->bm[0],
				SA_Title,		"Warehouse",
				SA_Quiet,		TRUE,
				SA_Exclusive,	TRUE,
				SA_ShowTitle,	FALSE,
				SA_BackFill,	LAYERS_NOBACKFILL,
				SA_DisplayID,	LORES_KEY,
				TAG_DONE))
			{
				struct ViewPort *vp = &si->s->ViewPort;
				
				if (si->dbi = AllocDBufInfo(vp))
				{
					if (si->safeport = CreateMsgPort())
					{
						si->dbi->dbi_SafeMessage.mn_ReplyPort = si->safeport;
						si->safe = TRUE;
						si->frame = 1;
						
						if ((ci.signal = si->signal = AllocSignal(-1)) != -1)
						{
							struct UCopList *ucl;
						
							ci.task = FindTask(NULL);
							ci.vp = vp;
							
							is.is_Code = myCopper;
							is.is_Data = (APTR)&ci;
							is.is_Node.ln_Pri = 0;
							is.is_Node.ln_Name = "Warehouse";
							
							if (ucl = AllocMem(sizeof(*ucl), MEMF_PUBLIC|MEMF_CLEAR))
							{
								CINIT(ucl, 3);
								CWAIT(ucl, 0, 0);
								CMOVE(ucl, custom.intreq, INTF_SETCLR|INTF_COPER);
								CEND(ucl);
								
								Forbid();
								vp->UCopIns = ucl;
								Permit();
								
								RethinkDisplay();
								
								AddIntServer(INTB_COPER, &is);
								
								si->s->UserData = (APTR)si;
								return(si->s);
							}	
							FreeSignal(ci.signal);
						}	
						DeleteMsgPort(si->safeport);
					}
					FreeDBufInfo(si->dbi);
				}		
				CloseScreen(si->s);
			}	
			FreeBitMap(si->bm[1]);
		}
		FreeBitMap(si->bm[0]);	
	}
	return(NULL);
}

VOID closeScreen(struct screenInfo *si)
{
	RemIntServer(INTB_COPER, &is);
	
	FreeSignal(ci.signal);
	
	if (!si->safe)
	{
		while (!GetMsg(si->safeport))
		{
			WaitPort(si->safeport);
		}		
	}	
	DeleteMsgPort(si->safeport);
	FreeDBufInfo(si->dbi);
	CloseScreen(si->s);
	FreeBitMap(si->bm[1]);
	FreeBitMap(si->bm[0]);
}
