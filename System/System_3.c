
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>

#include "System.h"
#include "Joystick.h"
#include "GUI.h"

struct Library *IntuitionBase, *GfxBase, *DataTypesBase;

BOOL openlibs()
{
    if (IntuitionBase = OpenLibrary("intuition.library", 39))
    {
        if (GfxBase = OpenLibrary("graphics.library", 39))
        {
            if (DataTypesBase = OpenLibrary("datatypes.library", 39))
            {
                return(TRUE);
            }
            CloseLibrary(GfxBase);
        }
        CloseLibrary(IntuitionBase);
    }
    return(FALSE);
}

void closelibs()
{
    CloseLibrary(DataTypesBase);
    CloseLibrary(GfxBase);
    CloseLibrary(IntuitionBase);
}

BOOL initsystem(struct system *sys)
{
    if (sys->joyio = openjoy(&sys->joyie))
    {
        if (sys->s = openscreen(&sys->tf))
        {
            if (sys->w = openwindow(sys->s))
            {
                if (allocbuffers(sys->sb, sys->s))
                {
                    if (sys->dbufports[0] = CreateMsgPort())
                    {
                        if (sys->dbufports[1] = CreateMsgPort())
                        {
                            attachports(sys->sb[0], sys->dbufports);
                            attachports(sys->sb[1], sys->dbufports);
                            sys->safe[0] = sys->safe[1] = TRUE;
                            sys->frame = 1;
                            sys->backDrawn[0] = sys->backDrawn[1] = FALSE;
                            return(TRUE);
                        }
                        DeleteMsgPort(sys->dbufports[0]);
                    }
                    freebuffers(sys->s, sys->sb);
                }
                CloseWindow(sys->w);
            }
            CloseScreen(sys->s);
            CloseFont(sys->tf);
        }
        closejoy(sys->joyio);
    }
    return(FALSE);
}

void closesystem(struct system *sys)
{
    if (!sys->safe[1])
        while (!GetMsg(sys->dbufports[1]))
            WaitPort(sys->dbufports[1]);

    if (!sys->safe[0])
        while (!GetMsg(sys->dbufports[0]))
            WaitPort(sys->dbufports[0]);

    freebuffers(sys->s, sys->sb);

    DeleteMsgPort(sys->dbufports[1]);
    DeleteMsgPort(sys->dbufports[0]);

    CloseWindow(sys->w);
    CloseScreen(sys->s);
    CloseFont(sys->tf);
    closejoy(sys->joyio);
}

int main()
{
    struct system sys;

    if (openlibs())
    {
        if (initsystem(&sys))
        {
            if (initgame(&sys))
            {
                playgame(&sys);
                closegame(&sys);
            }
            closesystem(&sys);
        }
        closelibs();
    }
    return(0);
}
