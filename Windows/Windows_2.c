
#include <intuition/intuition.h>
#include <clib/intuition_protos.h>
#include <clib/layers_protos.h>
#include <clib/graphics_protos.h>
#include <clib/alib_protos.h>
#include <clib/exec_protos.h>

#include "Screen.h"
#include "Windows.h"

/* Open fullscreen window */

struct Window *openWindow(struct Screen *s, ULONG idcmp)
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
		WA_IDCMP,			idcmp,
		WA_ReportMouse,		TRUE,
		TAG_DONE))
	{
		return(w);
	}
	return(NULL);
}

void simpleUpdate(struct windowData *wd, BOOL update, struct Region *refresh)
{
	struct Region *prev;
	static WORD counter = 0;
	UBYTE text[5];
	struct screenData *sd = (struct screenData *)wd->w->WScreen->UserData;

	WORD frame = (WORD)sd->dbi->dbi_UserData2;

	wd->w->RPort->BitMap = sd->bm[frame];

	if (update)
	{
		Move(wd->w->RPort, wd->bounds[frame].MinX + 1, wd->bounds[frame].MinY + 1 + wd->w->RPort->Font->tf_Baseline);
		SetABPenDrMd(wd->w->RPort, 2, 3, JAM2);

		sprintf(text, "%4d", counter);

		Text(wd->w->RPort, text, 4);
		counter++;

		return;
	}

	prev = InstallClipRegion(wd->w->WLayer, refresh);

	SetAPen(wd->w->RPort, 3);

	RectFill(wd->w->RPort, wd->bounds[frame].MinX, wd->bounds[frame].MinY, wd->bounds[frame].MaxX, wd->bounds[frame].MaxY);

	InstallClipRegion(wd->w->WLayer, prev);
}

void inactiveUpdate(struct windowData *wd, BOOL update, struct Region *refresh)
{
	struct Region *prev;
	static WORD counter = 0;
	UBYTE text[5];
	struct screenData *sd = (struct screenData *)wd->w->WScreen->UserData;

	WORD frame = (WORD)sd->dbi->dbi_UserData2;

	wd->w->RPort->BitMap = sd->bm[frame];

	if (update)
	{
		Move(wd->w->RPort, wd->bounds[frame].MinX + 1, wd->bounds[frame].MinY + 1 + wd->w->RPort->Font->tf_Baseline);
		SetABPenDrMd(wd->w->RPort, 2, 3, JAM2);

		sprintf(text, "%4d", counter);

		Text(wd->w->RPort, text, 4);
		counter++;

		return;
	}

	prev = InstallClipRegion(wd->w->WLayer, refresh);

	SetAPen(wd->w->RPort, 32);
	SetWriteMask(wd->w->RPort, 0x20);

	RectFill(wd->w->RPort, wd->bounds[frame].MinX, wd->bounds[frame].MinY, wd->bounds[frame].MaxX, wd->bounds[frame].MaxY);

	SetWriteMask(wd->w->RPort, 0xdf);

	InstallClipRegion(wd->w->WLayer, prev);
}


/* Prepare Region */

BOOL prepRegion(struct List *list, struct windowData *wd, struct Window *w, WORD left, WORD top, WORD width, WORD height, void (*update)(struct windowData *wd, BOOL update, struct Region *refresh))
{
	struct Region *reg;
	struct windowData *back;

	wd->w = w;

	wd->bounds[0].MinX = left;
	wd->bounds[0].MaxX = left + width - 1;
	wd->bounds[0].MinY = top;
	wd->bounds[0].MaxY = top + height - 1;

	if (wd->reg = reg = NewRegion())
	{
		OrRectRegion(reg, &wd->bounds[0]);
		wd->bounds[1] = wd->bounds[0];

		wd->update = update;

		/* Hide back layers */

		for (back = (struct windowData *)list->lh_TailPred; back->node.ln_Pred != NULL; back = (struct windowData *)back->node.ln_Pred)
		{
			ClearRectRegion(back->reg, &wd->bounds[0]);
		}

		AddTail(list, &wd->node);

		return(TRUE);
	}
	return(FALSE);
}

void freeRegion(struct windowData *wd)
{
	Remove(&wd->node);
	DisposeRegion(wd->reg);
}

BOOL loop(struct Window *w, struct List *list)
{
	BOOL done = FALSE;
	struct Screen *s = w->WScreen;
	struct screenData *sd = (struct screenData *)s->UserData;
	struct windowData *wd = (struct windowData *)list->lh_Head;
	struct copperData *cd = (struct copperData *)sd->is.is_Data;
	ULONG signals[] =
	{
		1L << w->UserPort->mp_SigBit,
		1L << sd->dbi->dbi_SafeMessage.mn_ReplyPort->mp_SigBit,
		1L << cd->signal
	};

	while (!done)
	{
		ULONG total = signals[0]|signals[1]|signals[2];
		ULONG result = Wait(total);

		if (result & signals[0])
		{
			struct IntuiMessage *msg;

			while (msg = (struct IntuiMessage *)GetMsg(w->UserPort))
			{
				ULONG class = msg->Class;
				UWORD code = msg->Code;
				WORD mx = msg->MouseX;
				WORD my = msg->MouseY;

				ReplyMsg((struct Message *)msg);

				if (class == IDCMP_RAWKEY)
				{
					if (code == 0x45)
					{
						done = TRUE;
					}
				}
			}
		}
		if (result & signals[1])
		{
			BOOL safe = (BOOL)sd->dbi->dbi_UserData1;
			WORD frame = (WORD)sd->dbi->dbi_UserData2;

			if (!safe)
			{
				while (!GetMsg(sd->dbi->dbi_SafeMessage.mn_ReplyPort))
				{
					WaitPort(sd->dbi->dbi_SafeMessage.mn_ReplyPort);
				}
				safe = TRUE;
			}

			/* Draw here */
			for (wd = (struct windowData *)list->lh_Head; wd->node.ln_Succ != NULL; wd = (struct windowData *)wd->node.ln_Succ)
			{
				wd->update(wd, wd->drawn >= 2, wd->reg);

				if (wd->drawn < 2)
				{
					wd->drawn++;
				}
			}

			sd->dbi->dbi_UserData1 = (APTR)safe;
		}
		if (result & signals[2])
		{
			BOOL safe = (BOOL)sd->dbi->dbi_UserData1;
			WORD frame = (WORD)sd->dbi->dbi_UserData2;

			if (safe)
			{
				ChangeVPBitMap(&s->ViewPort, sd->bm[frame], sd->dbi);

				safe = FALSE;
				frame ^= 1;

				sd->dbi->dbi_UserData1 = (APTR)safe;
				sd->dbi->dbi_UserData2 = (APTR)frame;
			}
		}
	}
}

int main(void)
{
	struct screenData sd = { 0 };
	struct Window *w;
	struct windowData bd = { 0 }, wd = { 0 };
	struct List list;

	if (setupScreen(&sd))
	{
		if (w = openWindow(sd.s, IDCMP_RAWKEY))
		{
			NewList(&list);
			if (prepRegion(&list, &bd, w, 0, 0, sd.s->Width, sd.s->Height, inactiveUpdate))
			{
				if (prepRegion(&list, &wd, w, 16, 16, 160, 160, simpleUpdate))
				{
					loop(w, &list);
					freeRegion(&wd);
				}
				freeRegion(&bd);
			}
			CloseWindow(w);
		}
		cleanupScreen(&sd);
	}
	return(0);
}
