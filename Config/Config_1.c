
#include "Config.h"

/* Read configuration from given file */
BOOL getConfig(struct config *c, STRPTR name)
{
    c->modeID     = DEF_MODEID;
    c->width      = DEF_WIDTH;
    c->height     = DEF_HEIGHT;
    c->depth      = DEF_DEPTH;
    c->pub        = DEF_PUB;
    c->pubName    = DEF_PUBNAME;
    c->blitter    = DEF_BLITTER;
    c->chunkyMode = DEF_CHUNKY;

    return(TRUE);
}

/* Write configuration to given file */
BOOL setConfig(struct config *c, STRPTR name)
{
    return(FALSE);
}
