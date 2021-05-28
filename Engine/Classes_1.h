
#include <exec/types.h>

#define BOARD_WIDTH  20
#define BOARD_HEIGHT 16

/* Klasyfikacja elementów planszy */
enum Root
{
    FLOOR, /* Puste pole */
    WALL, /* Ôciana lub drzwi itp. */
    OBJECT /* Obiekt. Kaûdy obiekt przechowuje teû informacjë o podîodze. */
};

/* Podîogi */
enum Floors
{
    FLOOR_NORMAL, /* Normalna podîoga */
    FLOOR_FLAGSTONE /* Przycisk */
};

/* Ôciany */
enum Walls
{
    WALL_SOLID, /* Normalna ôciana */
    WALL_DOOR /* Drzwi */
};

/* Obiekty */
enum Objects
{
    OBJECT_BOX, /* Pudîo */
    OBJECT_HERO, /* Bohater */
    OBJECT_SCROLL, /* Zwój tekstu */
    OBJECT_KEY /* Klucz */
};

struct Field
{
    WORD root : 2; /* Typ podstawowy */
    WORD sub  : 3; /* Podtyp */
    WORD floor: 3; /* Podtyp podîogi (gdy root == OBJECT) */
};

struct Board
{
    struct Field board[ BOARD_HEIGHT ][ BOARD_WIDTH ];
};

/* Metody obsîugi poszczególnych klas */

enum Methods
{
    METHOD_ENTER, /* Wejdú na to pole */
    METHOD_LEAVE /* Opuôê to pole */
};

/* Wyniki metod ENTER */

enum
{
    RESULT_OK,
    RESULT_BLOCK,
    RESULT_NONE
};

handleFloor( WORD m );
handleWall( WORD m );
handleObject( WORD m );

handleNormal( WORD m );
handleFlagstone( WORD m );

handleSolid( WORD m );
handleDoor( WORD m );

handleBox( WORD m );
handleHero( WORD m );
handleScroll( WORD m );
handleKey( WORD m);
