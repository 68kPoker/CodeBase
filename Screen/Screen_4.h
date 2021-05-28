
#ifndef SCREEN_H
#define SCREEN_H

#include <exec/types.h>
#include <exec/interrupts.h>

struct copperData
{
	struct ViewPort *vp;
	WORD signal;
	struct Task *task;
};

struct screenData
{
	struct BitMap *bm[2];
	struct Screen *s;
	struct DBufInfo *dbi;
	struct Interrupt is;
};

BOOL 			allocBitMaps	(struct BitMap *bmarray[], UWORD wide, UWORD tall, UBYTE deep, ULONG flags, struct BitMap *friend);
struct Screen 	*openScreen		(struct Rectangle *dclip, struct BitMap *bmarray[], ULONG modeID, STRPTR title, struct TextAttr *ta, ULONG *colors);
struct DBufInfo *allocDBufInfo	(struct ViewPort *vp);
void 			freeDBufInfo	(struct DBufInfo *dbi);
BOOL 			addUCL			(struct ViewPort *vp);
BOOL 			addCopper		(struct Interrupt *is, struct ViewPort *vp);
void 			remCopper		(struct Interrupt *is);

BOOL 			setupScreen		(struct screenData *sd);
void 			cleanupScreen	(struct screenData *sd);

#endif /* SCREEN_H */
