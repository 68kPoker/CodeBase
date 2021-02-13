
/* $Id$ */

#include <stdio.h>
#include <intuition/intuition.h>
#include <exec/memory.h>
#include <hardware/intbits.h>
#include <hardware/custom.h>
#include <graphics/gfxmacros.h>

#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>

#include "System.h"

extern __far struct Custom custom;

extern void myCopper(); /* Server */

struct Library *IntuitionBase, *GfxBase, *IFFParseBase;

static struct Windows *initGUI();
static void cleanupGUI();

static struct GUI gui =
{
	initGUI,
	cleanupGUI,
	320, 256, 5,
	320, 256,
	LORES_KEY
};

static struct Windows windows =
{
	0
};

static BOOL openLibs()
{
	printf("[internal] system.openLibs()\n");

	if (IntuitionBase = OpenLibrary("intuition.library", 39))
	{
		if (GfxBase = OpenLibrary("graphics.library", 39))
		{
			if (IFFParseBase = OpenLibrary("iffparse.library", 39))
			{
				return(TRUE);
			}
			CloseLibrary(GfxBase);
		}
		CloseLibrary(IntuitionBase);
	}
	return(FALSE);
}

static void closeLibs()
{
	printf("[internal] system.closeLibs()\n");

	CloseLibrary(IFFParseBase);
	CloseLibrary(GfxBase);
	CloseLibrary(IntuitionBase);
}

static struct Windows *initGUI()
{
	printf("gui.init()\n");

	if (gui.bitmaps[0] = AllocBitMap(gui.rasWidth, gui.rasHeight, gui.rasDepth, BMF_DISPLAYABLE|BMF_CLEAR, NULL))
	{
		if (gui.bitmaps[1] = AllocBitMap(gui.rasWidth, gui.rasHeight, gui.rasDepth, BMF_DISPLAYABLE|BMF_CLEAR, NULL))
		{
			if (gui.screen = OpenScreenTags(NULL,
				SA_Left,		0,
				SA_Top,			0,
				SA_Width,		gui.scrWidth,
				SA_Height,		gui.scrHeight,
				SA_Depth,		gui.rasDepth,
				SA_DisplayID,	gui.modeID,
				SA_BitMap,		gui.bitmaps[0],
				SA_Quiet,		TRUE,
				SA_Exclusive,	TRUE,
				SA_ShowTitle,	FALSE,
				SA_BackFill,	LAYERS_NOBACKFILL,
				TAG_DONE))
			{
				if (gui.dbi = AllocDBufInfo(&gui.screen->ViewPort))
				{
					if (gui.safeport = CreateMsgPort())
					{
						struct UCopList *ucl;
						const WORD copperCommands = 4, copperPri = 0;

						gui.dbi->dbi_SafeMessage.mn_ReplyPort = gui.safeport;
						gui.safeToWrite = TRUE;
						gui.frame = 1;

						if (ucl = AllocMem(sizeof(*ucl), MEMF_PUBLIC|MEMF_CLEAR))
						{
							CINIT(ucl, copperCommands);
							CWAIT(ucl, 0, 0);
							CMOVE(ucl, custom.intreq, INTF_SETCLR|INTF_COPER);
							CEND(ucl);

							Forbid();
							gui.screen->ViewPort.UCopIns = ucl;
							Permit();

							RethinkDisplay();

							if ((gui.cop.signal = AllocSignal(-1)) != -1)
							{
								gui.cop.task = FindTask(NULL);

								gui.is.is_Code = myCopper;
								gui.is.is_Data = (APTR)&gui.cop;
								gui.is.is_Node.ln_Pri = copperPri;
								gui.is.is_Node.ln_Name = "Gear Works";

								AddIntServer(INTB_COPER, &gui.is);
								return(&windows);
							}
						}
						DeleteMsgPort(gui.safeport);
					}
					FreeDBufInfo(gui.dbi);
				}
				CloseScreen(gui.screen);
			}
			FreeBitMap(gui.bitmaps[1]);
		}
		FreeBitMap(gui.bitmaps[0]);
	}
	return(NULL);
}

static void cleanupGUI()
{
	printf("gui.cleanup()\n");

	RemIntServer(INTB_COPER, &gui.is);
	FreeSignal(gui.cop.signal);

	if (!gui.safeToWrite)
	{
		while (!GetMsg(gui.safeport))
		{
			WaitPort(gui.safeport);
		}
	}

	DeleteMsgPort(gui.safeport);
	FreeDBufInfo(gui.dbi);
	CloseScreen(gui.screen);
	FreeBitMap(gui.bitmaps[1]);
	FreeBitMap(gui.bitmaps[0]);
}

static struct GUI *initSystem()
{
	printf("system.init()\n");

	if (openLibs())
	{
		if (gui.init())
		{
			return(&gui);
		}
		closeLibs();
	}
	return(NULL);
}

static void cleanupSystem()
{
	printf("system.cleanup()\n");
	gui.cleanup();
	closeLibs();
}

extern struct System system =
{
	initSystem,
	cleanupSystem
};
