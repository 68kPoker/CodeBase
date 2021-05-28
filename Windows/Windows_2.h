
#include <graphics/gfx.h>
#include <exec/nodes.h>

struct windowData
{
	struct Node node;
	struct Window *w;
	struct Rectangle bounds[2];
	struct Region *reg;
	void (*update)(struct windowData *wd, BOOL update, struct Region *refresh);
	WORD drawn;
};

struct Window *openWindow(struct Screen *s, ULONG idcmp);
BOOL prepRegion(struct List *list, struct windowData *wd, struct Window *w, WORD left, WORD top, WORD width, WORD height, void (*update)(struct windowData *wd, BOOL update, struct Region *refresh));
void freeRegion(struct windowData *wd);
