
#ifndef IFF_H
#define IFF_H

#include <exec/types.h>

typedef struct infoIFF
{
    struct IFFHandle *iff;
    BOOL isClip;
    LONG err;
    BYTE *buffer, *current; /* For buffered reading */
    LONG read; /* Left read bytes */
} IFF;

BOOL prepIFF(IFF *iff);
VOID freeIFF(IFF *iff);
BOOL openIFF(IFF *iff, STRPTR name);
VOID closeIFF(IFF *iff);
struct ContextNode *enterIFF(IFF *iff);
struct BitMapHeader *scanILBM(IFF *iff); /* Scan ILBM chunks */
struct VoiceHeader *scan8SVX(IFF *iff);
BOOL readIFF(IFF *iff, BYTE *buffer, LONG size);

#endif /* IFF_H */
