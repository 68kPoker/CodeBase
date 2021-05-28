
#include <exec/types.h>

extern WORD added; /* Added gadgets */

void addGadget( struct Gadget *glist, WORD left, WORD top, WORD width, WORD height );
struct Window *openWindow( WORD x, WORD y, WORD w, WORD h, struct Screen *s, struct Gadget *glist, LONG idcmp );
struct Screen *openScreen( STRPTR title, UBYTE depth, ULONG *colors );
void closeScreen(struct Screen *s);
