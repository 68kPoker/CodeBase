
/* $Id$ */

/*
 * System.h
 */

#include <exec/types.h>
#include <exec/interrupts.h>
#include <dos/dos.h>

struct Windows
{
	struct Window	(*open)(); /* Open window with current params */
	void 			(*close)(struct Window *w); /* Close window */
	WORD 	left,
			top,
			width,
			height;
	ULONG 	idcmp;
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
	struct Window 		*window; /* Backdrop */
};

extern struct System
{
	struct GUI *(*init)(); /* General init (libs, devs etc.) */
	void (*cleanup)(); /* Clean up */
} system;
