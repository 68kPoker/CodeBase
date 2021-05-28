
#include <exec/types.h>
#include <intuition/intuition.h>

enum
{
    GAME_OPTIONS,
    BEGIN_GAME,
    LOAD_GAME,
    SHOW_PROGRESS,
    SHOW_HISCORE,
    SHOW_AUTHORS,
    QUIT_GAME,
    PLAY_GAME,
    EDIT_LEVEL,
    PREV_LEVEL,
    NEXT_LEVEL,
    EXIT_TO_MENU,
    OPTIONS_COUNT
};

enum
{
    IMG_BUTTON,
    IMG_PRESSED,
    IMG_CLOSE,
    IMG_CLOSE_PRESSED,
    IMG_NEXT,
    IMG_NEXT_PRESSED,
    IMG_PREV,
    IMG_PREV_PRESSED,
    IMG_COUNT
};

enum
{
    GID_MENU1,
    GID_MENU2,
    GID_MENU3,
    GID_MENU4,
    GID_MENU5,
    GID_CLOSE,
    GID_COUNT
};

/* Przyciski wyboru poziomu */
enum
{
    RID_CLOSE,
    RID_OPT1,
    RID_OPT2,
    RID_PREV,
    RID_NEXT,
    RID_COUNT
};

struct windowInfo
{
    struct Gadget gads[GID_COUNT];
    struct IntuiText text[GID_COUNT];
    struct Image img[IMG_COUNT];
};

struct reqInfo
{
    struct Gadget gads[RID_COUNT];
    struct IntuiText text[RID_COUNT];
    struct Image *img;
};

extern STRPTR itemStrings[OPTIONS_COUNT];

BOOL initWindow(struct windowInfo *wi, struct BitMap *gfx);
void freeWindow(struct windowInfo *wi);
struct Window *openBDWindow(struct Screen *s);
struct Window *openReqWindow(struct Window *w);

void initReq(struct reqInfo *ri, struct windowInfo *wi, STRPTR opt1, STRPTR opt2);

void moveWindow(struct Window *w, WORD dx, WORD dy);

void initTexts(struct IntuiText text[]);
void initEditTexts(struct IntuiText text[]);

void initButton(struct Gadget *gad, struct IntuiText *text, WORD gid, WORD x, WORD y, struct Image *render, struct Image *select);

void initMenuTexts(struct IntuiText *intuiTexts, STRPTR *strings);
void initMenuButton(struct Gadget *gad, struct IntuiText *text, WORD id, WORD x, WORD y, struct Image *img);
BOOL initImages(struct Image img[], struct BitMap *gfx);
void freeImages(struct Image img[]);
