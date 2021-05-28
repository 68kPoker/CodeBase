
#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <devices/inputevent.h>

struct Joystick
{
    struct IOStdReq *io;
    struct InputEvent ie;
};

#endif /* JOYSTICK_H */
