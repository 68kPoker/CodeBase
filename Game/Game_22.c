
/*
 * Root Warehouse game handlers. Handle events from IDCMP, joystick,
 *      timer etc.
 */

#include <assert.h>
#include <devices/inputevent.h>
#include <clib/graphics_protos.h>
#include "Game.h"
#include <stdio.h>
#include "debug.h"

LONG selectTile(struct gameUser *user, WORD mouseX, WORD mouseY)
    {
    assert(mouseX >= 0 && mouseX < BOARD_WIDTH && mouseY >= 0 && mouseY < BOARD_HEIGHT);

    /* Toggle selection */
    user->board[mouseY][mouseX].selected ^= 1;

    return(TRUE);
    }

LONG paintTile(struct gameUser *user, WORD mouseX, WORD mouseY)
    {
    assert(mouseX >= 0 && mouseX < BOARD_WIDTH && mouseY >= 0 && mouseY < BOARD_HEIGHT);

    /* Paint current tile */
    user->board[mouseY][mouseX].type = user->currentType;
    user->board[mouseY][mouseX].redraw = 2;

    return(TRUE);
    }

LONG mouseButtons(struct gameUser *user, UWORD code, WORD mouseX, WORD mouseY)
    {
    mouseX >>= 4;
    mouseY >>= 4;

    if (code == IECODE_LBUTTON)
        {
        /* LMB pressed. Enable paintMode and draw or select tile */
        user->paintMode = TRUE;

        if (user->selectMode)
            {
            /* Select tile under mouse */
            selectTile(user, mouseX, mouseY);
            }
        else
            {
            /* Paint tile under mouse */
            paintTile(user, mouseX, mouseY);
            }
        removeCursor(user);
        showCursor(user, mouseX, mouseY);
        }
    else if (code == (IECODE_LBUTTON|IECODE_UP_PREFIX))
        {
        /* LMB released. Disable paintMode */
        user->paintMode = FALSE;
        }
    return(TRUE);
    }

LONG removeCursor(struct gameUser *user)
    {
    /* Redraw original tile */
    user->board[user->cursY][user->cursX].redraw = 2;
    return(TRUE);
    }

LONG showCursor(struct gameUser *user, WORD mouseX, WORD mouseY)
    {
    assert(mouseX >= 0 && mouseX < BOARD_WIDTH && mouseY >= 0 && mouseY < BOARD_HEIGHT);

    user->cursX = mouseX;
    user->cursY = mouseY;
    user->updateCurs = 2;
    return(TRUE);
    }

LONG mouseMove(struct gameUser *user, UWORD code, WORD mouseX, WORD mouseY)
    {
    mouseX >>= 4;
    mouseY >>= 4;

    if (!(mouseX >= 0 && mouseX < BOARD_WIDTH && mouseY >= 0 && mouseY < BOARD_HEIGHT))
        {
        return(FALSE);
        }

    /* Check if moved to other tile */
    if (mouseX != user->prevX || mouseY != user->prevY)
        {
        if (user->paintMode)
            {
            /* Paint mode is ON. Select or paint new tile */
            if (user->selectMode)
                {
                selectTile(user, mouseX, mouseY);
                }
            else
                {
                paintTile(user, mouseX, mouseY);
                }
            }
        /* Paint mode is OFF. Just show cursor */
        removeCursor(user);
        showCursor(user, mouseX, mouseY);

        user->prevX = mouseX;
        user->prevY = mouseY;
        }
    return(TRUE);
    }

LONG rawKey(struct gameUser *user, UWORD code, WORD qualifier, WORD mouseX, WORD mouseY)
    {
    if (code == 0x45)
        {
        return(FALSE);
        }
    return(TRUE);
    }

void drawTile(struct gameUser *user, struct RastPort *rp, WORD frame, WORD x, WORD y)
    {
    SetAPen(rp, user->board[y][x].type);
    RectFill(rp, x << 4, y << 4, (x << 4) + 15, (y << 4) + 15);
    }

LONG draw(struct gameUser *user, struct RastPort *rp, WORD frame)
    {
    WORD x, y;

    for (y = 0; y < BOARD_HEIGHT; y++)
        {
        for (x = 0; x < BOARD_WIDTH; x++)
            {
            struct gameTile *tile = &user->board[y][x];
            if (tile->redraw > 0)
                {
                tile->redraw--;
                drawTile(user, rp, frame, x, y);
                }
            }
        }

    if (user->updateCurs > 0)
        {
        user->updateCurs--;
        SetAPen(rp, 2);
        Move(rp, user->cursX << 4, user->cursY << 4);
        Draw(rp, (user->cursX << 4) + 15, user->cursY << 4);
        Draw(rp, (user->cursX << 4) + 15, (user->cursY << 4) + 15);
        Draw(rp, user->cursX << 4, (user->cursY << 4) + 15);
        Draw(rp, user->cursX << 4, (user->cursY << 4) + 1);
        }
    }

