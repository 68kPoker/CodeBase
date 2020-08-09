
/* $Id$ */

/*
 * Game.h
 */

#include <exec/types.h>

#include "System.h"

extern struct Game
{
	LONG (*play)();
} game;
