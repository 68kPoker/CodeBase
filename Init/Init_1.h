
#include <exec/types.h>

struct BitMap *initScreen(struct screenAux *sa, STRPTR name);
void freeScreen(struct screenAux *sa, struct BitMap *gfx);
BOOL initWindows(struct Screen *s, struct BitMap *gfx, struct Window *w[]);
void freeWindows(struct Window *w[]);
