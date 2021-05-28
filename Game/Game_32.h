
#include <exec/types.h>

#define BOARD_WIDTH  20
#define BOARD_HEIGHT 16

#define ID_MAGA MAKE_ID('M','A','G','A')
#define ID_NAGL MAKE_ID('N','A','G','L')
#define ID_PLAN MAKE_ID('P','L','A','N')
#define ID_STAN MAKE_ID('S','T','A','N')

typedef struct tile
{
    WORD kind : 4, subKind : 4, floor : 4;
} TILE;

struct boardData
{
    TILE board[BOARD_HEIGHT][BOARD_WIDTH];
};

BOOL loadBoard(STRPTR name, struct boardData *bd);
