
#include "Menu.h"

/* Otwórz menu */
BOOL openMenu(struct myMenu *menu, struct Screen *s)
{
    if (menu->window = OpenWindowTags(NULL,
        WA_CustomScreen,    s,
        WA_Left,            menu->left,
        WA_Top,             menu->top,
        WA_Width,           menu->width,
        WA_Height,          menu->height,
        WA_Borderless,      TRUE,
        WA_RMBTrap,         TRUE,
        WA_Activate,        TRUE,
        WA_IDCMP,           IDCMP_GADGETDOWN|IDCMP_MOUSEBUTTONS,
        WA_Gadgets,         menu->gadgets,
        TAG_DONE))
    {
        return(TRUE);
    }
    return(FALSE);
}

/* Obsîuû menu */
BOOL handleMenu(struct myMenu *menu)
{
    struct IntuiMessage *msg;

    while (msg = GT_GetIMsg(menu->window->UserPort))
    {
        ULONG class = msg->Class;
        UWORD code = msg->Code;
        WORD mx = msg->MouseX;
        WORD my = msg->MouseY;
        APTR iaddress = msg->IAddress;
        GT_ReplyIMsg(msg);

        if (class == IDCMP_GADGETDOWN)
        {

        }
        else if (class == IDCMP_MOUSEBUTTONS)
        {
            if (code == IECODE_RBUTTON)
            {
                return(TRUE);
            }
        }
    }
    return(FALSE);
}

/* Zamknij menu */
void closeMenu(struct myMenu *menu)
{
    CloseWindow(menu->window);
}
