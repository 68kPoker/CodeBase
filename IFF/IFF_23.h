
#include <exec/types.h>

struct IFFHandle *openIFF(STRPTR name, LONG mode, BOOL *clip, LONG *err);
WORD countIFF(LONG *chunks);
LONG scanIFF(struct IFFHandle *iff, LONG *props, LONG *colls, LONG *stops);
BYTE *findPropData(struct IFFHandle *iff, LONG type, LONG id);
BYTE *readBODY(struct IFFHandle *iff, LONG *size);
void freeBODY(BYTE *buffer, LONG size);
void closeIFF(struct IFFHandle *iff, BOOL clip);
