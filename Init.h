
#include <exec/interrupts.h>

#include "Screen.h"
#include "Window.h"
#include "Audio.h"
#include "IFF.h"

enum
{
    SAMPLE_DIG,
    SAMPLE_BOX,
    SAMPLES
};

struct gameInit
{
    struct BitMap *bm[2]; /* Screen bitmaps */
    struct TextFont *tf;
    struct Screen *s;
    struct Interrupt copis; /* Copper interrupt */
    struct copperData copdata;

    struct BitMap *gfx;
    struct IOAudio *ioa;
    struct soundSample samples[SAMPLES];

    struct windowInfo wi;
    struct menuInfo mi;
    struct Window *w;
};

BOOL openLibs(void);
BOOL initGameScreen(struct gameInit *gi);
BOOL initGameGfx(struct gameInit *gi);
BOOL initGameSfx(struct gameInit *gi);
BOOL initGameWindows(struct gameInit *gi);
BOOL initGame(struct gameInit *gi);

void closeLibs(void);
void freeGameScreen(struct gameInit *gi);
void freeGameGfx(struct gameInit *gi);
void freeGameSfx(struct gameInit *gi);
void freeGameWindows(struct gameInit *gi);
void freeGame(struct gameInit *gi);
