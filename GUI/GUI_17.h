
#include <exec/types.h>

#include "CInput.h"

typedef struct CScreen {
    struct TextFont *font;
    struct Screen *s;
    struct ScreenBuffer *sb[2];
    UWORD frame;
    CSafeToDraw safe;
    CCopper cop;
    struct Window *backw;
    CIDCMP idcmp;
} CScreen;

BOOL openScreen( CInputs inputs, CScreen *s, struct TagItem *more, CInputHandler safeHandler, CInputHandler copHandler );
void closeScreen( CScreen *s );

BOOL openFont( CScreen *s );
void closeFont( CScreen *s );

BOOL openBackWindow( CInputs inputs, CScreen *s, CInputHandler handler );
void closeBackWindow( CScreen *s );
