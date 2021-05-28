
#ifndef JOYSTICK_PROTOS_H
#define JOYSTICK_PROTOS_H

/*==================== Joystick ====================*/

typedef struct Joystick JOY;

JOY *openJoystick();
void readEvent(JOY *joy);
void closeJoystick(JOY *joy);

#endif /* JOYSTICK_PROTOS_H */
