
#include <exec/types.h>

extern struct copperData
{
	struct ViewPort *vp;
	WORD signal;
	struct Task *task;
} copperData;

struct Screen *openScreen(void);
void closeScreen(struct Screen *s);
struct Window *openBoardWindow(struct Screen *s);
