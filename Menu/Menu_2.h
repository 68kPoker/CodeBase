
/* Menu dla Amigi */

#include <exec/types.h>

/* Menu skîada sië z okna z przyciskami i grafikâ */

/* Wyróûniamy Menu gîówne */

struct myMenu
{
    struct myMenu *parent; /* Menu nadrzëdne */
    WORD left, top, width, height; /* Wymiary */
    struct Window *window; /* Okno z menu */
    struct Gadget *gadgets; /* Gadûety */
};

/* Otwórz menu */
BOOL openMenu(struct myMenu *menu, struct Screen *s);

/* Obsîuû menu */
BOOL handleMenu(struct myMenu *menu);

/* Zamknij menu */
void closeMenu(struct myMenu *menu);
