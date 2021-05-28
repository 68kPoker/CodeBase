
#ifndef CINPUT_H
#define CINPUT_H

#include <exec/types.h>
#include <exec/interrupts.h>
#include <devices/inputevent.h>

#define ESC_KEY 0x45

typedef void( *CInputHandler )( APTR user );

typedef struct {
    ULONG         signalMask;
    CInputHandler handler;
    APTR          user;
} CInput;

typedef struct {
    struct MsgPort *userPort;
} CIDCMP;

typedef struct {
    struct IOStdReq *io;
    struct InputEvent ie;
} CController;

typedef struct {
    struct ViewPort *vp;
    WORD signal;
    struct Task *task;
    struct Interrupt is;

    struct CScreen *s;
} CCopper;

typedef struct {
    struct MsgPort *mp;
    BOOL safe;

    struct CScreen *s;
} CSafeToDraw;

typedef enum {
    IN_IDCMP,
    IN_JOYSTICK,
    IN_COPPER,
    IN_SAFE,
    INPUTS
} EInput;

typedef CInput CInputs[ INPUTS ]; /* CInput array */

void initInput( CInput *in, ULONG signalMask, CInputHandler handler, APTR user );
void initIDCMP( CInput *in, CIDCMP *idcmp, struct MsgPort *userPort, CInputHandler handler );
void initCopper( CInput *in, CCopper *cop, CInputHandler handler );
void initSafe( CInput *in, CSafeToDraw *safe, CInputHandler handler );

BOOL openController( CInput *in, CController *con, CInputHandler handler );
void readEvent( CController *con );
void closeController( CController *con );

void game( CInputs inputs );

#endif /* CINPUT_H */
