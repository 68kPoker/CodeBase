
#include <intuition/intuition.h>
#include <clib/intuition_protos.h>

#include "GUI.h"

struct screenParams defScreenParams =
{
    LORES_KEY, { 0, 0, 319, 255 }, 5
};

struct Screen *openScreen(struct screenParams *sp);

/* Open default screen */

struct Screen *openDefScreen(void)
{
    return openScreen(&defScreenParams);
}

struct Screen *openScreen(struct screenParams *sp)
{
    struct Screen *s;

    if (s = OpenScreenTags(NULL,
        SA_DisplayID,   sp->modeID,
        SA_DClip,       &sp->dclip,
        SA_Depth,       sp->depth,
        SA_Title,       "Magazyn",
        SA_Quiet,       TRUE,
        SA_ShowTitle,   FALSE,
        SA_Exclusive,   TRUE,
        TAG_DONE))
    {
        return s;
    }
    return NULL;
}

int main(void)
{
    struct Screen *s;

    if (s = openDefScreen())
    {
        Delay(400);
        CloseScreen(s);
    }
    return 0;
}
