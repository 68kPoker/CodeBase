
#ifndef IFF_H
#define IFF_H

#include <exec/types.h>
#include <datatypes/soundclass.h>

struct soundSample
{
    struct VoiceHeader vhdr;
    BYTE *data;
    LONG size;
};

struct IFFHandle *openIFF(STRPTR name, LONG mode);
void closeIFF(struct IFFHandle *iff);
BOOL scanILBM(struct IFFHandle *iff);
BOOL loadCMAP(struct IFFHandle *iff, struct Screen *s);
struct BitMap *loadBitMap(struct IFFHandle *iff);

BOOL loadSample(STRPTR name, struct soundSample *s);
void freeSample(struct soundSample *s);

#endif /* IFF_H */
