
#include <exec/lists.h>

#include "Fields.h"
#include "Objects.h"

enum
{
    ACTION_LEAVE_FIELD, /* Leave field */
    ACTION_ENTER_FIELD, /* Enter field */
    ACTION_COUNT
};

struct Game
{
    struct Board *board; /* Active board */
    struct Board *backupBoard; /* For game-restart */
    struct List objectList; /* List of objects */
    WORD boxes, placedBoxes; /* Amount of boxes */
};

/* Field state handlers array */
extern LONG (*fieldStateHandlers[FT_COUNT])(WORD action, struct Game *game, struct Board *board, WORD x, WORD y);
extern LONG (*objectStateHandlers[OT_COUNT])(WORD action, struct Game *game, struct Object *obj);

extern LONG moveObject(struct Game *game, struct Object *obj, WORD dx, WORD dy);
