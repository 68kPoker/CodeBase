
/* Magazyn: Gadûety */

#ifndef GADGETS_H
#define GADGETS_H

#include <exec/types.h>

#define WordWidth(w) (((w)+15)>>4)

#define TEXT_COLOR 10

enum GadgetTypes
{
    GAD_BUTTON, /* Przycisk        */
    GAD_TOGGLE, /* Przeîâcznik     */
    GAD_MX,     /* Wybór opcji     */
    GAD_STRING, /* Wpisanie tekstu */
    GADGETS
};

enum ButtonTypes
{
    BUTTON_NOGFX, /* Przycisk z wîasnâ grafikâ */
    BUTTON_GFX,   /* Przycisk graficzny        */
    BUTTON_TEXT,  /* Przycisk tekstowy         */
    BUTTONS
};

VOID initText(struct IntuiText *text, STRPTR name);
VOID initButton(struct Gadget *gad, struct IntuiText *text, WORD gid, WORD x, WORD y, struct Image *render, struct Image *select);
BOOL cutImage(struct Image *img, struct BitMap *bm, WORD x, WORD y, WORD width, WORD height);

#endif /* GADGETS_H */
