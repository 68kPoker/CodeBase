
/* Game configuration */

#ifndef CONFIG_H
#define CONFIG_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef GRAPHICS_MODEID_H
#include <graphics/modeid.h>
#endif

#define DEF_MODEID  LORES_KEY
#define DEF_WIDTH   304
#define DEF_HEIGHT  200
#define DEF_DEPTH   5
#define DEF_BLITTER FALSE
#define DEF_CHUNKY  CHUNKY_OFF
#define DEF_PUB     TRUE
#define DEF_PUBNAME NULL

enum chunkyModes
{
    CHUNKY_OFF,
    CHUNKY_GRAPHICS,
    CHUNKY_CUSTOM
};

struct config
{
    /* Video options */
    ULONG modeID; /* Display mode */
    UWORD width, height; /* Display size */
    UBYTE depth; /* Color depth */
    BOOL  doubleBuffering; /* Use double-buffered display? */
    BOOL  pub; /* Use public screen? */
    STRPTR pubName; /* Public screen name */

    /* Graphics options */
    BOOL  blitter; /* Allow to use Blitter directly? */
    WORD  chunkyMode; /* Enable chunky? */
};

/* Read configuration from given file */
BOOL getConfig(struct config *, STRPTR name);

/* Write configuration to given file */
BOOL setConfig(struct config *, STRPTR name);

#endif /* CONFIG_H */
