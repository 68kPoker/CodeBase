
/*
 * Magazyn
 * Plansza do gry
 */

#include <exec/types.h>

/* Typ + opc. identyfikator */

typedef struct
{
	UBYTE type, ID;
} typeID;

/* Pole dwuwarstwowe (podloga + obiekt) */

typedef struct
{
	typeID floor, object;
} field;

/* Wyliczenie typow */

enum floor
{
	FLOOR,
	WALL,
	FLAGSTONE
};

enum object
{
	NONE,
	HERO,
	BOX
};

/* Tablica dwuwymiarowa pol */

#define HEIGHT	20
#define WIDTH	16
#define MAX_UPDATE	6

typedef struct
{
	field board[HEIGHT][WIDTH];
} board;

/* Dodatkowe informacje */

typedef struct
{
	WORD x, y;
} position;

typedef struct
{
	WORD boxes, placed; /* Liczba skrzyn */
	position heroPos; /* Polozenie bohatera */
	position update[MAX_UPDATE]; /* Kafle do odrysowania */
	WORD updateCount; /* Liczba kafli */
} boardInfo;

/* Funkcje */

/* Zainicjuj pusta plansze */

void initBoard( board *b, boardInfo *bi );

/* Zmien polozenie bohatera */

void placeHero( board *b, boardInfo *bi, WORD x, WORD y );

/* Przeskanuj plansze */

BOOL scanBoard( board *b, boardInfo *bi );
