
/*
**  (C)2018-2020 Robert Szacki Software House
**
**  » Magazyn «
**
**  $Id$
*/

#ifndef JOYSTICK_H
#define JOYSTICK_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#define TIMEOUT 100

struct IOStdReq* openGameport(struct InputEvent* ie);
BOOL setController  (struct IOStdReq* io);
void setTrigger     (struct IOStdReq* io);
void clearIO        (struct IOStdReq* io);
void readEvent      (struct IOStdReq* io, struct InputEvent* ie);
void closeGameport  (struct IOStdReq* io);
void resetController(struct IOStdReq* io);

#endif /* JOYSTICK_H */
