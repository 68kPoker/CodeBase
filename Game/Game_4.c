
#include <stdio.h>

#include <dos/dos.h>
#include <intuition/intuition.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>

#define COLUMNS 20
#define ROWS    16
#define NAME    64

enum
{
    PLAY,
    RESTART,
    QUIT
};

enum
{
    FLOOR,
    WALL,
    BOX,
    CHERRY,
    FLAGSTONE,
    SKULL,
    HERO
};

extern struct Window *window;

BOOL loadBoard(STRPTR name);

WORD board[ROWS][COLUMNS];

WORD action = PLAY;

BOOL loadBoard(STRPTR name)
{
    BPTR f;
    BOOL result = FALSE;

    if (f = Open(name, MODE_OLDFILE))
    {
        WORD row, col;
        UBYTE columns[COLUMNS + 1];
        for (row = 0; row < ROWS; row++)
        {
            WORD *cell = &board[row][0], c;
            if (FGets(f, columns, COLUMNS + 1))
            {
                for (col = 0; col < COLUMNS; col++)
                {
                    switch (columns[col])
                    {
                        case '#': *cell = WALL;     break;
                        case '·': *cell = FLOOR;    break;
                        case '*': *cell = BOX;      break;
                        case '$': *cell = CHERRY;   break;
                        case 'o': *cell = FLAGSTONE;break;
                        case 'H': *cell = HERO;     break;
                        case '@': *cell = SKULL;    break;
                        defualt:  *cell = FLOOR;
                    }
                    cell++;
                }
            }
            while ((c = FGetC(f)) != '\n' && c != EOF)
                ;
        }
        Close(f);
        result = TRUE;
    }
    else
        printf("Couldn't open '%s'\n", name);
    return(result);
}

void drawCell(WORD row, WORD col, WORD cell)
{
    struct RastPort *rp = window->RPort;

    if (cell > 0)
    {
        SetAPen(rp, cell);
        RectFill(rp, col << 4, row << 4, (col << 4) + 15, (row << 4) + 15);
    }
}

void drawBoard(void)
{
    WORD row, col;

    for (row = 0; row < ROWS; row++)
    {
        for (col = 0; col < COLUMNS; col++)
        {
            drawCell(row, col, board[row][col]);
        }
    }
}

BOOL processBoard(void)
{
    struct MsgPort *mp = window->UserPort;
    struct IntuiMessage *msg;
    BOOL done = FALSE;

    WaitPort(mp);

    while (msg = (struct IntuiMessage *)GetMsg(mp))
    {
        if (msg->Class == IDCMP_RAWKEY)
        {
            if (msg->Code == 0x45)
            {
                action = QUIT;
                done = TRUE;
            }
        }
        ReplyMsg((struct Message *)msg);
    }
    return(done);
}

void game(void)
{
    char name[NAME];
    BOOL done = FALSE;
    WORD level = 1;

    sprintf(name, "Data/Level%02d.txt", level);

    while (!done)
    {
        if (loadBoard(name))
        {
            drawBoard();
            while (!(done = processBoard()))
            {
                if (action == QUIT)
                {
                    done = TRUE;
                }
            }
        }
        else
        {
            printf("No more level files.\n");
            done = TRUE;
        }
    }
}
