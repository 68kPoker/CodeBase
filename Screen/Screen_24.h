
#ifndef SCREEN_H
#define SCREEN_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#define WIDTH  320
#define HEIGHT 256
#define DEPTH  5

struct Screen *openScreen();
VOID closeScreen( struct Screen *s );

#endif /* SCREEN_H */
