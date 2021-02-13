
/* $Id: System.c,v 1.1 12/.0/.0 .1:.0:.1 Unknown Exp Locker: Unknown $ */

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

extern struct Custom custom;

extern void myCopper(); /* Server */

struct Library *IntuitionBase, *GfxBase, *IFFParseBase, *LayersBase;

static struct Windows *initGUI();
static void cleanupGUI();
static struct Window *openWindow(WORD i);
static void closeWindow(WORD i);

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
	openWindow,
	closeWindow
};

static struct Window *openWindow(WORD i)
{
	struct WindowInfo *wi;

	if (wi = AllocMem(sizeof(*wi), MEMF_PUBLIC|MEMF_CLEAR))
	{
		wi->gui = &gui;
		if (i == 0)
		{
			/* Backdrop */
			if (windows.win[0] = OpenWindowTags(NULL,
				WA_CustomScreen,	gui.screen,
				WA_Left,			0,
				WA_Top,				0,
				WA_Width,			gui.screen->Width,
				WA_Height,			gui.screen->Height,
				WA_Backdrop,		TRUE,
				WA_Borderless,		TRUE,
				WA_Activate,		TRUE,
				WA_RMBTrap,			TRUE,
				WA_IDCMP,			IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE,
				WA_ReportMouse,		TRUE,
				TAG_DONE))
			{
				windows.win[0]->UserData = (APTR)wi;
				wi->win = windows.win[0];
				return(windows.win[0]);
			}
		}
		else if (i == 1)
		{
			/* Icon selection */
			if (windows.win[1] = OpenWindowTags(NULL,
				WA_CustomScreen,	gui.screen,
				WA_Left,			0,
				WA_Top,				240,
				WA_Width,			gui.screen->Width,
				WA_Height,			16,
				WA_Borderless,		TRUE,
				WA_Activate,		TRUE,
				WA_RMBTrap,			TRUE,
				WA_IDCMP,			IDCMP_MOUSEBUTTONS,
				WA_SimpleRefresh,	TRUE,
				WA_BackFill,		LAYERS_NOBACKFILL,
				TAG_DONE))
			{
				windows.win[1]->UserData = (APTR)wi;
				wi->win = windows.win[1];
				/* printf("Menu opened\n"); */
				return(windows.win[1]);
			}
		}
		else if (i == 2)
		{
			if (windows.win[2] = OpenWindowTags(NULL,
				WA_CustomScreen,	gui.screen,
				WA_Left,			120,
				WA_Top,				100,
				WA_Width,			80,
				WA_Height,			80,
				WA_Borderless,		TRUE,
				WA_Activate,		TRUE,
				WA_RMBTrap,			TRUE,
				WA_IDCMP,			IDCMP_MOUSEBUTTONS,
				WA_SimpleRefresh,	TRUE,
				WA_BackFill,		LAYERS_NOBACKFILL,
				TAG_DONE))
			{
				windows.win[2]->UserData = (APTR)wi;
				wi->win = windows.win[2];
				/* printf("Intro opened\n"); */
				return(windows.win[2]);
			}
		}
		FreeMem(wi, sizeof(*wi));
	}
	return(NULL);
}

static void closeWindow(WORD i)
{
	if (windows.win[i])
	{
		FreeMem(windows.win[i]->UserData, sizeof(struct WindowInfo));
		CloseWindow(windows.win[i]);
		windows.win[i] = NULL;
		/* printf("Window %d closed\n", i); */
	}
}

static BOOL openLibs()
{
	/* printf("[internal] system.openLibs()\n"); */

	if (IntuitionBase = OpenLibrary("intuition.library", 39))
	{
		if (GfxBase = OpenLibrary("graphics.library", 39))
		{
			if (IFFParseBase = OpenLibrary("iffparse.library", 39))
			{
				if (LayersBase = OpenLibrary("layers.library", 39))
				{
					return(TRUE);
				}
				CloseLibrary(IFFParseBase);
			}
			CloseLibrary(GfxBase);
		}
		CloseLibrary(IntuitionBase);
	}
	return(FALSE);
}

static void closeLibs()
{
	/* printf("[internal] system.closeLibs()\n"); */

	CloseLibrary(LayersBase);
	CloseLibrary(IFFParseBase);
	CloseLibrary(GfxBase);
	CloseLibrary(IntuitionBase);
}

static struct Windows *initGUI()
{
	/* printf("gui.init()\n"); */

	if (gui.bitmaps[0] = AllocBitMap(gui.rasWidth, gui.rasHeight, gui.rasDepth, BMF_DISPLAYABLE|BMF_INTERLEAVED|BMF_CLEAR, NULL))
	{
		if (gui.bitmaps[1] = AllocBitMap(gui.rasWidth, gui.rasHeight, gui.rasDepth, BMF_DISPLAYABLE|BMF_INTERLEAVED|BMF_CLEAR, NULL))
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

								if (windows.open(0))
								{
									return(&windows);
								}
								FreeSignal(gui.cop.signal);
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
	/* printf("gui.cleanup()\n"); */

	windows.close(2);
	windows.close(1);
	windows.close(0);

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
	/* printf("system.init()\n"); */

	if (openLibs())
	{
		if (gui.windows = gui.init())
		{
			return(&gui);
		}
		closeLibs();
	}
	return(NULL);
}

/* Main loop */
static LONG loopSystem()
{
	ULONG signals[SRC_COUNT] = { 0 }, total = 0;
	WORD i;

	/* printf("system.loop()\n"); */

	while (!system.done)
	{
		ULONG result;
		total = 0;
		for (i = 0; i < MAX_WINDOWS; i++)
		{
			if (windows.win[i])
			{
				total |= signals[SRC_WINDOWS + i] = 1L << windows.win[i]->UserPort->mp_SigBit;
			}
		}

		total |= signals[SRC_ANIMFRAME] = 1L << gui.cop.signal;

		result = Wait(total);


		for (i = 0; i < MAX_WINDOWS; i++)
		{
			if (windows.win[i] && (result & signals[SRC_WINDOWS + i]))
			{
				struct WindowInfo *wi = (struct WindowInfo *)windows.win[i]->UserData;

				wi->handleIDCMP(wi);
			}
		}

		if (result & signals[SRC_ANIMFRAME])
		{
			system.handleAnimFrame(&gui);
		}
	}
	return(0);
}

static void cleanupSystem()
{
	/* printf("system.cleanup()\n"); */
	gui.cleanup();
	closeLibs();
}

extern struct System system =
{
	initSystem,
	loopSystem,
	cleanupSystem
};
