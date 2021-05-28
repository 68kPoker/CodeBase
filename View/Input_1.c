
/* Magazyn */

/* Wlasny input handler */

#include <devices/input.h>
#include <exec/interrupts.h>
#include <devices/inputevent.h>
#include <clib/exec_protos.h>

#define INPUT_PRI 120

struct IOStdReq *inpio;
struct Interrupt inpis;
struct InputEvent inpie;

__saveds struct InputEvent *myInput(register __a0 struct InputEvent *ie, register __a1 APTR data)
{
	return(NULL);
}

BOOL openInput(void)
{
	struct MsgPort *mp;

	if (mp = CreateMsgPort())
	{
		if (inpio = CreateIORequest(mp, sizeof(*inpio)))
		{
			if (OpenDevice("input.device", 0, (struct IORequest *)inpio, 0) == 0)
			{
				inpio->io_Command = IND_ADDHANDLER;
				inpio->io_Data = (APTR)&inpis;

				inpis.is_Data = NULL;
				inpis.is_Code = (void(*)())myInput;
				inpis.is_Node.ln_Pri = INPUT_PRI;
				inpis.is_Node.ln_Name = "Magazyn";

				DoIO((struct IORequest *)inpio);
				return(TRUE);
			}
			DeleteIORequest((struct IORequest *)inpio);
		}
		DeleteMsgPort(mp);
	}
	return(FALSE);
}

void closeInput(void)
{
	struct MsgPort *mp = inpio->io_Message.mn_ReplyPort;

	inpio->io_Command = IND_REMHANDLER;
	inpio->io_Data = (APTR)&inpis;

	DoIO((struct IORequest *)inpio);

	CloseDevice((struct IORequest *)inpio);
	DeleteIORequest((struct IORequest *)inpio);
	DeleteMsgPort(mp);
}
