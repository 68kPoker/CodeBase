
#include <exec/types.h>

/* Open main window */

struct Window *openWindow(struct Screen *s);

/* Animation loop */

BOOL animateLoop(struct Screen *s);

/* Game engine */

BOOL gameEngine(struct Screen *s);
