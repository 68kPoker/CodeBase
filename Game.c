
#include "Game.h"

LONG handleHero(WORD action, struct Game *game, struct Object *obj);
LONG handleBox(WORD action, struct Game *game, struct Object *obj);

LONG (*objectStateHandlers[OT_COUNT])(WORD action, struct Game *game, struct Object *obj) =
{
    handleBox, /* Box */
    handleHero
};

LONG (*fieldStateHandlers[FT_COUNT])(WORD action, struct Game *game, struct Board *board, WORD x, WORD y) =
{
};

LONG handleBox(WORD action, struct Game *game, struct Object *obj)
{
    struct Board *board = game->board;
    WORD x, y;

    if (action == ACTION_LEAVE_FIELD)
    {
        x = obj->state.prevX;
        y = obj->state.prevY;

        /* Check for flagstone */
        struct Field *field = &board->fields[y][x];

        if (field->type == FT_FLAGSTONE)
        {
            /* Release the flagstone */
            game->placedBoxes--;
        }
    }
    else if (action == ACTION_ENTER_FIELD)
    {
        x = obj->x;
        y = obj->y;

        /* Check for flagstone */
        struct Field *field = &board->fields[y][x];

        if (field->type == FT_FLAGSTONE)
        {
            /* Activate the flagstone */
            game->placedBoxes++;
        }
    }
}

LONG handleHero(WORD action, struct Game *game, struct Object *obj)
{

}

LONG moveObject(struct Game *game, struct Object *obj, WORD dx, WORD dy)
{
    struct Board *board = game->board;

    obj->state.prevX = obj->x;
    obj->state.prevY = obj->y;
    obj->x += dx;
    obj->y += dy;

    struct Field *field = &board->fields[obj->state.prevY][obj->state.prevX];
    field->object = NULL;

    field = &board->fields[obj->y][obj->x];
    field->object = obj;

    objectStateHandlers[obj->type](ACTION_LEAVE_FIELD, game, obj);
    objectStateHandlers[obj->type](ACTION_ENTER_FIELD, game, obj);

    obj->state.updated = TRUE; /* Updated position */
}
