
/* Basis code */

#include <intuition/intuition.h>
#include <exec/memory.h>
#include <exec/interrupts.h>
#include <graphics/gfxmacros.h>
#include <hardware/custom.h>
#include <hardware/intbits.h>

#include <devices/inputevent.h>
#include <devices/gameport.h>

#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>

#define DEPTH 5
#define IDCMP IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE|IDCMP_RAWKEY
#define TIMEOUT 50

far extern struct Custom custom;
extern void myCopper(void);

/* Double-buffered screen */

struct Screen       *screen;
struct ScreenBuffer *sbuffers[2];
struct Window       *window;
struct MsgPort      *safeport;
struct Interrupt    copperint;

struct copperData
{
    struct ViewPort *vp;
    WORD            signal;
    struct Task     *task;
} copperdata;

BOOL    safe;
UWORD   frame;

/* Joystick stuff */

struct MsgPort      *joyport;
struct IOStdReq     *joyio;
struct InputEvent   joyie;

/* Open double-buffered screen */

BOOL setupScreen(void)
{
    struct Rectangle dclip = { 0, 0, 319, 255 };

    if (screen = OpenScreenTags(NULL,
        SA_DClip,       &dclip,
        SA_Depth,       DEPTH,
        SA_DisplayID,   LORES_KEY,
        SA_Quiet,       TRUE,
        SA_ShowTitle,   FALSE,
        SA_BackFill,    LAYERS_NOBACKFILL,
        TAG_DONE))
    {
        if (sbuffers[0] = AllocScreenBuffer(screen, NULL, SB_SCREEN_BITMAP))
        {
            if (sbuffers[1] = AllocScreenBuffer(screen, NULL, 0))
            {
                if (safeport = CreateMsgPort())
                {
                    struct UCopList *ucl;

                    sbuffers[0]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = safeport;
                    sbuffers[1]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = safeport;

                    safe = TRUE;
                    frame = 1;

                    if (ucl = AllocMem(sizeof(*ucl), MEMF_PUBLIC|MEMF_CLEAR))
                    {
                        CINIT(ucl, 3);
                        CWAIT(ucl, 0, 0);
                        CMOVE(ucl, custom.intreq, INTF_SETCLR|INTF_COPER);
                        CEND(ucl);

                        Forbid();
                        screen->ViewPort.UCopIns = ucl;
                        Permit();

                        RethinkDisplay();

                        if ((copperdata.signal = AllocSignal(-1)) != -1)
                        {
                            copperdata.vp = &screen->ViewPort;
                            copperdata.task = FindTask(NULL);

                            copperint.is_Code = myCopper;
                            copperint.is_Data = (APTR)&copperdata;
                            copperint.is_Node.ln_Pri = 0;

                            AddIntServer(INTB_COPER, &copperint);

                            if (window = OpenWindowTags(NULL,
                                WA_CustomScreen,    screen,
                                WA_Left,            0,
                                WA_Top,             0,
                                WA_Width,           screen->Width,
                                WA_Height,          screen->Height,
                                WA_Backdrop,        TRUE,
                                WA_Borderless,      TRUE,
                                WA_Activate,        TRUE,
                                WA_RMBTrap,         TRUE,
                                WA_SimpleRefresh,   TRUE,
                                WA_BackFill,        LAYERS_NOBACKFILL,
                                WA_ReportMouse,     TRUE,
                                WA_IDCMP,           IDCMP,
                                TAG_DONE))
                            {
                                return(TRUE);
                            }

                            RemIntServer(INTB_COPER, &copperint);
                            FreeSignal(copperdata.signal);
                        }
                    }
                    DeleteMsgPort(safeport);
                }
                FreeScreenBuffer(screen, sbuffers[1]);
            }
            FreeScreenBuffer(screen, sbuffers[0]);
        }
        CloseScreen(screen);
    }
    return(FALSE);
}

void closeScreen(void)
{
    CloseWindow(window);
    RemIntServer(INTB_COPER, &copperint);
    FreeSignal(copperdata.signal);

    if (!safe)
    {
        while (!GetMsg(safeport))
        {
            WaitPort(safeport);
        }
    }

    DeleteMsgPort(safeport);
    FreeScreenBuffer(screen, sbuffers[1]);
    FreeScreenBuffer(screen, sbuffers[0]);
    CloseScreen(screen);
}

/* Open gameport */

BOOL checkController(void)
{
    BOOL success = FALSE;
    BYTE ctype;

    Forbid();

    joyio->io_Command   = GPD_ASKCTYPE;
    joyio->io_Data      = (APTR)&ctype;
    joyio->io_Length    = 1;
    joyio->io_Flags     = IOF_QUICK;

    DoIO((struct IORequest *)joyio);

    if (ctype == GPCT_NOCONTROLLER)
    {
        success = TRUE;

        ctype = GPCT_ABSJOYSTICK;

        joyio->io_Command   = GPD_SETCTYPE;
        joyio->io_Data      = (APTR)&ctype;
        joyio->io_Length    = 1;
        joyio->io_Flags     = IOF_QUICK;

        DoIO((struct IORequest *)joyio);
    }

    Permit();

    return(success);
}

void clearController(void)
{
    BYTE ctype = GPCT_NOCONTROLLER;

    joyio->io_Command   = GPD_SETCTYPE;
    joyio->io_Data      = (APTR)&ctype;
    joyio->io_Length    = 1;
    joyio->io_Flags     = IOF_QUICK;

    DoIO((struct IORequest *)joyio);
}

void setTrigger(void)
{
    struct GamePortTrigger gpt;

    gpt.gpt_Keys    = GPTF_DOWNKEYS|GPTF_UPKEYS;
    gpt.gpt_XDelta  = 1;
    gpt.gpt_YDelta  = 1;
    gpt.gpt_Timeout = TIMEOUT;

    joyio->io_Command   = GPD_SETTRIGGER;
    joyio->io_Data      = (APTR)&gpt;
    joyio->io_Length    = sizeof(gpt);
    joyio->io_Flags     = IOF_QUICK;

    DoIO((struct IORequest *)joyio);
}

void clearIO(struct IOStdReq *io)
{
    io->io_Command  = CMD_CLEAR;
    io->io_Data     = NULL;
    io->io_Length   = 0;
    io->io_Flags    = IOF_QUICK;

    DoIO((struct IORequest *)joyio);
}

void readEvent(void)
{
    joyio->io_Command  = GPD_READEVENT;
    joyio->io_Data     = (APTR)&joyie;
    joyio->io_Length   = sizeof(joyie);
    joyio->io_Flags    = 0;

    SendIO((struct IORequest *)joyio);
}

BOOL setupGameport(void)
{
    if (joyport = CreateMsgPort())
    {
        if (joyio = CreateIORequest(joyport, sizeof(*joyio)))
        {
            if (!OpenDevice("gameport.device", 1, (struct IORequest *)joyio, 0))
            {
                if (checkController())
                {
                    setTrigger();
                    clearIO(joyio);
                    readEvent();
                    return(TRUE);
                }
                CloseDevice((struct IORequest *)joyio);
            }
            DeleteIORequest((struct IORequest *)joyio);
        }
        DeleteMsgPort(joyport);
    }
    return(FALSE);
}

void closeGameport(void)
{
    if (!CheckIO((struct IORequest *)joyio))
    {
        AbortIO((struct IORequest *)joyio);
    }
    WaitIO((struct IORequest *)joyio);

    clearController();

    CloseDevice((struct IORequest *)joyio);
    DeleteIORequest((struct IORequest *)joyio);
    DeleteMsgPort(joyport);
}
