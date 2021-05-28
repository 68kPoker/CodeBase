
/* Interfejs uûytkownika */

#ifndef GUI_H
#define GUI_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef GRAPHICS_TEXT_H
#include <graphics/text.h>
#endif

#define GLEB 5

enum /* Typy okien */
{
    OKNO_GLOWNE,
    OKNO_OPCJI /* Opcje planszy */
};

struct ekranInfo
{
    struct Screen *s;
    struct VisualInfo *vi;
    struct ScreenBuffer *bufory[2];
    struct MsgPort *porty[2];
    BOOL moznaRysowac, moznaZmieniac;
    struct TextAttr ta;
};

struct oknoInfo
{
    struct Window *w;
    struct Menu *menu;
    struct Gadget *gadzety;
};

#endif /* GUI_H */
