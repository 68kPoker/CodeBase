
#ifndef SCREEN_H
#define SCREEN_H

#include <exec/types.h>

struct copperInfo 
{
	struct ViewPort *vp;
	WORD signal;
	struct Task *task;
};	

struct screenInfo
{
	struct BitMap *bm[2];
	struct Screen *s;
	struct DBufInfo *dbi;
	WORD signal;
	struct MsgPort *safeport;
	BOOL safe;
	UWORD frame;
};	

struct Screen *openScreen(struct screenInfo *si);
VOID closeScreen(struct screenInfo *si);

#endif /* SCREEN_H */
