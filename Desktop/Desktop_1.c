
/* Mój pulpit - ekran publiczny DESKTOP.1 */
/* Moje gry i programy sâ na nim otwierane */
/* Inne program teû mogâ */

#include <dos/dos.h>
#include <graphics/displayinfo.h>
#include <exec/memory.h>
#include <exec/lists.h>
#include <intuition/intuition.h>
#include <libraries/gadtools.h>
#include <intuition/gadgetclass.h>

#include <clib/diskfont_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/alib_protos.h>
#include <clib/exec_protos.h>

#define DESC_SIZE 32

struct myNode
{
	struct Node node;
	ULONG modeID;
	WORD width, height, depth;
};

struct TextAttr ta =
{
	"centurion.font",
	9,
	FS_NORMAL,
	FPF_DISKFONT|FPF_DESIGNED
}, topaz =
{
	"topazpl.font",
	8,
	FS_NORMAL,
	FPF_DISKFONT|FPF_DESIGNED
};

struct ColorSpec colors[] =
{
	{ 0, 0, 0, 0 },
	{ 1, 0, 0, 0 },
	{ 2, 15, 15, 15 },
	{ 3, 5, 5, 15 },
	{ -1 }
};

/* Najpierw przeszukujë listë dostëpnych trybów ekranu */

scanDisplayInfo(struct List *list)
{
	ULONG modeID = INVALID_ID;
	WORD i = 1;

	while ((modeID = NextDisplayInfo(modeID)) != INVALID_ID)
	{
		DisplayInfoHandle dih;

		if (dih = FindDisplayInfo(modeID))
		{
			struct DimensionInfo dim;

			if (GetDisplayInfoData(dih, (UBYTE *)&dim, sizeof(dim), DTAG_DIMS, 0) > 0)
			{
				WORD width = dim.Nominal.MaxX - dim.Nominal.MinX + 1;
				WORD height = dim.Nominal.MaxY - dim.Nominal.MinY + 1;
				struct DisplayInfo disp;

				if (GetDisplayInfoData(dih, (UBYTE *)&disp, sizeof(disp), DTAG_DISP, 0) > 0)
				{
					ULONG prop = disp.PropertyFlags;

					if ((dim.MaxDepth >= 5) && (width == 320 || width == 640) && (height == 256 || height == 512) && (!(prop & (DIPF_IS_HAM|DIPF_IS_DUALPF))) && ((modeID & MONITOR_ID_MASK) != DEFAULT_MONITOR_ID))
					{
						struct myNode *node;

						if (node = AllocMem(sizeof(*node), MEMF_PUBLIC|MEMF_CLEAR))
						{
							if (node->node.ln_Name = AllocMem(DESC_SIZE, MEMF_PUBLIC|MEMF_CLEAR))
							{
								node->width = width;
								node->height = height;
								node->depth = dim.MaxDepth;
								node->modeID = modeID;

								sprintf(node->node.ln_Name, "%3d x %3d (%d)%s%s", width, height, 1 << dim.MaxDepth, prop & DIPF_IS_AA ? " AA" : "", prop & DIPF_IS_EXTRAHALFBRITE ? " EHB" : "");

								AddTail(list, &node->node);
							}
						}
						/*

						printf("$%05X: ", modeID);
						printf("%3d x %3d (%d)%s%s", width, height, 1 << dim.MaxDepth, prop & DIPF_IS_AA ? " AA" : "", prop & DIPF_IS_EHB ? " EHB" : "");
						if (prop & DIPF_IS_PAL)
							printf("PAL ");
						if (prop & DIPF_IS_ECS)
							printf("ECS ");
						if (prop & DIPF_IS_AA)
							printf("AA ");
						if (prop & DIPF_IS_EXTRAHALFBRITE)
							printf("EHB ");
						if (prop & DIPF_IS_LACE)
							printf("Lace ");
						putchar('\n');
						*/
					}
				}
			}
		}
		i++;
	}
}

void freeNodes(struct List *list)
{
	struct Node *node, *next;

	node = list->lh_Head;
	next = node->ln_Succ;

	while (next)
	{
		FreeMem(node->ln_Name, DESC_SIZE);
		FreeMem(node, sizeof(struct myNode));
		node = next;
		next = next->ln_Succ;
	}
}

/* Otwieram okno z wyborem trybu */

struct Window *openWindow()
{
	struct Window *w;

	if (w = OpenWindowTags(NULL,
		WA_Width,		320,
		WA_Height,		200,
		WA_Title,		"Manedûer blatu",
		WA_ScreenTitle,	"Menedûer blatu (C)2021 Robert Szacki",
		WA_CloseGadget,	TRUE,
		WA_DepthGadget,	TRUE,
		WA_SimpleRefresh,	TRUE,
		WA_DragBar,		TRUE,
		WA_Activate,	TRUE,
		WA_IDCMP,		BUTTONIDCMP|SLIDERIDCMP|LISTVIEWIDCMP|IDCMP_CLOSEWINDOW|IDCMP_REFRESHWINDOW,
		TAG_DONE))
	{
		return(w);
	}
	return(NULL);
}

LONG getColors(struct Gadget *gad, WORD level)
{
	return(1 << level);
}

ULONG getModeID(struct Window *w, struct List *list, ULONG *depth)
{
	struct VisualInfo *vi;
	ULONG modeID = INVALID_ID;

	if (vi = GetVisualInfoA(w->WScreen, NULL))
	{
		struct NewGadget ng;
		struct Gadget *prev, *glist, *gads[2];

		ng.ng_LeftEdge = w->BorderLeft + 32;
		ng.ng_TopEdge = w->BorderTop + 16;
		ng.ng_Width = 240;
		ng.ng_Height = 120;
		ng.ng_VisualInfo = vi;
		ng.ng_TextAttr = &topaz;
		ng.ng_GadgetText = "Wybierz tryb ekranu";
		ng.ng_UserData = NULL;
		ng.ng_GadgetID = 1;
		ng.ng_Flags = PLACETEXT_ABOVE;

		if (prev = CreateContext(&glist))
		{
			if (gads[0] = prev = CreateGadget(LISTVIEW_KIND, prev, &ng,
				GTLV_Labels,	list,
				GTLV_ShowSelected,	0,
				GTLV_Selected, 0,
				TAG_DONE))
			{
				ng.ng_TopEdge += ng.ng_Height + 4;
				ng.ng_Width = 160;
				ng.ng_Height = 14;
				ng.ng_GadgetText = "";
				ng.ng_GadgetID = 2;
				ng.ng_Flags = PLACETEXT_RIGHT;

				gads[1] = prev = CreateGadget(SLIDER_KIND, prev, &ng,
					GTSL_Min, 5,
					GTSL_Max, 8,
					GTSL_Level, 5,
					GA_Immediate, TRUE,
					GTSL_MaxLevelLen, 12,
					GTSL_LevelFormat, "Gîëbia: %ld",
					GTSL_LevelPlace, PLACETEXT_RIGHT,
					GTSL_DispFunc, getColors,
					TAG_DONE);

				ng.ng_TopEdge += ng.ng_Height + 4;
				ng.ng_Width = 240;
				ng.ng_GadgetText = "Otwórz ekran";
				ng.ng_GadgetID = 3;
				ng.ng_Flags = PLACETEXT_IN;

				prev = CreateGadget(BUTTON_KIND, prev, &ng,
					TAG_DONE);

				BOOL done = FALSE;

				AddGList(w, glist, -1, -1, NULL);
				RefreshGList(glist, w, NULL, -1);
				GT_RefreshWindow(w, NULL);


				while (!done)
				{
					struct IntuiMessage *msg;
					WaitPort(w->UserPort);
					while ((!done) && (msg = GT_GetIMsg(w->UserPort)))
					{
						if (msg->Class == IDCMP_CLOSEWINDOW)
						{
							done = TRUE;
							modeID = INVALID_ID;
						}
						else if (msg->Class == IDCMP_REFRESHWINDOW)
						{
							GT_BeginRefresh(w);
							GT_EndRefresh(w, TRUE);
						}
						else if (msg->Class == IDCMP_GADGETUP)
						{
							struct Gadget *gad = (struct Gadget *)msg->IAddress;

							if (gad->GadgetID == 3)
							{
								WORD i;
								LONG selected;
								struct myNode *node = (struct myNode *)list->lh_Head;

								GT_GetGadgetAttrs(gads[0], w, NULL,
									GTLV_Selected, &selected,
									TAG_DONE);

								GT_GetGadgetAttrs(gads[1], w, NULL,
									GTSL_Level,	depth,
									TAG_DONE);

								for (i = 0; i < selected; i++)
								{
									node = (struct myNode *)node->node.ln_Succ;
								}

								modeID = node->modeID;
								done = TRUE;
							}
						}
						GT_ReplyIMsg(msg);
					}
				}
			}
			RemoveGList(w, glist, -1);
			FreeGadgets(glist);
		}
		FreeVisualInfo(vi);
	}
	return(modeID);
}

int main()
{
	struct List list;
	struct Window *w;
	struct TextFont *tf;

	NewList(&list);

	scanDisplayInfo(&list);

	if (tf = OpenDiskFont(&ta))
	{
		if (w = openWindow())
		{
			ULONG modeID;
			LONG depth = 0;

			modeID = getModeID(w, &list, &depth);

			CloseWindow(w);

			if (modeID != INVALID_ID)
			{
				struct Screen *s;
				printf("Selected: $%x, %ld\n", modeID, depth);
				UWORD pens[] = { ~0 };
				WORD signal;

				if (modeID & EXTRAHALFBRITE_KEY)
				{
					depth = 6;
				}

				if ((signal = AllocSignal(-1)) != -1)
				{
					if (s = OpenScreenTags(NULL,
						SA_Width,	STDSCREENWIDTH,
						SA_Height,	STDSCREENHEIGHT,
						SA_Depth,	depth,
						SA_DisplayID,	modeID,
						SA_Title,	"GearWorks Software Desktop",
						SA_PubName,	"DESKTOP.1",
						SA_Pens,	pens,
						SA_SharePens,	TRUE,
						SA_Font,	&ta,
						SA_Colors,	colors,
						SA_Quiet,	TRUE,
						SA_ShowTitle,	FALSE,
						SA_Draggable,	FALSE,
						SA_PubTask,	FindTask(NULL),
						SA_PubSig,	signal,
						SA_Interleaved, TRUE,
						TAG_DONE))
					{
						struct RastPort *rp = &s->RastPort;
						Move(rp, 0, tf->tf_Baseline);
						SetAPen(rp, 3);
						Text(rp, "DESKTOP.1", 9);

						PubScreenStatus(s, 0);

						Wait(SIGBREAKF_CTRL_C|(1L << signal));
						CloseScreen(s);
					}
					FreeSignal(signal);
				}
			}
		}
		CloseFont(tf);
	}

	freeNodes(&list);
	return 0;
}
