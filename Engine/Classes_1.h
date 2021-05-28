
#include <exec/types.h>

#define BOARD_WIDTH  20
#define BOARD_HEIGHT 16

/* Klasyfikacja element�w planszy */
enum Root
{
    FLOOR, /* Puste pole */
    WALL, /* �ciana lub drzwi itp. */
    OBJECT /* Obiekt. Ka�dy obiekt przechowuje te� informacj� o pod�odze. */
};

/* Pod�ogi */
enum Floors
{
    FLOOR_NORMAL, /* Normalna pod�oga */
    FLOOR_FLAGSTONE /* Przycisk */
};

/* �ciany */
enum Walls
{
    WALL_SOLID, /* Normalna �ciana */
    WALL_DOOR /* Drzwi */
};

/* Obiekty */
enum Objects
{
    OBJECT_BOX, /* Pud�o */
    OBJECT_HERO, /* Bohater */
    OBJECT_SCROLL, /* Zw�j tekstu */
    OBJECT_KEY /* Klucz */
};

struct Field
{
    WORD root : 2; /* Typ podstawowy */
    WORD sub  : 3; /* Podtyp */
    WORD floor: 3; /* Podtyp pod�ogi (gdy root == OBJECT) */
};

struct Board
{
    struct Field board[ BOARD_HEIGHT ][ BOARD_WIDTH ];
};

/* Metody obs�ugi poszczeg�lnych klas */

enum Methods
{
    METHOD_ENTER, /* Wejd� na to pole */
    METHOD_LEAVE /* Opu�� to pole */
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
