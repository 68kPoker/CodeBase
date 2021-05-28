
#include "Screen.h"
#include "Windows.h"

enum
{
    GID_TILE,
    GID_DISK
};

int main()
{
    struct Screen *s;
    struct windowData menu;

    if (s = openScreen())
    {
        struct Window *w = obtainWindow(s);
        if (initWindow(w, &menu))
        {
            addItem(&menu, "Tile", GID_TILE, 0, 0, 80, 16);
            addItem(&menu, "Disk", GID_DISK, 80, 0, 80, 16);


            Delay(500);

            closeWindow(&menu);
        }
        closeScreen(s);
    }
    return(0);
}
