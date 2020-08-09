
/* $Id$ */

#include <stdio.h>
#include <dos/dos.h>
#include <clib/exec_protos.h>

#include "Game.h"

static LONG play()
{
	struct GUI *gui;

	/* Let's play */
	printf("game.play()\n");

	/* Init system resources */
	if (gui = system.init())
	{
		WORD i;
		for (i = 0; i < 100; i++)
		{
			Wait(SIGBREAKF_CTRL_C|(1L << gui->cop.signal));
		}
		system.cleanup();
	}

	return(RETURN_OK);
}

struct Game game = { play };

int main()
{
	game.play();

	return(RETURN_OK);
}
