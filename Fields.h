
#ifndef FIELDS_H
#define FIELDS_H

/* Board fields form a rectangular array */

#include <exec/types.h>

#define BOARD_WIDTH  20
#define BOARD_HEIGHT 16

/* Field main types */
enum
{
    FT_BACK, /* Background */
    FT_FLOOR, /* Regular floor */
    FT_WALL, /* Regular wall */
    FT_LOW_WALL, /* Lowered wall */
    FT_FLAGSTONE, /* A flagstone (subType determines a kind) */
    FT_COUNT
};

/* Field has type, sub-types, identifier, state */
struct Field
{
    WORD type; /* Main type */
    UBYTE subType, subType2; /* Sub-types */
    WORD identifier; /* To identify field across others */
    struct Object *object; /* Pointer to object on this field */
    struct FieldState
    {
        WORD state;
    } state;
};

struct Board
{
    struct Field fields[BOARD_HEIGHT][BOARD_WIDTH];
};

#endif /* FIELDS_H */
