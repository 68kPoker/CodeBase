
/* Magazyn */

/* GUI.c - Graficzny interfejs uzytkownika */

#include <intuition/intuition.h>
#include <clib/intuition_protos.h>
#include <clib/exec_protos.h>

#define DEPTH 5

#define ESC_KEY 0x45

enum
{
	TILE_FLOOR,
	TILE_WALL,
	TILE_BOX,
	TILE_FLAGSTONE,
	TILES
};	

enum
{
	GID_CLOSE,
	GID_DEPTH,
	GID_TILE,
	GID_BOARD,
	MAIN_GADS
};

struct Screen *screen;
struct Gadget mainGList[MAIN_GADS];

/* Przygotuj gadzet */

struct Gadget *prepGadget(struct Gadget *gad, struct Gadget *prev, WORD x, WORD y, WORD w, WORD h, WORD gid)
{
	if (prev)
		prev->NextGadget = gad;
		
	gad->NextGadget = NULL;
	gad->LeftEdge = x;
	gad->TopEdge = y;
	gad->Width = w;
	gad->Height = h;
	gad->Flags = GFLG_GADGHNONE;
	gad->Activation = GACT_IMMEDIATE|GACT_RELVERIFY|GACT_FOLLOWMOUSE;
	gad->GadgetType = GTYP_BOOLGADGET;
	gad->GadgetRender = gad->SelectRender = NULL;
	gad->GadgetText = NULL;
	gad->GadgetID = gid;
	gad->MutualExclude = 0;
	gad->SpecialInfo = NULL;
	gad->UserData = NULL;
	
	return(gad);
}

/* Przygotuj gadzety w oknie glownym */

void prepMainGList(void)
{
	struct Gadget *prev;
	
	prev = prepGadget(mainGList + GID_CLOSE, NULL, 0, 0, 16, 16, GID_CLOSE);
	prev = prepGadget(mainGList + GID_TILE, prev, 16, 0, 16 * TILES, 16, GID_TILE);
	prev = prepGadget(mainGList + GID_DEPTH, prev, 304, 0, 16, 16, GID_DEPTH);
	prev = prepGadget(mainGList + GID_BOARD, prev, 0, 16, 320, 240, GID_BOARD);
}

/* Otworz okno glowne */

struct Window *openMainWindow(void)
{
	struct Window *w;
	
	prepMainGList();
	
	if (w = OpenWindowTags(NULL,
		WA_Gadgets,			mainGList,
		WA_CustomScreen,	screen,
		WA_Left,			0,
		WA_Top,				0,
		WA_Width,			screen->Width,
		WA_Height,			screen->Height,
		WA_Backdrop,		TRUE,
		WA_Borderless,		TRUE,
		WA_Activate,		TRUE,
		WA_IDCMP,			IDCMP_GADGETUP|IDCMP_GADGETDOWN|IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE,
		TAG_DONE))
	{	
		return(w);
	}
	return(NULL);
}

/* Przygotuj ekran */

struct Window *openScreen(void)
{
	struct Rectangle dclip = { 0, 0, 319, 255 };
	
	if (screen = OpenScreenTags(NULL,
		SA_DClip,		&dclip,
		SA_BackFill,	LAYERS_NOBACKFILL,
		SA_Depth,		DEPTH,
		SA_DisplayID,	LORES_KEY,
		SA_Quiet,		TRUE,
		SA_ShowTitle,	FALSE,
		SA_Exclusive,	TRUE,
		SA_Interleaved,	TRUE,
		TAG_DONE))		
	{	
		struct Window *w;

		if (w =	openMainWindow())
		{
			return(w);
		}	
		CloseScreen(screen);
	}
	return(NULL);
}	

void closeScreen(struct Window *w)
{
	struct Screen *s = w->WScreen;
	
	CloseWindow(w);
	CloseScreen(s);
}	

int main(void)
{
	struct Window *w;
	
	if (w = openScreen())
	{	
		BOOL done = FALSE;
		struct Gadget *active = NULL;
		
		while (!done)
		{
			struct IntuiMessage *msg;
			
			WaitPort(w->UserPort);
			while (msg = (struct IntuiMessage *)GetMsg(w->UserPort))
			{
				if (msg->Class == IDCMP_RAWKEY)
				{
					if (msg->Code == ESC_KEY)
					{
						done = TRUE;
					}	
				}	
				else if (msg->Class == IDCMP_GADGETDOWN)
				{
					struct Gadget *gad = (struct Gadget *)msg->IAddress;
					struct RastPort *rp = w->RPort;
					
					if (gad->GadgetID == GID_BOARD)
					{
						SetAPen(rp, 3);
						RectFill(rp, msg->MouseX & 0xfff0, msg->MouseY & 0xfff0, msg->MouseX | 0x000f, msg->MouseY | 0x000f);
					}
					else
					{
						SetAPen(rp, 3);
						RectFill(rp, gad->LeftEdge, gad->TopEdge, gad->LeftEdge + gad->Width - 1, gad->TopEdge + gad->Height - 1);
					}	
					
					active = gad;
				}
				else if (msg->Class == IDCMP_GADGETUP)
				{
					struct Gadget *gad = (struct Gadget *)msg->IAddress;
					struct RastPort *rp = w->RPort;
					
					if (gad->GadgetID != GID_BOARD)
					{
						SetAPen(rp, 2);
						RectFill(rp, gad->LeftEdge, gad->TopEdge, gad->LeftEdge + gad->Width - 1, gad->TopEdge + gad->Height - 1);
					}	
					
					active = NULL;
				}
				else if (msg->Class == IDCMP_MOUSEBUTTONS)
				{
					if (msg->Code == (IECODE_LBUTTON|IECODE_UP_PREFIX))
					{
						struct RastPort *rp = w->RPort;
						
						if (active && active->GadgetID != GID_BOARD)
						{
							SetAPen(rp, 0);
							RectFill(rp, active->LeftEdge, active->TopEdge, active->LeftEdge + active->Width - 1, active->TopEdge + active->Height - 1);	
						}	
					}
				}	
				else if (msg->Class == IDCMP_MOUSEMOVE)
				{
					if (active && active->GadgetID == GID_BOARD)
					{
						if (msg->MouseX >= active->LeftEdge && msg->MouseX < active->LeftEdge + active->Width && msg->MouseY >= active->TopEdge && msg->MouseY < active->TopEdge + active->Height)
						{
							static WORD prevx = -1, prevy = -1;
							if ((msg->MouseX >> 4) != prevx || (msg->MouseY >> 4) != prevy)
							{
								struct RastPort *rp = w->RPort;
							
								SetAPen(rp, 3);
								RectFill(rp, msg->MouseX & 0xfff0, msg->MouseY & 0xfff0, msg->MouseX | 0x000f, msg->MouseY | 0x000f);						
								prevx = msg->MouseX >> 4;
								prevy = msg->MouseY >> 4;
							}	
						}	
					}
				}
			
				ReplyMsg((struct Message *)msg);
			}	
		}	
		closeScreen(w);
	}
	return(0);
}
