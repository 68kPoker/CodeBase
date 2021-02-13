
/* $Id: System.h,v 1.1 12/.0/.0 .1:.0:.2 Unknown Exp Locker: Unknown $ */

/*
 * System.h
 */

#include <exec/types.h>
#include <exec/interrupts.h>
#include <dos/dos.h>

#define MAX_WINDOWS 3

/* Signal sources */
enum
{
	SRC_WINDOWS,
	SRC_ANIMFRAME=MAX_WINDOWS,
	SRC_COUNT
};

struct WindowInfo
{
	struct Window *win;
	LONG (*handleIDCMP)(struct WindowInfo *wi);

	struct GUI *gui;
};

struct Windows
{
	struct Window	*(*open)(WORD i); /* Open window with current params */
	void 			(*close)(WORD i); /* Close window */
	WORD 	left,
			top,
			width,
			height;
	ULONG 	idcmp;

	struct Window *win[MAX_WINDOWS];
};

struct CopperIS
{
	struct Task *task;
	UWORD signal;
};

struct GUI
{
	struct Windows *(*init)(); /* Basic screen init */
	void (*cleanup)();

	UWORD rasWidth, rasHeight;
	UWORD rasDepth;
	UWORD scrWidth, scrHeight;
	ULONG modeID;

	struct BitMap		*bitmaps[2];
	struct Screen		*screen;
	struct Interrupt	is;
	struct CopperIS		cop;
	struct DBufInfo		*dbi;
	struct MsgPort 		*safeport;
	BOOL 				safeToWrite;
	WORD				frame;
	struct Windows 		*windows;
};

extern struct System
{
	struct GUI *(*init)(); /* General init (libs, devs etc.) */
	LONG (*loop)(); /* Main loop */
	void (*cleanup)(); /* Clean up */

	LONG (*handleAnimFrame)(struct GUI *gui); /* Called every frame */

	BOOL done;
} system;
