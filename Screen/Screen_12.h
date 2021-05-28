
/*
** Screen related.
*/

#include <exec/types.h>

struct Screen *openScreen(struct ColorMap *cm, UBYTE depth);
struct Window *openWindow(struct Screen *s);
