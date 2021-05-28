
#ifndef MAINLOOP_PROTOS_H
#define MAINLOOP_PROTOS_H

#include <exec/types.h>

#include "System.h"

/*==================== Pëtla gîówna ====================*/

struct Window *menu(SYSINFO *data, WORD gid, WORD x, WORD y);

SYSINFO *initSys(SYSDATA *sysdata);
void freeData(SYSINFO *);

void mainLoop(SYSINFO *);

#endif /* MAINLOOP_PROTOS_H */
