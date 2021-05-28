
#ifndef SCREEN_H
#define SCREEN_H

#include <exec/interrupts.h>
#include <graphics/gfx.h>
#include <graphics/text.h>

#define SCREEN_BUFFERS  ( 2 )
#define COPPER_PRIORITY ( 0 )
#define COPPER_COMMANDS ( 3 )

struct screenUser
{
    struct TextFont     *font;                      /* Game font      */
    struct BitMap       *bitmaps[ SCREEN_BUFFERS ]; /* Screen bitmaps */
    struct Screen       *screen;
    struct ScreenBuffer *buffers[ SCREEN_BUFFERS ];
    struct MsgPort      *safePort;
    BOOL                safeToWrite;
    UWORD               frame;
    struct Interrupt    copper;
    struct Task         *task;
    WORD                copperSignal;               /* Send by Copper */
};

struct screenParam
{
    struct Rectangle    displayClip; /* Obtain from DimensionInfo */
    ULONG               modeID; /* Obtain via BestModeIDA or ASL req */
    ULONG               *paletteRGB; /* Obtain from picture file */
    struct BitMap       *bitmaps[ SCREEN_BUFFERS ]; /* Screen bitmaps */
    struct TextAttr     textAttr;
    STRPTR              title;
};

BOOL initScreen( struct screenUser *su, struct screenParam *sp );
VOID safeToWrite( struct screenUser *su );
VOID changeScreen( struct screenUser *su );
VOID freeScreen( struct screenUser *su );

#endif /* SCREEN_H */
