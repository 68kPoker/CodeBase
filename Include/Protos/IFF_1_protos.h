
#include "IFF.h"

#include <exec/types.h>

/*==================== Pliki ====================*/

GFX *loadGraphics(STRPTR name);
SFX *loadSound(STRPTR name);
LEV *loadLevel(STRPTR name);

BOOL saveLevel(LEV *lev, STRPTR name);

void freeGraphics(GFX *);
void freeSound(SFX *);
void freeLevel(LEV *);
