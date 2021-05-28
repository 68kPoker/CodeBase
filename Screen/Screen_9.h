
#include <exec/interrupts.h>

struct screenAux
{
	struct BitMap *bitmap;
	struct TextAttr	*ta;
	struct TextFont *font;
	ULONG *colors;
	struct Screen *screen;
	struct copperAux
	{
		struct ViewPort *vp;
		WORD signal;
		struct Task *task;
	} copper;
	struct Interrupt is;
};

BOOL preOpenScreen(struct screenAux *sa, UWORD width, UWORD height, UBYTE depth, struct TextAttr *ta, struct IFFHandle *iff);
struct Screen *openScreen(struct screenAux *sa);
BOOL postOpenScreen(struct screenAux *sa);
void preCloseScreen(struct screenAux *sa);
void postCloseScreen(struct screenAux *sa);
