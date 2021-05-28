
#include <exec/types.h>
#include <datatypes/soundclass.h>

struct sound
{
    struct VoiceHeader vhdr;
    BYTE *data;
    LONG size;
};

struct IFFHandle *open_iff(STRPTR name, LONG mode);
void close_iff(struct IFFHandle *iff);
BOOL scan_ilbm(struct IFFHandle *iff);
BOOL load_cmap(struct IFFHandle *iff, struct Screen *s);
struct BitMap *load_bitmap(struct IFFHandle *iff);

BOOL load_sample(STRPTR name, struct sound *s);
void free_sample(struct sound *s);
