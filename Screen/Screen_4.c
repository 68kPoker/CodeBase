
#include <intuition/screens.h>
#include <exec/memory.h>
#include <graphics/gfxmacros.h>
#include <exec/interrupts.h>
#include <hardware/intbits.h>
#include <hardware/custom.h>

#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/exec_protos.h>

#include "Screen.h"

#include <stdio.h>
#include "debug.h"

__far extern struct Custom custom;

/* Copper server */

void myCopper(void);

/* Alloc two custom bitmaps first */

BOOL allocBitMaps(struct BitMap *bmarray[], UWORD wide, UWORD tall, UBYTE deep, ULONG flags, struct BitMap *friend)
{
	struct BitMap *bm;

	flags |= BMF_DISPLAYABLE; /* These are displayed bitmaps */

	if (bmarray[0] = bm = AllocBitMap(wide, tall, deep, flags, friend))
	{
		if (bmarray[1] = bm = AllocBitMap(wide, tall, deep, flags, friend))
		{
			return(TRUE);
		}
		FreeBitMap(bmarray[0]);
	}
	return(FALSE);
}

/* Open screen */

struct Screen *openScreen(struct Rectangle *dclip, struct BitMap *bmarray[], ULONG modeID, STRPTR title, struct TextAttr *ta, ULONG *colors)
{
	struct Screen *s;

	if (s = OpenScreenTags(NULL,
		SA_DClip,		dclip,
		SA_BitMap,		bmarray[0],
		SA_DisplayID,	modeID,
		SA_Title,		title,
		SA_ShowTitle,	FALSE,
		SA_Quiet,		TRUE,
		SA_Exclusive,	TRUE,
		SA_Font,		ta,
		SA_Colors32,	colors,
		SA_BackFill,	LAYERS_NOBACKFILL,
		TAG_DONE))
	{
		return(s);
	}
	return(NULL);
}

/* Alloc DBufInfo */

struct DBufInfo *allocDBufInfo(struct ViewPort *vp)
{
	struct DBufInfo *dbi;
	struct MsgPort *safemp;

	if (dbi = AllocDBufInfo(vp))
	{
		if (safemp = CreateMsgPort())
		{
			dbi->dbi_SafeMessage.mn_ReplyPort = safemp;
			dbi->dbi_UserData1 = (APTR)TRUE; /* Safe to draw */
			dbi->dbi_UserData2 = (APTR)1; /* Frame */
			return(dbi);
		}
		FreeDBufInfo(dbi);
	}
	return(NULL);
}

void freeDBufInfo(struct DBufInfo *dbi)
{
	struct MsgPort *safemp = dbi->dbi_SafeMessage.mn_ReplyPort;
	BOOL safe = (BOOL)dbi->dbi_UserData1;

	if (!safe)
	{
		while (!GetMsg(safemp))
		{
			WaitPort(safemp);
		}
	}

	DeleteMsgPort(safemp);
	FreeDBufInfo(dbi);
}

/* Add UCL */

BOOL addUCL(struct ViewPort *vp)
{
	struct UCopList *ucl;

	if (ucl = AllocMem(sizeof(*ucl), MEMF_PUBLIC|MEMF_CLEAR))
	{
		CINIT(ucl, 3);
		CWAIT(ucl, 0, 0);
		CMOVE(ucl, custom.intreq, INTF_SETCLR|INTF_COPER);
		CEND(ucl);

		Forbid();
		vp->UCopIns = ucl;
		Permit();

		/* RethinkDisplay() here */

		return(TRUE);
	}
	return(FALSE);
}

/* Add Copper interrupt */

BOOL addCopper(struct Interrupt *is, struct ViewPort *vp)
{
	struct copperData *cd;

	if (cd = AllocMem(sizeof(*cd), MEMF_PUBLIC|MEMF_CLEAR))
	{
		cd->vp = vp;
		cd->task = FindTask(NULL);
		if ((cd->signal = AllocSignal(-1)) != -1)
		{
			is->is_Code = myCopper;
			is->is_Data = (APTR)cd;
			is->is_Node.ln_Pri = 0;

			AddIntServer(INTB_COPER, is);

			return(TRUE);
		}
		FreeMem(cd, sizeof(*cd));
	}
	return(FALSE);
}

/* Remove Copper */

void remCopper(struct Interrupt *is)
{
	RemIntServer(INTB_COPER, is);
	FreeSignal(((struct copperData *)is->is_Data)->signal);
	FreeMem(is->is_Data, sizeof(struct copperData));
}

/* Summing up */

BOOL setupScreen(struct screenData *sd)
{
	struct Rectangle dclip = { 0, 0, 319, 255 };

	if (allocBitMaps(sd->bm, 320, 256, 6, BMF_CLEAR, NULL))
	{
		if (sd->s = openScreen(&dclip, sd->bm, LORES_KEY|EXTRAHALFBRITE_KEY, "Magazyn", NULL, NULL))
		{
			struct ViewPort *vp = &sd->s->ViewPort;

			sd->s->UserData = (APTR)sd;

			if (sd->dbi = allocDBufInfo(vp))
			{
				if (addUCL(vp))
				{
					RethinkDisplay();
					if (addCopper(&sd->is, vp))
					{
						return(TRUE);
					}
				}
				freeDBufInfo(sd->dbi);
			}
			CloseScreen(sd->s);
		}
		FreeBitMap(sd->bm[1]);
		FreeBitMap(sd->bm[0]);
	}
	return(FALSE);
}

void cleanupScreen(struct screenData *sd)
{
	remCopper(&sd->is);
	freeDBufInfo(sd->dbi);
	CloseScreen(sd->s);
	FreeBitMap(sd->bm[1]);
	FreeBitMap(sd->bm[0]);
}
