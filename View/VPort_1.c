
#include "vport.h"

#include <graphics/view.h>
#include <clib/exec_protos.h>

BOOL bindDBuf(struct DBufStatus *p, struct DBufInfo *dbi)
{
    dbi->dbi_SafeMessage.mn_ReplyPort = p->mp[SAFE];
    dbi->dbi_DispMessage.mn_ReplyPort = p->mp[DISP];
    return(TRUE);
}

BOOL initDBuf(struct DBufStatus *p)
{
    if (p->mp[SAFE] = CreateMsgPort())
        {
        if (p->mp[DISP] = CreateMsgPort())
            {
            p->safe[SAFE] = p->safe[DISP] = TRUE;
            p->frame = 1;
            return(TRUE);
            }
        DeleteMsgPort(p->mp[SAFE]);
        }
    return(FALSE);
}

VOID freeDBuf(struct DBufStatus *p)
{
    safeDBuf(p, DISP);
    safeDBuf(p, SAFE);
    DeleteMsgPort(p->mp[DISP]);
    DeleteMsgPort(p->mp[SAFE]);
}

BOOL safeDBuf(struct DBufStatus *p, WORD type)
{
    if (!p->safe[type])
        {
        while (!GetMsg(p->mp[type]))
            {
            WaitPort(p->mp[type]);
            }
        p->safe[type] = TRUE;
        }
    return(TRUE);
}
