
/* $Id$ */

#ifndef WINDOWS_H
#define WINDOWS_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif /* EXEC_TYPES_H */

#ifndef GRAPHICS_GFX_H
#include <graphics/gfx.h>
#endif /* GRAPHICS_GFX_H */

#define ESC_KEY (0x45)

typedef LONG( *handleWindow )( struct windowData *wd, struct windowMsg *msg );

enum
{
    WINDOW_MSG_INIT,
    WINDOW_MSG_USER,
    WINDOW_MSG_DRAW,
    WINDOW_MSG_ANIMATE
};

enum
{
    SIGNAL_SRC_COPPER,
    SIGNAL_SRC_SAFE,
    SIGNAL_SRC_USER,
    SIGNAL_SRC_COUNT
};

struct windowMsg
{
    WORD kind;
    struct IntuiMessage *imsg;
};

enum
{
    WINDOW_KIND_BACKDROP,
    WINDOW_KIND_REQ
};

struct windowData
{
    struct Window *window;
    struct Rectangle bounds;
    handleWindow handle;
    BOOL close;
    struct Region *update[ 2 ];
    LONG flags;
};

struct windowData *windowOpen( struct screenData *sd, WORD kind, struct Rectangle *rect, handleWindow handle );
VOID windowUpdate( struct screenData *sd, struct windowData *wd );
VOID windowClose( struct windowData *wd );

#endif /* WINDOWS_H */
