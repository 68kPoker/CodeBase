
#ifndef WINGAD_PROTOS_H
#define WINGAD_PROTOS_H

#include "Screen.h"
#include "IFF.h"

#include <intuition/classes.h>
#include <intuition/classusr.h>

/*==================== Okienka i gadûety ====================*/

Class *makeImage();

BOOL initImages(Object *img[], Class *cl, struct BitMap *gfx);
void freeImages(Object *img[], WORD count);

WIN *openWindow(SCR *scr, Object *img[]);
WIN *openBoardWindow(SCR *scr);
WIN *openMenu(SCR *scr, Object *img[], GFX *gfx, WORD type, WORD x, WORD y);

void closeWindow(WIN *);
void closeMenu(WIN *win);

#endif /* WINGAD_PROTOS_H */
