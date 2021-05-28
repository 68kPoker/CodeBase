
#include "Ekran.h"
#include "Okna.h"

#include "Plansza.h"

#include <clib/exec_protos.h>

void setPos( position *p, WORD x, WORD y )
{
	p->x = x;
	p->y = y;
}

void initBoard( board *b, boardInfo *bi )
{
	WORD x, y;
	field *f;

	/* Ustawiamy bohatera */
	setPos(&bi->heroPos, 1, 1);

	for (y = 0; y < HEIGHT; y++)
	{
		for (x = 0; x < WIDTH; x++)
		{
			f = &b->board[y][x];

			if (x == 0 || x == (WIDTH - 1) || y == 0 || y == (HEIGHT - 1))
				f->floor.type = WALL;
			else
				f->floor.type = FLOOR;

			f->floor.ID = f->object.ID = 0;

			if (x == bi->heroPos.x && y == bi->heroPos.y)
				f->object.type = HERO;
			else
				f->object.type = NONE;
		}
	}

	bi->updateCount = 0;
	bi->placed = 0;
}

void placeHero( board *b, boardInfo *bi, WORD x, WORD y )
{
	setPos(&bi->heroPos, x, y);

	b->board[y][x].object.type = HERO;
}

BOOL scanBoard( board *b, boardInfo *bi )
{
	WORD x, y;
	field *f;
	WORD fields = 0;

	bi->boxes = 0;

	for (y = 0; y < HEIGHT; y++)
	{
		for (x = 0; x < WIDTH; x++)
		{
			f = &b->board[y][x];

			if (f->object.type == BOX)
			{
				bi->boxes++;
			}
			else if (f->floor.type == FLAGSTONE)
			{
				fields++;
			}
		}
	}
	return(bi->boxes > 0 && bi->boxes == fields);
}

/* Dodaj pole do odswiezenia */
void addUpdate( board *b, boardInfo *bi, WORD x, WORD y )
{
	WORD i = bi->updateCount;

	if (i < MAX_UPDATE)
	{
		setPos(&bi->update[i++], x, y);
		bi->updateCount = i;
	}
}

BOOL pushBox( board *b, boardInfo *bi, WORD dx, WORD dy )
{
	WORD x = bi->heroPos.x + dx, y = bi->heroPos.y + dy;

	field *box = &b->board[y][x], *t = &b->board[y + dy][x + dx];

	if (t->floor.type == WALL || t->object.type != NONE)
	{
		return(FALSE);
	}

	addUpdate(b, bi, x + dx, y + dy);

	t->object = box->object;

	if (t->floor.type == FLAGSTONE)
	{
		bi->placed++;
	}

	if (box->floor.type == FLAGSTONE)
	{
		bi->placed--;
	}

	return(TRUE);
}

BOOL moveHero( board *b, boardInfo *bi, WORD dx, WORD dy )
{
	WORD x = bi->heroPos.x, y = bi->heroPos.y;

	field *hero = &b->board[y][x], *t = &b->board[y + dy][x + dx];

	bi->updateCount = 0;

	if (t->floor.type == WALL)
	{
		return(FALSE);
	}

	if (t->object.type != NONE && (t->object.type == BOX && !pushBox(b, bi, dx, dy)))
	{
		return(FALSE);
	}

	addUpdate(b, bi, x, y);
	addUpdate(b, bi, x + dx, y + dy);

	setPos(&bi->heroPos, x + dx, y + dy);

	t->object = hero->object;
	hero->object.type = 0;
	hero->object.ID = 0;

	return(TRUE);
}

int main(void)
{
	static board b;
	static boardInfo bi;
	static screen s;
	static window backw;

	initBoard(&b, &bi);
	placeHero(&b, &bi, 3, 3);

	b.board[1][3].floor.type = FLAGSTONE;
	b.board[2][3].object.type = BOX;

	if (openScreen(&s))
	{
		if (openBackWindow(&s, &backw))
		{
			WaitPort(backw.w->UserPort);
			closeWindow(&backw);
		}
		closeScreen(&s);
	}

	return(0);
}
