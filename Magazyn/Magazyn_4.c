
#include "Game.h"

#include <clib/graphics_protos.h>

LONG play(struct gameTemplate *game)
{
    BltBitMap(game->gfx->bm, 0, 0, game->bm->bm, 0, 0, 320, 256, 0xc0, 0xff, NULL);
    Delay(400);
    return(0);
}

int main()
{
    struct gameTemplate *game;

    if (game = newGameTemplate())
    {
        if (game->bm = newGameBitMap())
        {
            if (game->gfx = newGameGraphics("Dane/Magazyn.iff"))
            {
                if (game->s = newGameScreen(game->bm, game->gfx))
                {
                    play(game);
                    disposeGameScreen(game->s);
                }
                disposeGameGraphics(game->gfx);
            }
            disposeGameBitMap(game->bm);
        }
        disposeGameTemplate(game);
    }
    return(0);
}
