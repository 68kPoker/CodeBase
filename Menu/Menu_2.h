
/* Menu dla Amigi */

#include <exec/types.h>

/* Menu sk�ada si� z okna z przyciskami i grafik� */

/* Wyr��niamy Menu g��wne */

struct myMenu
{
    struct myMenu *parent; /* Menu nadrz�dne */
    WORD left, top, width, height; /* Wymiary */
    struct Window *window; /* Okno z menu */
    struct Gadget *gadgets; /* Gad�ety */
};

/* Otw�rz menu */
BOOL openMenu(struct myMenu *menu, struct Screen *s);

/* Obs�u� menu */
BOOL handleMenu(struct myMenu *menu);

/* Zamknij menu */
void closeMenu(struct myMenu *menu);
