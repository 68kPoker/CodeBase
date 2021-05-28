
#include "Engine.h"

/* Player direction */
WORD dx = 0, dy = 0;

WORD px = 1, py = 1; /* Hero position */

WORD placed = 0; /* Placed boxes */

ULONG placedMask = { 0 }; /* Placed boxes mask */

WORD triggerMask[TRIGGERS] = { 0 }; /* Trigger mask */

WORD triggerCount = 0;

WORD fields[HEIGHT][WIDTH] = { 0 };
WORD objects[HEIGHT][WIDTH] = { 0 };
WORD identifiers[HEIGHT][WIDTH] = { 0 };

struct Position
{
	WORD x, y;
} positions[IDENTIFIERS];

WORD idCount = 0;

/* Board field */
WORD *field(WORD x, WORD y)
{
	return(&fields[y][x]);
}

/* Board object */
WORD *object(WORD x, WORD y)
{
	return(&objects[y][x]);
}

/* Field identifier */
WORD *id(WORD x, WORD y)
{
	return(&identifiers[y][x]);
}

/* Player movement (game turn) */
turn()
{
	/* Move player if possible */
	if (checkMove())
	{
		move();
		return(TRUE);
	}
	return(FALSE);
}

move()
{
	WORD *self = object(px, dy);
	WORD *tobject = object(px + dx, py + dy);

	if (*tobject != O_NONE)
	{
		/* Pushable box */
		WORD *target = object(px + dx + dx, py + dy + dy);
		WORD *stone = field(px + dx, py + dy);
		WORD *tid = id(px + dx, py + dy);

		if (*stone == F_FLAGSTONE)
		{
			placed--;
			placedMask &= ~(1L << *tid);
		}

		stone = field(px + dx + dx, py + dy + dy);
		tid = id(px + dx + dx, py + dy + dy);

		*target = *tobject;

		if (*stone == F_FLAGSTONE)
		{
			WORD i;

			placed++;
			placedMask |= 1L << *id;

			for (i = 0; i < triggerCount; i++)
			{
				if ((placedMask & triggerMask[i]) == triggerMask[i])
				{
					WORD j;

					trigger(i);

					for (j = 0; j < idCount; j++)
					{
						if (triggerMask[i] & (1L << j))
						{
							target = object(positions[j].x, positions[j].y);
							stone = field(positions[j].x, positions[j].y);

							*stone = F_FLOOR;
							*target = O_NONE;
						}
					}
					break;
				}
			}
		}
	}

	*tobject = *self;
	*self = O_NONE;

	px += dx;
	py += dy;
}

/* Check if movement is possible */
checkMove()
{
	WORD tfield = *field(px + dx, py + dy);
	WORD tobject = *object(px + dx, py + dy);

	if (tfield == F_WALL)
	{
		return(FALSE);
	}

	if (tobject == O_NONE)
	{
		return(TRUE);
	}

	if (tobject == O_BOX)
	{
		/* Check if pushing is possible */
		return(checkPush());
	}

	return(FALSE);
}

/* Check if box pushing is possible */
checkPush()
{
	WORD tobject = *object(px + dx + dx, py + dy + dy);

	return(tobject == O_NONE);
}
