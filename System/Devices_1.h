
#include <exec/types.h>

struct IOStdReq *openGameport(UBYTE unit, WORD timeOut, struct InputEvent *ie);
void readEvent(struct IOStdReq *io, struct InputEvent *ie);

void closeGameport(struct IOStdReq *io);
