
#include <exec/types.h>

extern struct copperData
{
	struct ViewPort *vp;
	WORD signal;
	struct Task *task;
} copperData;

struct Screen *openScreen(void);
struct Window *openBoardWindow(struct Screen *s, struct BitMap *gfx);
void closeWindow(struct Window *w);
