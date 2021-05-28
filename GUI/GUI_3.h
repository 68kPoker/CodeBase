
#include <exec/types.h>

struct Screen *openscreen(struct TextFont **tf);
struct Window *openwindow(struct Screen *s);
BOOL allocbuffers(struct ScreenBuffer *sb[], struct Screen *s);
void freebuffers(struct Screen *s, struct ScreenBuffer *sb[]);
void attachports(struct ScreenBuffer *sb, struct MsgPort *mp[]);
