
#ifndef PICTURE_H
#define PICTURE_H

#include <exec/types.h>

enum
    {
    NOMEM = 1,
    NOFILE,
    NOBMHD,
    NOCMAP,
    NOGFXMEM,
    CMPERR
    };

struct PictureInfo
    {
    struct BitMap *bitmap;
    ULONG *colors;
    };

LONG loadPicture(STRPTR name, struct PictureInfo *pi);
VOID freePicture(struct PictureInfo *pi);

#endif /* PICTURE_H */
