
/* Audio.c: Sound playing */

/* $Id: Audio.c,v 1.1 12/.0/.0 .1:.1:.5 Robert Exp $ */

#include <stdlib.h>

#include <devices/audio.h>
#include <clib/exec_protos.h>

#include "Audio.h"
#include "IFF.h"

UBYTE channels[] = { 0xf };

struct IOAudio *allocChannels(void)
{
    struct MsgPort *mp;

    if (mp = CreateMsgPort())
    {
        struct IOAudio *ioa;

        if (ioa = (struct IOAudio *)CreateIORequest(mp, sizeof(*ioa)))
        {
            ioa->ioa_Request.io_Command = ADCMD_ALLOCATE;
            ioa->ioa_Request.io_Flags = ADIOF_NOWAIT;
            ioa->ioa_AllocKey = 0;
            ioa->ioa_Data = channels;
            ioa->ioa_Length = sizeof(channels);
            ioa->ioa_Request.io_Message.mn_Node.ln_Pri = 127;

            if (!OpenDevice("audio.device", 0L, (struct IORequest *)ioa, 0))
            {
                srand(100);
                return(ioa);
            }
            else
            {
                printf("Couldn't alloc Audio channels (%d)!\n", ioa->ioa_Request.io_Error);
            }
            DeleteIORequest((struct IORequest *)ioa);
        }
        DeleteMsgPort(mp);
    }
    return(0);
}

void freeChannels(struct IOAudio *ioa)
{
    struct MsgPort *mp = ioa->ioa_Request.io_Message.mn_ReplyPort;

    ioa->ioa_Request.io_Command  = ADCMD_FINISH;
    ioa->ioa_Request.io_Flags    = 0;
    ioa->ioa_Request.io_Unit     = (APTR)1 << 0;
    BeginIO((struct IORequest *)ioa);

    CloseDevice((struct IORequest *)ioa);
    DeleteIORequest((struct IORequest *)ioa);
    DeleteMsgPort(mp);
}

void playSample(struct IOAudio *ioa, struct soundSample *s, WORD chan)
{
    ioa->ioa_Request.io_Command  = ADCMD_FINISH;
    ioa->ioa_Request.io_Flags    = 0;
    ioa->ioa_Request.io_Unit     = (APTR)1 << chan;
    BeginIO((struct IORequest *)ioa);

    ioa->ioa_Request.io_Command  = CMD_WRITE;
    ioa->ioa_Request.io_Flags    = ADIOF_PERVOL;
    ioa->ioa_Data                = s->data;
    ioa->ioa_Length              = s->size;
    ioa->ioa_Period              = PAL_CLOCK / s->vhdr.vh_SamplesPerSec;
    ioa->ioa_Volume              = 25;
    ioa->ioa_Cycles              = 1;
    ioa->ioa_Request.io_Unit     = (APTR)1 << chan;
    BeginIO((struct IORequest *)ioa);

    /* WaitIO((struct IORequest *)ioa); */
}
