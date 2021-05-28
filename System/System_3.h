
#include <devices/inputevent.h>
#include <intuition/classusr.h>

struct system
{
    struct IOStdReq *joyio;
    struct InputEvent joyie;
    struct TextFont *tf;
    struct Screen *s;
    struct Window *w;
    struct ScreenBuffer *sb[2];
    struct MsgPort *dbufports[2];
    BOOL safe[2];
    WORD frame;
    BOOL backDrawn[2]; /* Background drawn? */
    Object *gfx;
    void (*drawback)(struct BitMap *bm, struct RastPort *rp);
};

BOOL initsystem(struct system *sys);
void closesystem(struct system *sys);

extern BOOL initgame(struct system *sys);
extern void playgame(struct system *sys);
extern void closegame(struct system *sys);
