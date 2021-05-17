
#ifndef WINDOWS_H
#define WINDOWS_H

#include <exec/types.h>

#define ESC_KEY 0x45

enum
{
	SIGBIT_USERPORT,
	SIGBIT_COPPER,
	SIGBIT_SAFE,
	SIGBITS
};	

struct Window *openWindow(struct Screen *s);
LONG mainLoop(struct Window *w, struct screenInfo *si);

#endif /* WINDOWS_H */
