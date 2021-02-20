
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

enum
{
    MID_SAVE,
    MID_RESTORE,
    MID_NEXT,
    MID_PREV,
    MID_COUNT
};

struct windowInfo
{
    struct Gadget gads[GID_COUNT];
    struct IntuiText text[GID_COUNT];
    struct Image img[IMG_COUNT];
};

struct menuInfo
{
    struct Gadget gads[MID_COUNT];
    struct IntuiText text[MID_COUNT];
    struct Image *img;
};

void initWindow(struct windowInfo *wi, struct BitMap *gfx);
void freeWindow(struct windowInfo *wi);
struct Window *openBDWindow(struct Screen *s, struct windowInfo *wi);
struct Window *openMenuWindow(struct Window *p, WORD left, WORD width, WORD height, struct Gadget *gads);

void initEditorMenu(struct menuInfo *mi, struct windowInfo *wi);

void moveWindow(struct Window *w, WORD dx, WORD dy);
