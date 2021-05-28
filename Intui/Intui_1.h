
#include <exec/types.h>

void setupText(struct IntuiText *it, UBYTE fpen, UBYTE bpen, UBYTE drmd, WORD left, WORD top, struct TextAttr *ta, UBYTE *text, struct IntuiText *next);
void setupImage(struct Image *img, WORD left, WORD top, WORD width, WORD height, WORD depth, UWORD *data, UBYTE pick, UBYTE onoff, struct Image *next);
void setupGadget(struct Gadget *gad, struct Gadget *next, WORD left, WORD top, WORD width, WORD height, UWORD flags, UWORD act, UWORD type, APTR render, APTR select, struct IntuiText *it, APTR info, UWORD id, APTR user);
