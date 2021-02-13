
/* $Id: Game.h,v 1.1 12/.0/.0 .1:.0:.4 Unknown Exp Locker: Unknown $ */

/*
 * Game.h
 */

#include <exec/types.h>

#include "System.h"
#include "IFF.h"
#include "Blit.h"
#include "Board.h"

#define BOARD_WIDTH  20
#define BOARD_HEIGHT 16

enum
{
	T_FLOOR,
	T_WALL,
	T_BOX,
	T_KEY,
	T_HERO,
	T_SKULL,
	T_DOOR,
	T_CHERRY,
	T_PLACE,
	T_SLIDEUP,
	T_SLIDERIGHT,
	T_SLIDEDOWN,
	T_SLIDELEFT,
	T_COUNT
};

struct EditorBoard
{
	WORD board[BOARD_HEIGHT][BOARD_WIDTH];
};

extern struct Game
{
	LONG (*play)();
	LONG (*init)(struct GUI *gui);
	void (*cleanup)();
	LONG (*handleIDCMP)(struct WindowInfo *wi);
	LONG (*handleAnimFrame)(struct GUI *gui);
	void (*draw)(struct WindowInfo *wi);

	WORD frames;
	WORD prevx, prevy;
	WORD tilex, tiley;
	BOOL paint;

	BOOL ready, start, move[4], stop[4], moved, edit;

	struct ILBMInfo gfx;
	struct EditorBoard board;
	struct Board map; /* Run-time */
} game;
