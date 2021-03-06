
/* Definicje gry Magazyn */

#include <exec/types.h>

#define WIDTH   20
#define HEIGHT  16

/* Kierunki */
enum { UP, RIGHT_UP, RIGHT, RIGHT_DOWN, DOWN, LEFT_DOWN, LEFT, LEFT_UP };

/* Kafelki */
enum { FLOOR, WALL, FLAGSTONE, MUD, FILLED_MUD };

/* Obiekty */
enum { NOOBJ, BOX, HERO, KEY, DOOR };

/* Pole */
typedef struct Tile {
    UWORD floor, object;
} TILE;

/* Plansza */
typedef struct Board {
    TILE tiles[ HEIGHT ][ WIDTH ];
    WORD heroX, heroY;
    WORD dir;
    WORD boxes, placed, keys;
} BOARD;

/* Dodatkowe informacje */
typedef struct BoardInfo {
    BOARD *board;
    WORD score;
} BOARD_INFO;

typedef struct EditorInfo {
    BOARD_INFO *info;
    WORD cursX, cursY;
    TILE curTile;
    BOOL changeFloor; /* Czy modyfikowa� pod�og�? */
} EDITOR;

/* Zainicjuj plansz� */
void boardClear(BOARD *b);

/* Zmie� po�o�enie bohatera */
void heroPlace(BOARD *b, WORD newX, WORD newY);
void putHero(BOARD *b, TILE *t);

/* Umie�� pud�o */
void putBox(BOARD *b, TILE *t);

/* Usu� pud�o */
void remBox(BOARD *b, TILE *t);

void putObject(BOARD *b, TILE *t, WORD object);
void remObject(BOARD *b, TILE *t);
void putFloor(BOARD *b, TILE *t, WORD floor);

void putWall(TILE *t);
void remWall(TILE *t);

/* Wype�nij bagno */
void fillMud(TILE *t);

BOOL pushBox(BOARD *b, TILE *t, WORD dir);

BOOL moveHero(BOARD *b, WORD dir);

/* Przygotuj edytor */
void editorSetup(EDITOR *ed);
