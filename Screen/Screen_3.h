
#include <exec/types.h>

struct screenInfo
{
	struct Screen *s;
	struct BitMap *bm[2];
	WORD signal;
	struct DBufInfo *dbi;
	BOOL safe;
};

struct Screen *openScreen(ULONG tag1, ...);
WORD addCopper(struct ViewPort *vp);
void remCopper(void);
BOOL addUCL(struct ViewPort *vp);
struct DBufInfo *getDBufInfo(struct ViewPort *vp, BOOL *safe);
void freeDBufInfo(struct DBufInfo *dbi, BOOL safe);
void safeToDraw(struct DBufInfo *dbi, BOOL *safe);
void changeVPBitMap(struct ViewPort *vp, struct DBufInfo *dbi, struct BitMap *bm, BOOL *safe);

struct Screen *openDBufScreen(struct BitMap **bm, WORD *signal, struct DBufInfo **dbi, BOOL *safe);
void closeDBufScreen(struct Screen *s, struct BitMap **bm, struct DBufInfo *dbi, BOOL safe);

struct Screen *openScreenInfo(struct screenInfo *si);
void closeScreenInfo(struct screenInfo *si);
