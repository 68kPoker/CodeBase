
#include "Fields.h"
#include "Board.h"

#include <exec/types.h>
#include <intuition/intuition.h>
#include <libraries/gadtools.h>
#include <intuition/gadgetclass.h>
#include <workbench/icon.h>

#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/exec_protos.h>
#include <clib/icon_protos.h>

#define SELECT_LEFT	  	8
#define SELECT_TOP	  	32
#define SELECT_WIDTH  	96
#define SELECT_HEIGHT 	14
#define SELECT_INTERVAL ((SELECT_HEIGHT * 2) - SELECT_LEFT)

enum
{
	GID_FLOOR,
	GID_FLAGSTONE,
	GID_DIR,
	GID_OBJECT,
	GID_ITEM,
	GID_KEY,
	GID_COUNT
};

enum
{
	BASIC_WALL,
	BASIC_FLOOR,
	BASIC_BOX,
	BASIC_PLACE,
	BASIC_DOOR,
	BASIC_KEY,
	BASIC_FRUITS,
	BASIC_COINS,
	BASIC_SLIDELEFT,
	BASIC_SLIDERIGHT,
	BASIC_SLIDEUP,
	BASIC_SLIDEDOWN,
	BASIC_PLAYER,
	BASIC_SKULL,
	BASIC_TYPES
};

STRPTR floorNames[FLOOR_TYPES + 1] =
{
	"Back",
	"Normal",
	"Wall",
	"Flagstone",
	"Door",
	"Slider",
	NULL
};

STRPTR flagStoneNames[FLAGSTONE_TYPES + 1] =
{
	"Square",
	"Triangle",
	"Circle",
	NULL
};

STRPTR dirNames[] =
{
	"Left",
	"Right",
	"Up",
	"Down",
	NULL
};

STRPTR objectNames[OBJECT_TYPES + 1] =
{
	"None",
	"Box",
	"Item",
	"Player",
	"Skull",
	NULL
};

STRPTR itemNames[ITEM_TYPES + 1] =
{
	"Coins",
	"Fruits",
	"Key",
	NULL
};

STRPTR appearNames[APPEAR_TYPES + 1] =
{
	"At start",
	"Placement",
	NULL
};

STRPTR behaviorNames[BEHAVIOR_TYPES + 1] =
{
	"Normal",
	"Changeable",
	NULL
};

STRPTR actionNames[ACTION_TYPES + 1] =
{
	"None",
	"Show object",
	"Remove wall",
	NULL
};

STRPTR keyNames[KEY_TYPES + 1] =
{
	"Gold",
	"Silver",
	"Bronze",
	NULL
};

STRPTR iconNames[BASIC_TYPES + 1] =
{
	"Icons/Wall",
	"Icons/Floor",
	"Icons/Box",
	"Icons/Place",
	"Icons/Door",
	"Icons/Key",
	"Icons/Fruits",
	"Icons/Coins",
	"Icons/SlideLeft",
	"Icons/SlideRight",
	"Icons/SlideUp",
	"Icons/SlideDown",
	"Icons/Hero",
	"Icons/Skull",
	NULL
};

ULONG myPaletteRGB32[98] =
{
	0x00200000,	/* Record Header */
	0xAAAAAAAA,0xAAAAAAAA,0xAAAAAAAA,
	0x12222222,0x0AAAAAAA,0x00000000,
	0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
	0xFFFFFFFF,0x65555555,0xFFFFFFFF,
	0xEEEEEEEE,0x7EEEEEEE,0x00000000,
	0xE5555555,0x9DDDDDDD,0x4DDDDDDD,
	0xFFFFFFFF,0x4EEEEEEE,0x4EEEEEEE,
	0x01111111,0xCEEEEEEE,0x00000000,
	0xDCCCCCCC,0x00000000,0x00000000,
	0x00000000,0x61111111,0xFFFFFFFF,
	0xFFFFFFFF,0xFFFFFFFF,0x00000000,
	0xC2222222,0xC2222222,0xC2222222,
	0x94444444,0x00000000,0x94444444,
	0x7DDDDDDD,0x7DDDDDDD,0x7DDDDDDD,
	0xA6666666,0x5DDDDDDD,0x28888888,
	0x68888888,0x28888888,0x00000000,
	0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
	0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
	0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
	0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
	0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
	0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
	0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
	0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
	0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
	0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
	0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
	0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
	0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
	0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
	0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
	0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
	0x00000000	/* Terminator */
};

struct tileInfo
{
	WORD floor, flag, dir, object, item, key;
};

struct Gadget *createFieldSelectionGadgets(struct VisualInfo *vi, struct TextAttr *ta, struct Gadget *gads[])
{
	struct Gadget *glist, *prev;
	struct NewGadget ng;

	prev = CreateContext(&glist);

	ng.ng_VisualInfo = vi;
	ng.ng_LeftEdge	 = SELECT_LEFT;
	ng.ng_TopEdge	 = SELECT_TOP;
	ng.ng_Width		 = SELECT_WIDTH;
	ng.ng_Height	 = SELECT_HEIGHT;
	ng.ng_Flags		 = PLACETEXT_RIGHT | NG_HIGHLABEL;
	ng.ng_TextAttr	 = ta;

	ng.ng_GadgetText = "Floor";
	ng.ng_GadgetID	 = GID_FLOOR;
	ng.ng_UserData	 = NULL;

	gads[ng.ng_GadgetID] = prev = CreateGadget(CYCLE_KIND, prev, &ng,
		GTCY_Labels, 	floorNames,
		/* GTMX_TitlePlace, PLACETEXT_ABOVE,
		GTMX_Spacing,	2, */
		TAG_DONE);

	ng.ng_TopEdge	 += SELECT_INTERVAL;

	ng.ng_GadgetText = "Flag";
	ng.ng_GadgetID	 = GID_FLAGSTONE;

	gads[ng.ng_GadgetID] = prev = CreateGadget(CYCLE_KIND, prev, &ng,
		GTCY_Labels, 	flagStoneNames,
		GA_Disabled,	TRUE,
		TAG_DONE);

	ng.ng_TopEdge	 += SELECT_INTERVAL;

	ng.ng_GadgetText = "Direction";
	ng.ng_GadgetID	 = GID_DIR;

	gads[ng.ng_GadgetID] = prev = CreateGadget(CYCLE_KIND, prev, &ng,
		GTCY_Labels, 	dirNames,
		GA_Disabled,	TRUE,
		TAG_DONE);

	ng.ng_TopEdge	 += SELECT_INTERVAL;

	ng.ng_GadgetText = "Object";
	ng.ng_GadgetID	 = GID_OBJECT;

	gads[ng.ng_GadgetID] = prev = CreateGadget(CYCLE_KIND, prev, &ng,
		GTCY_Labels, 	objectNames,
		TAG_DONE);

	ng.ng_TopEdge	 += SELECT_INTERVAL;

	ng.ng_GadgetText = "Item";
	ng.ng_GadgetID	 = GID_ITEM;

	gads[ng.ng_GadgetID] = prev = CreateGadget(CYCLE_KIND, prev, &ng,
		GTCY_Labels, 	itemNames,
		GA_Disabled,	TRUE,
		TAG_DONE);

	ng.ng_TopEdge	 += SELECT_INTERVAL;

	ng.ng_GadgetText = "Key";
	ng.ng_GadgetID	 = GID_KEY;

	gads[ng.ng_GadgetID] = prev = CreateGadget(CYCLE_KIND, prev, &ng,
		GTCY_Labels, 	keyNames,
		GA_Disabled,	TRUE,
		TAG_DONE);

	return(glist);
}

struct Window *createFieldSelectionWindow(struct Screen *s, struct VisualInfo *vi, struct Gadget *glist, struct Gadget *gads)
{
	struct Window *w;

	if (w = OpenWindowTags(NULL,
		WA_CustomScreen,	s,
		WA_Left,			440,
		WA_Top,				s->BarHeight + 1,
		WA_InnerWidth,		640 - 440,
		WA_InnerHeight,		240,
		WA_Gadgets,			glist,
		WA_CloseGadget,		TRUE,
		WA_DepthGadget,		TRUE,
		WA_DragBar,			TRUE,
		WA_IDCMP,			IDCMP_CLOSEWINDOW|CYCLEIDCMP,
		WA_Title,			"Field selection",
		TAG_DONE))
	{
		GT_RefreshWindow(w, NULL);

		return(w);
	}
	return(NULL);
}

BOOL loadIcons(struct DiskObject *dob[])
{
	STRPTR name;
	WORD i;

	for (i = 0; i < BASIC_TYPES; i++)
	{
		name = iconNames[i];
		if (dob[i] = GetDiskObject(name))
		{
		}
	}
	return(TRUE);
}

void unloadIcons(struct DiskObject *dob[])
{
	WORD i;

	for (i = 0; i < BASIC_TYPES; i++)
	{
		if (dob[i])
			FreeDiskObject(dob[i]);
	}
}

struct Window *createBoardWindow(struct Screen *s)
{
	struct Window *w;

	if (w = OpenWindowTags(NULL,
		WA_CustomScreen,	s,
		WA_Left,			0,
		WA_Top,				s->BarHeight + 1,
		WA_InnerWidth,		480,
		WA_InnerHeight,		240,
		WA_Title,			"Warehouse board",
		WA_DragBar,			TRUE,
		WA_DepthGadget,		TRUE,
		WA_CloseGadget,		TRUE,
		WA_IDCMP,			IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE | IDCMP_CLOSEWINDOW,
		WA_ReportMouse,		TRUE,
		TAG_DONE))
	{
		return(w);
	}
	return(NULL);
}

void mainLoop(struct Window *w, struct Window *sw, struct Gadget *gads[], struct DiskObject *dob[])
{
	BOOL done = FALSE, paint = FALSE;
	ULONG signals[] =
	{
		1L << w->UserPort->mp_SigBit,
		1L << sw->UserPort->mp_SigBit
	};

	ULONG total = signals[0] | signals[1];
	struct tileInfo img;

	img.floor = FLOOR_NONE;
	img.flag = FLAGSTONE_SQUARE;
	img.dir = DIR_LEFT;
	img.object = OBJECT_NONE;
	img.item = ITEM_COINS;
	img.key = KEY_GOLD;

	while (!done)
	{
		struct IntuiMessage *msg;
		ULONG result = Wait(total);

		if (result & signals[1])
		{
			while (msg = GT_GetIMsg(sw->UserPort))
			{
				if (msg->Class == IDCMP_CLOSEWINDOW)
				{
					done = TRUE;
				}
				else if (msg->Class == IDCMP_GADGETUP)
				{
					struct Gadget *gad = (struct Gadget *)msg->IAddress;

					if (gad->GadgetID == GID_FLOOR)
					{
						img.floor = msg->Code;

						if (msg->Code == FLOOR_FLAGSTONE)
						{
							GT_SetGadgetAttrs(gads[GID_FLAGSTONE], sw, NULL, GA_Disabled, FALSE, TAG_DONE);
						}
						else
						{
							GT_SetGadgetAttrs(gads[GID_FLAGSTONE], sw, NULL, GA_Disabled, TRUE, TAG_DONE);
						}

						if (msg->Code == FLOOR_DOOR)
						{
							GT_SetGadgetAttrs(gads[GID_KEY], sw, NULL, GA_Disabled, FALSE, TAG_DONE);
						}
						else
						{
							GT_SetGadgetAttrs(gads[GID_KEY], sw, NULL, GA_Disabled, TRUE, TAG_DONE);
						}

						if (msg->Code == FLOOR_SLIDER)
						{
							GT_SetGadgetAttrs(gads[GID_DIR], sw, NULL, GA_Disabled, FALSE, TAG_DONE);
						}
						else
						{
							GT_SetGadgetAttrs(gads[GID_DIR], sw, NULL, GA_Disabled, TRUE, TAG_DONE);
						}

						if (msg->Code == FLOOR_DOOR)
						{
							GT_SetGadgetAttrs(gads[GID_OBJECT], sw, NULL, GA_Disabled, TRUE, TAG_DONE);
						}
						else
						{
							GT_SetGadgetAttrs(gads[GID_OBJECT], sw, NULL, GA_Disabled, FALSE, TAG_DONE);
						}
					}
					else if (gad->GadgetID == GID_OBJECT)
					{
						img.object = msg->Code;

						if (msg->Code == OBJECT_BOX)
						{
							GT_SetGadgetAttrs(gads[GID_FLAGSTONE], sw, NULL, GA_Disabled, FALSE, TAG_DONE);
						}
						else
						{
							GT_SetGadgetAttrs(gads[GID_FLAGSTONE], sw, NULL, GA_Disabled, TRUE, TAG_DONE);
						}

						if (msg->Code == OBJECT_ITEM)
						{
							GT_SetGadgetAttrs(gads[GID_ITEM], sw, NULL, GA_Disabled, FALSE, TAG_DONE);
						}
						else
						{
							GT_SetGadgetAttrs(gads[GID_ITEM], sw, NULL, GA_Disabled, TRUE, TAG_DONE);
						}
					}
					else if (gad->GadgetID == GID_ITEM)
					{
						img.item = msg->Code;

						if (msg->Code == ITEM_KEY)
						{
							GT_SetGadgetAttrs(gads[GID_KEY], sw, NULL, GA_Disabled, FALSE, TAG_DONE);
						}
						else
						{
							GT_SetGadgetAttrs(gads[GID_KEY], sw, NULL, GA_Disabled, TRUE, TAG_DONE);
						}
					}
					else if (gad->GadgetID == GID_DIR)
					{
						img.dir = msg->Code;
					}
				}
				GT_ReplyIMsg(msg);
			}
		}

		if (result & signals[0])
		{
			while (msg = GT_GetIMsg(w->UserPort))
			{
				ULONG class = msg->Class;
				WORD code = msg->Code;
				WORD mx = (msg->MouseX - w->BorderLeft) >> 5;
				WORD my = (msg->MouseY - w->BorderTop) >> 4;
				static WORD oldx = -1, oldy = -1;

				if ((class == IDCMP_MOUSEBUTTONS && code == IECODE_LBUTTON) || (class == IDCMP_MOUSEMOVE && paint && (oldx != mx || oldy != my)))
				{
					if (mx >= 1 && mx < 14 && my >= 1 && my < 14)
					{
						oldx = mx;
						oldy = my;

						WORD basic = BASIC_FLOOR;

						if (img.object == OBJECT_NONE)
						{
							switch (img.floor)
							{
								case FLOOR_WALL:
									basic = BASIC_WALL; break;
								case FLOOR_DOOR:
									basic = BASIC_DOOR; break;
								case FLOOR_FLAGSTONE:
									basic = BASIC_PLACE; break;
								case FLOOR_SLIDER:
									basic = BASIC_SLIDELEFT + img.dir;
									break;
							}
						}
						else switch (img.object)
						{
							case OBJECT_ITEM:
								switch (img.item)
								{
									case ITEM_COINS: 	basic = BASIC_COINS; break;
									case ITEM_FRUITS:	basic = BASIC_FRUITS; break;
									case ITEM_KEY:	basic = BASIC_KEY; break;
								}
								break;
							case OBJECT_BOX:
								basic = BASIC_BOX;
								break;
							case OBJECT_PLAYER:
								basic = BASIC_PLAYER;
								break;
							case OBJECT_SKULL:
								basic = BASIC_SKULL;
								break;
						}

						DrawImage(w->RPort, (struct Image *)dob[basic]->do_Gadget.GadgetRender, w->BorderLeft + (mx << 5), w->BorderTop + (my << 4));
						paint = TRUE;
					}
				}
				else if (class == IDCMP_MOUSEBUTTONS && code == (IECODE_LBUTTON | IECODE_UP_PREFIX))
				{
					paint = FALSE;
				}
				else if (class == IDCMP_CLOSEWINDOW)
				{
					done = TRUE;
				}
				GT_ReplyIMsg(msg);
			}
		}
	}

}

int main()
{
	struct Screen *s;
	UWORD pens[] = { ~0 };
	ULONG colors[(1 * 3) + 2] = { 1 << 16 };
	struct DiskObject *dob[BASIC_TYPES] = { 0 };

	if (s = OpenScreenTags(NULL,
		SA_Left,	0,
		SA_Top,		0,
		SA_Width,	640,
		SA_Height,	256,
		SA_Depth,	4,
		SA_FullPalette, TRUE,
		SA_DisplayID,	HIRES_KEY,
		SA_Title,	"Warehouse - Editor",
		SA_Pens,	pens,
		SA_Exclusive,	TRUE,
		SA_Colors32,	myPaletteRGB32,
		TAG_DONE))
	{
		struct Window *w;
		if (loadIcons(dob))
		{
			if (w = createBoardWindow(s))
			{
				WORD x, y, z;

				for (z = 0; z < 2; z++)
				{
					for (x = z; x < 15; x += 2)
					{
						for (y = 0; y < 15; y++)
						{
							WORD img = BASIC_FLOOR;
							if (x == 0 || x == 14 || y == 0 || y == 14)
								img = BASIC_WALL;
							DrawImage(w->RPort, (struct Image *)dob[img]->do_Gadget.GadgetRender, w->BorderLeft + (x << 5), w->BorderTop + (y << 4));
						}
						WaitTOF();
					}
				}
				struct VisualInfo *vi;

				if (vi = GetVisualInfoA(s, NULL))
				{
					struct Gadget *glist, *gads[GID_COUNT];

					if (glist = createFieldSelectionGadgets(vi, s->Font, gads))
					{
						struct Window *sw;
						if (sw = createFieldSelectionWindow(s, vi, glist, gads))
						{
							mainLoop(w, sw, gads, dob);
							CloseWindow(sw);
						}
						FreeGadgets(glist);
					}
					FreeVisualInfo(vi);
				}
				CloseWindow(w);
			}
			unloadIcons(dob);
		}
		CloseScreen(s);
	}

	return(0);
}
