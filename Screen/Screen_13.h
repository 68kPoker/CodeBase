
#include <exec/types.h>

typedef struct
{
	struct TextFont *tf;
	struct Screen *s;
} screen;

BOOL openScreen(screen *s);
void closeScreen(screen *s);
