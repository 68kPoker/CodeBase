
#include <exec/memory.h>
#include <intuition/screens.h>
#include <hardware/intbits.h>
#include <exec/interrupts.h>
#include <graphics/gfxmacros.h>
#include <hardware/custom.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/utility_protos.h>
#include <clib/graphics_protos.h>

#include "Screen.h"

__far extern struct Custom custom;

extern void myCopper(void);

static struct Interrupt is;
static struct copperData
{
	struct ViewPort *vp;
	WORD signal;
	struct Task *task;
} cd;

struct Screen *openScreen(ULONG tag1, ...)
{
	struct Rectangle dclip =
	{
		0, 		0,
		319, 	255
	};
	struct TagItem tags[] =
	{
		SA_DClip,		(ULONG)&dclip,
		SA_Depth,		5,
		SA_BitMap,		NULL,
		SA_DisplayID,	LORES_KEY,
		SA_Quiet,		TRUE,
		SA_ShowTitle,	FALSE,
		SA_Exclusive,	TRUE,
		SA_BackFill,	(ULONG)LAYERS_NOBACKFILL,
		SA_Title,		(ULONG)"C-1200 Screen",
		TAG_DONE
	};

	ApplyTagChanges(tags, (struct TagItem *)&tag1);

	return(OpenScreenTagList(NULL, tags));
}

WORD addCopper(struct ViewPort *vp)
{
	is.is_Code = myCopper;
	is.is_Data = (APTR)&cd;
	is.is_Node.ln_Pri = 0;
	is.is_Node.ln_Name = "C-1200";

	if ((cd.signal = AllocSignal(-1)) != -1)
	{
		cd.task = FindTask(NULL);
		cd.vp = vp;

		AddIntServer(INTB_COPER, &is);
		return(cd.signal);
	}
	return(-1);
}

void remCopper(void)
{
	RemIntServer(INTB_COPER, &is);
	FreeSignal(cd.signal);
}

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

		RethinkDisplay();
		return(TRUE);
	}
	return(FALSE);
}

struct DBufInfo *getDBufInfo(struct ViewPort *vp, BOOL *safe)
{
	struct DBufInfo *dbi;

	if (dbi = AllocDBufInfo(vp))
	{
		if (dbi->dbi_SafeMessage.mn_ReplyPort = CreateMsgPort())
		{
			*safe = TRUE;
			return(dbi);
		}
		FreeDBufInfo(dbi);
	}
	return(NULL);
}

void freeDBufInfo(struct DBufInfo *dbi, BOOL safe)
{
	safeToDraw(dbi, &safe);
	DeleteMsgPort(dbi->dbi_SafeMessage.mn_ReplyPort);
	FreeDBufInfo(dbi);
}

void safeToDraw(struct DBufInfo *dbi, BOOL *safe)
{
	struct MsgPort *mp = dbi->dbi_SafeMessage.mn_ReplyPort;
	if (!*safe)
	{
		while (!GetMsg(mp))
		{
			WaitPort(mp);
		}
		*safe = TRUE;
	}
}

void changeVPBitMap(struct ViewPort *vp, struct DBufInfo *dbi, struct BitMap *bm, BOOL *safe)
{
	if (*safe)
	{
		ChangeVPBitMap(vp, bm, dbi);
		*safe = FALSE;
	}
}

struct Screen *openDBufScreen(struct BitMap **bm, WORD *signal, struct DBufInfo **dbi, BOOL *safe)
{
	if (bm[0] = AllocBitMap(320, 256, 5, BMF_DISPLAYABLE|BMF_INTERLEAVED|BMF_CLEAR, NULL))
	{
		if (bm[1] = AllocBitMap(320, 256, 5, BMF_DISPLAYABLE|BMF_INTERLEAVED|BMF_CLEAR, NULL))
		{
			struct Screen *s;

			if (s = openScreen(SA_BitMap, bm[0], TAG_DONE))
			{
				struct ViewPort *vp = &s->ViewPort;

				if ((*signal = addCopper(vp)) != -1)
				{
					if (addUCL(vp))
					{
						if (*dbi = getDBufInfo(vp, safe))
						{
							return(s);
						}
					}
					remCopper();
				}
				CloseScreen(s);
			}
			FreeBitMap(bm[1]);
		}
		FreeBitMap(bm[0]);
	}
	return(NULL);
}

void closeDBufScreen(struct Screen *s, struct BitMap **bm, struct DBufInfo *dbi, BOOL safe)
{
	freeDBufInfo(dbi, safe);
	remCopper();
	CloseScreen(s);
	FreeBitMap(bm[1]);
	FreeBitMap(bm[0]);
}

struct Screen *openScreenInfo(struct screenInfo *si)
{
	return(si->s = openDBufScreen(si->bm, &si->signal, &si->dbi, &si->safe));
}

void closeScreenInfo(struct screenInfo *si)
{
	closeDBufScreen(si->s, si->bm, si->dbi, si->safe);
}
