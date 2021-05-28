
#include <dos/dos.h>
#include <exec/types.h>
#include <intuition/intuition.h>
#include <hardware/blit.h>
#include <hardware/custom.h>

#include <clib/exec_protos.h>

struct totalnode
{
    struct bltnode bn;
    struct BitMap *bm, *gfx;
};

struct Task *task;
WORD signal;

extern int func(), clean();

void test(struct Window *w, struct BitMap *gfx)
{
    struct totalnode bn =
    {
        {
            NULL,
            (int(*)())func,
            CLEANUP,
            0,
            0,
            (int(*)())clean
        },
        w->RPort->BitMap,
        gfx
    };

    task = FindTask(NULL);

    if ((signal = AllocSignal(-1)) != -1)
    {
        QBSBlit(&bn);
        Wait((1L << signal) | SIGBREAKF_CTRL_C);
        Delay(100);
        FreeSignal(signal);
    }
}
