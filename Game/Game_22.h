
#include <exec/types.h>

#define BOARD_WIDTH  20
#define BOARD_HEIGHT 16

struct gameTile
    {
    WORD type;
    BOOL selected; /* This tile is selected */
    BOOL redraw; /* Redraw this tile */
    };

struct gameUser
    {
    BOOL paintMode; /* Paint/select the tiles upon mouse move? */
    BOOL selectMode; /* Select or paint tiles? */
    WORD prevX, prevY; /* Previously pointed tile */
    WORD currentType; /* Current tile type */
    WORD cursX, cursY;
    BOOL updateCurs;
    struct gameTile board[BOARD_HEIGHT][BOARD_WIDTH];
    };

extern LONG mouseButtons(struct gameUser *user, UWORD code, WORD mouseX, WORD mouseY);
extern LONG mouseMove(struct gameUser *user, UWORD code, WORD mouseX, WORD mouseY);
extern LONG rawKey(struct gameUser *user, UWORD code, WORD qualifier, WORD mouseX, WORD mouseY);
extern LONG draw(struct gameUser *user, struct RastPort *rp, WORD frame);
