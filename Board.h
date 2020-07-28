
/* Run-time board */

#ifndef BOARD_H
#define BOARD_H

#include <exec/types.h>

#define BOARD_WIDTH  20
#define BOARD_HEIGHT 16

typedef struct
{
	UWORD type;

	/* Additional info depending on main type */
	union floorInfo
	{
		UWORD doorType; /* Additional type */
		UWORD flagstoneType;
	} info;
	UWORD id; /* Special identifier */
} FLOOR;

typedef struct
{
	UWORD type;

	union objectInfo
	{
		struct itemInfo
		{
			UWORD itemType;
			union itemMoreInfo
			{
				UWORD keyType;
			} info;
		} item;
		UWORD boxType;
	} info;
	UWORD id;
} OBJECT;

typedef struct Field
{
	FLOOR  floor;
	OBJECT object;
} FIELD;

struct Board
{
	FIELD array[BOARD_HEIGHT][BOARD_WIDTH];
};

void printFieldInfo(FIELD *f);

#endif /* BOARD_H */
