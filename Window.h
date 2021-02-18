
#include <exec/types.h>
#include <intuition/intuition.h>

enum
{
    IMG_BUTTON,
    IMG_PRESSED,
    IMG_COUNT
};

enum
{
    GID_MENU1,
    GID_MENU2,
    GID_MENU3,
    GID_MENU4,
    GID_MENU5,
    GID_COUNT
};

struct windowInfo
{
    struct Gadget gads[GID_COUNT];
    struct IntuiText text[GID_COUNT];
    struct Image img[IMG_COUNT];
};

void initWindow(struct windowInfo *wi, struct BitMap *gfx);
void freeWindow(struct windowInfo *wi);
struct Window *openBDWindow(struct Screen *s, struct windowInfo *wi);
struct Window *openMenuWindow(struct Window *p, WORD width, WORD height);

void moveWindow(struct Window *w, WORD dx, WORD dy);
