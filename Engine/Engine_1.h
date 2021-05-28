
#include <exec/types.h>

#define WIDTH  20
#define HEIGHT 16

#define TRIGGERS 4
#define IDENTIFIERS 32

enum
{
	F_FLOOR,
	F_WALL,
	F_FLAGSTONE
};

enum
{
	O_NONE,
	O_PLAYER,
	O_BOX
};

struct Position
{
	WORD x, y;
};

/* Player direction */
extern WORD dx, dy;
extern WORD px, py; /* Hero position */
extern WORD placed; /* Placed boxes */
extern ULONG placedMask; /* Placed boxes mask */
extern WORD triggerMask[TRIGGERS]; /* Trigger mask */
extern WORD triggerCount;

extern WORD fields[HEIGHT][WIDTH];
extern WORD objects[HEIGHT][WIDTH];
extern WORD identifiers[HEIGHT][WIDTH];

extern struct Position positions[IDENTIFIERS];

extern WORD idCount;

/* Player movement (game turn) */
extern turn();
