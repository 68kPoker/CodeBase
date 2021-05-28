
/* Object */

#include <exec/types.h>

enum types
{
    HERO
};

enum dirs /* Basic dirs */
{
    DOWN,
    LEFT,
    UP,
    RIGHT,
    DIRS
};

enum states
{
    STOP,
    MOVE
};

struct object
{
    UWORD type;
    UWORD x, y; /* Position */
    UWORD offset : 4, dir : 4; /* Precise position */
    UWORD reqdir : 2, nextdir : 2; /* Requested dir */
    UWORD state  : 6;
};

BOOL objectRotate(struct object *o); /* Rotate if needed */
void objectMove(struct object *o, WORD dir); /* Move object */
void objectStep(struct object *o); /* One step of movement */
