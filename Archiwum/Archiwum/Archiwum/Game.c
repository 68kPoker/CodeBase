
/* $Id: Game.c,v 1.1 12/.0/.0 .1:.0:.3 Unknown Exp Locker: Unknown $ */

#include <stdio.h>
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>

#include "Game.h"

static LONG handleMenu(struct WindowInfo *wi);
static LONG handleIntro(struct WindowInfo *wi);

static BOOL convertMap()
{
	WORD x, y;

	for (y = 0; y < BOARD_HEIGHT; y++)
	{
		for (x = 0; x < BOARD_WIDTH; x++)
		{
			TILE *tile = &game.map.tileBoard[y][x];

			tile->floor.type = FT_NORMAL;
			tile->object.type = OT_NONE;

			switch (game.board.board[y][x])
			{
				case T_FLOOR:
					break;
				case T_WALL:
					tile->object.type = OT_WALL;
					break;
				case T_BOX:
					tile->object.type = OT_BOX;
					game.map.boxesAll++;
					break;
				case T_KEY:
					tile->object.type = OT_KEY;
					break;
				case T_DOOR:
					tile->object.type = OT_DOOR;
					break;
				case T_HERO:
					tile->object.type = OT_PLAYER;
					game.map.playerInfo.pos.x = x;
					game.map.playerInfo.pos.y = y;
					break;
				case T_PLACE:
					tile->floor.type = FT_FLAGSTONE;
					break;
				case T_SLIDELEFT:
					tile->floor.type = FT_SLIDER;
					tile->floor.info.direction = SD_LEFT;
					break;
				case T_SLIDERIGHT:
					tile->floor.type = FT_SLIDER;
					tile->floor.info.direction = SD_RIGHT;
					break;
				case T_SLIDEUP:
					tile->floor.type = FT_SLIDER;
					tile->floor.info.direction = SD_UP;
					break;
				case T_SLIDEDOWN:
					tile->floor.type = FT_SLIDER;
					tile->floor.info.direction = SD_DOWN;
					break;
			}
		}
	}
	return(TRUE);
}

static LONG getTileGfx(TILE *tile)
{
	WORD gfx;

	if (tile->object.type != OT_NONE)
	{
		switch (tile->object.type)
		{
			case OT_WALL:
				return(T_WALL);
			case OT_BOX:
				return(T_BOX);
			case OT_PLAYER:
				return(T_HERO);
			case OT_KEY:
				return(T_KEY);
		}
	}
	else
	{
		switch (tile->floor.type)
		{
			case FT_NORMAL:
				return(T_FLOOR);
			case FT_FLAGSTONE:
				return(T_PLACE);
			case FT_SLIDER:
				switch (tile->floor.info.direction)
				{
					case SD_LEFT:
						return(T_SLIDELEFT);
					case SD_RIGHT:
						return(T_SLIDERIGHT);
					case SD_UP:
						return(T_SLIDEUP);
					case SD_DOWN:
						return(T_SLIDEDOWN);
				}
				break;
		}
	}
	return(T_SKULL);
}

/* Init game resources */
static LONG init(struct GUI *gui)
{
	game.gfx.cm = gui->screen->ViewPort.ColorMap;
	if (readILBM(&game.gfx, "Data/Warehouse.iff"))
	{
		struct BitMap *bm;

		if (bm = AllocBitMap(320, 256, 5, BMF_INTERLEAVED, NULL))
		{
			struct Window *intro;
			struct WindowInfo *introwi;

			BltBitMap(game.gfx.bm, 0, 0, bm, 0, 0, 320, 256, 0xc0, 0xff, NULL);
			WaitBlit();
			FreeBitMap(game.gfx.bm);
			game.gfx.bm = bm;

			MakeScreen(gui->screen);
			RethinkDisplay();

			clearBoard(&game.map);

			intro = gui->windows->open(2);
			introwi = (struct WindowInfo *)intro->UserData;

			introwi->handleIDCMP = handleIntro;

			return(TRUE);
		}
		FreeBitMap(game.gfx.bm);
	}
	return(FALSE);
}

static void draw(struct WindowInfo *wi)
{
	struct RastPort *rp = wi->win->RPort;
	WORD x, y;

	for (y = 0; y < BOARD_HEIGHT; y++)
	{
		for (x = 0; x < BOARD_WIDTH; x++)
		{
			if (game.map.tileBoard[y][x].object.moved)
			{
				WORD gfx = getTileGfx(&game.map.tileBoard[y][x]);
				WORD iconx = gfx, icony = 1;

				game.board.board[y][x] = gfx;

				if (gfx == T_HERO)
				{
					WORD dir = 0;

					if (game.move[0])
						dir = 1;
					else if (game.move[1])
						dir = 0;
					else if (game.move[2])
						dir = 2;
					else if (game.move[3])
						dir = 3;

					iconx = dir;
					icony = 5;
				}
				else if (gfx == T_BOX)
				{
					if (game.map.tileBoard[y][x].floor.type == FT_FLAGSTONE)
					{
						iconx = 4;
						icony = 5;
					}
				}
				BltBitMapRastPort(game.gfx.bm, iconx << 4, icony << 4, rp, x << 4, y << 4, 16, 16, 0xc0);
			}
		}
	}
	game.ready = TRUE;
}

/* Cleanup game resources */
static void cleanup()
{
	FreeBitMap(game.gfx.bm);
}

static void drawCursor(struct RastPort *rp, WORD x, WORD y)
{
	SetDrMd(rp, JAM2);
	SetAPen(rp, 19);
	Move(rp, x, y);
	Draw(rp, x + 15, y);
	Draw(rp, x + 15, y + 15);
	Draw(rp, x, y + 15);
	Draw(rp, x, y + 1);
}

static void removeCursor(struct RastPort *rp, WORD x, WORD y)
{
	BltBitMapRastPort(game.gfx.bm, game.board.board[y >> 4][x >> 4] << 4, 1 << 4, rp, x, y, 16, 16, 0xc0);
}

static LONG handleIDCMP(struct WindowInfo *wi)
{
	struct IntuiMessage *msg;

	while (msg = (struct IntuiMessage *)GetMsg(wi->win->UserPort))
	{
		LONG class = msg->Class;
		WORD code = msg->Code;
		WORD mousex = msg->MouseX;
		WORD mousey = msg->MouseY;

		ReplyMsg((struct Message *)msg);

		if (class == IDCMP_RAWKEY)
		{
			/* printf("Code = $%x\n", code); */
			if (code == 0x45)
			{
				system.done = TRUE;
			}
			else if (code == 0x50)
			{
				struct Window *intro;
				struct WindowInfo *introwi;

				if (wi->gui->windows->win[2] == NULL)
				{
					intro = wi->gui->windows->open(2);
					introwi = (struct WindowInfo *)intro->UserData;

					introwi->handleIDCMP = handleIntro;

					BltBitMapRastPort(game.gfx.bm, 0, 96, intro->RPort, 0, 0, 80, 80, 0xc0);
				}
			}

			if (game.start)
			{
				if (code == 0x4C)
				{
					game.move[0] = TRUE;
				}
				else if (code == 0x4D)
				{
					game.move[1] = TRUE;
				}
				else if (code == 0x4E)
				{
					game.move[2] = TRUE;
				}
				else if (code == 0x4F)
				{
					game.move[3] = TRUE;
				}
				else if (code == (0x4C | IECODE_UP_PREFIX))
				{
					game.stop[0] = TRUE;
				}
				else if (code == (0x4D | IECODE_UP_PREFIX))
				{
					game.stop[1] = TRUE;
				}
				else if (code == (0x4E | IECODE_UP_PREFIX))
				{
					game.stop[2] = TRUE;
				}
				else if (code == (0x4F | IECODE_UP_PREFIX))
				{
					game.stop[3] = TRUE;
				}
			}
		}
		else if (class == IDCMP_MOUSEBUTTONS)
		{
			if (game.edit)
			{
				if (code == IECODE_LBUTTON)
				{
					struct RastPort *rp = wi->win->RPort;
					WORD tile = game.tilex;

					game.board.board[mousey >> 4][mousex >> 4] = tile;

					game.paint = TRUE;

					BltBitMapRastPort(game.gfx.bm, tile << 4, game.tiley << 4, rp, mousex & 0xfff0, mousey & 0xfff0, 16, 16, 0xc0);

					drawCursor(rp, mousex & 0xfff0, mousey & 0xfff0);
				}
				else if (code == (IECODE_LBUTTON|IECODE_UP_PREFIX))
				{
					game.paint = FALSE;
				}
				else if (code == IECODE_RBUTTON)
				{
					if (wi->gui->windows->win[1] == NULL)
					{
						struct Window *menu;
						if (menu = wi->gui->windows->open(1))
						{
							struct WindowInfo *menuinfo = (struct WindowInfo *)menu->UserData;

							menuinfo->handleIDCMP = handleMenu;
							bltRastPort(game.gfx.bm, 0, 16, NULL, 0, 0, NULL, menu->RPort, 0, 0, 320, 16, FALSE);
							drawCursor(menu->RPort, game.tilex << 4, 0);
						}
					}
				}
			}
		}
		else if (class == IDCMP_MOUSEMOVE)
		{
			if (game.edit)
			{
				mousex >>= 4;
				mousey >>= 4;

				if (mousex != game.prevx || mousey != game.prevy)
				{
					struct RastPort *rp = wi->win->RPort;

					if (game.paint)
					{
						WORD tile = game.tilex;
						game.board.board[mousey][mousex] = tile;
						BltBitMapRastPort(game.gfx.bm, tile << 4, game.tiley << 4, rp, mousex << 4, mousey << 4, 16, 16, 0xc0);
					}
					removeCursor(rp, game.prevx << 4, game.prevy << 4);
					drawCursor(rp, mousex << 4, mousey << 4);

					game.prevx = mousex;
					game.prevy = mousey;
				}
			}
		}
	}
	return(0);
}

static LONG handleMenu(struct WindowInfo *wi)
{
	struct IntuiMessage *msg;

	while (msg = (struct IntuiMessage *)GetMsg(wi->win->UserPort))
	{
		LONG class = msg->Class;
		WORD code = msg->Code;
		WORD mousex = msg->MouseX;
		WORD mousey = msg->MouseY;

		ReplyMsg((struct Message *)msg);

		if (class == IDCMP_MOUSEBUTTONS)
		{
			if (code == IECODE_LBUTTON || code == IECODE_RBUTTON)
			{
				if (code == IECODE_LBUTTON)
				{
					game.tilex = mousex >> 4;
				}
				wi->gui->windows->close(1);
			}
			return(0);
		}
	}

	return(0);
}

static LONG handleIntro(struct WindowInfo *wi)
{
	struct IntuiMessage *msg;

	while (msg = (struct IntuiMessage *)GetMsg(wi->win->UserPort))
	{
		LONG class = msg->Class;
		WORD code = msg->Code;
		WORD mousex = msg->MouseX;
		WORD mousey = msg->MouseY;

		ReplyMsg((struct Message *)msg);

		if (class == IDCMP_MOUSEBUTTONS)
		{
			if (code == IECODE_LBUTTON || code == IECODE_RBUTTON)
			{
				if (code == IECODE_LBUTTON)
				{
					if (mousey < 14)
					{
					}
					else if (mousey < 34)
					{
						/* Editor */
						game.edit = TRUE;
						game.start = FALSE;
					}
					else if (mousey < 54)
					{
						/* Gra */
						convertMap();
						game.frames = 0;
						game.start = TRUE;
						game.edit = FALSE;
						removeCursor(wi->gui->windows->win[0]->RPort, game.prevx << 4, game.prevy << 4);
					}
					else
					{
						system.done	= TRUE;
					}
				}
				wi->gui->windows->close(2);
			}
			return(0);
		}
	}

	return(0);
}

static LONG handleAnimFrame(struct GUI *gui)
{
	static BOOL drawn = FALSE;

	if (!drawn && game.ready)
	{
		WaitBlit();
		ChangeVPBitMap(&gui->screen->ViewPort, gui->bitmaps[gui->frame], gui->dbi);

		gui->screen->RastPort.BitMap = gui->bitmaps[gui->frame];

		gui->safeToWrite = FALSE;
		gui->frame ^= 1;
		game.ready = FALSE;

		drawn = TRUE;
	}

	if (game.start)
	{
		if (game.frames < 6)
			game.frames++;
		else
		{
			WORD i;
			struct Window *win = gui->windows->win[0];
			struct WindowInfo *wi = (struct WindowInfo *)win->UserData;

			if (game.moved)
			{
				for (i = 0; i < 4; i++)
					if (game.stop[i])
					{
						game.move[i] = FALSE;
						game.stop[i] = FALSE;
					}
			}
			game.map.dir_x = 0;
			game.map.dir_y = 0;

			if (game.move[0])
			{
				game.map.dir_x = 0;
				game.map.dir_y = -1;
			}
			else if (game.move[1])
			{
				game.map.dir_x = 0;
				game.map.dir_y = 1;
			}
			else if (game.move[2])
			{
				game.map.dir_x = 1;
				game.map.dir_y = 0;
			}
			else if (game.move[3])
			{
				game.map.dir_x = -1;
				game.map.dir_y = 0;
			}

			game.frames = 0;
			animateBoard(&game.map);


			if (game.moved = (game.map.dir_x != 0 || game.map.dir_y != 0))
			{
				game.map.tileBoard[game.map.playerInfo.pos.y][game.map.playerInfo.pos.x].object.moved = TRUE;
			}

			game.draw(wi);

			for (i = 0; i < 4; i++)
				if (game.stop[i])
				{
					game.move[i] = FALSE;
					game.stop[i] = FALSE;
				}
		}
	}

	return(0);
}

static LONG play()
{
	struct GUI *gui;

	/* Let's play */
	/* printf("game.play()\n"); */

	/* Init system resources */
	if (gui = system.init())
	{
		/* Init game resources */
		if (game.init(gui))
		{
			/* Setup handlers */
			struct Windows *windows = gui->windows;
			struct Window *bdw = windows->win[0];
			struct WindowInfo *wi = (struct WindowInfo *)bdw->UserData;
			struct RastPort *rp = bdw->RPort;
			struct Window *intro;
			struct WindowInfo *introwi;

			rp->BitMap = gui->bitmaps[gui->frame];

			game.draw(wi);

			wi->handleIDCMP = game.handleIDCMP;

			system.handleAnimFrame = game.handleAnimFrame;

			/* drawCursor(rp, game.prevx << 4, game.prevy << 4); */

			game.tilex = 1;
			game.tiley = 1;

			intro = gui->windows->win[2];
			introwi = (struct WindowInfo *)intro->UserData;
			intro->RPort->BitMap = gui->bitmaps[gui->frame];

			BltBitMapRastPort(game.gfx.bm, 0, 96, intro->RPort, 0, 0, 80, 80, 0xc0);

			/* Enter loop */
			system.loop();

			/* printf("Frames = %d\n", game.frames); */
			game.cleanup();
		}
		system.cleanup();
	}

	return(RETURN_OK);
}

struct Game game = { play, init, cleanup, handleIDCMP, handleAnimFrame, draw };

int main()
{
	game.play();

	return(RETURN_OK);
}
