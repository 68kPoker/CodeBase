
/* Przerwanie Blittera */

#include <hardware/blit.h>
#include <hardware/intbits.h>
#include <hardware/custom.h>
#include <dos/dos.h>
#include <exec/interrupts.h>
#include <intuition/screens.h>

#include <clib/exec_protos.h>

struct blitterData
{
    struct Task *task;
    ULONG signalmask;
    WORD signal;
};

extern __far struct Custom custom;
extern void blitterHandler();

struct BitMap *allocBitMaps(struct BitMap **rasbm, WORD width, WORD height, UBYTE depth, WORD raswidth, WORD rasheight);
struct Screen *openScreen(ULONG modeID, WORD width, WORD height, UBYTE depth, struct BitMap *bm);
void test(struct Screen *s, struct BitMap *rasbm, ULONG signalmask);

BOOL prepBlitterData(struct Interrupt *is, struct blitterData *bd, void (*blitterHandler)(void))
{
    bd->task = FindTask(NULL);
    if ((bd->signal = AllocSignal(-1)) != -1)
    {
        bd->signalmask = 1L << bd->signal;
        is->is_Data = (APTR)bd;
        is->is_Code = blitterHandler;
        return(TRUE);
    }
    return(FALSE);
}

BOOL readBlitterIntState()
{
    return(custom.intenar & INTF_BLIT);
}

struct Interrupt *setBlitterVector(struct Interrupt *is)
{
    struct Interrupt *previs;
    previs = SetIntVector(INTB_BLIT, is);
    custom.intena = INTF_SETCLR|INTF_BLIT;
}

void resetBlitterVector(struct Interrupt *is, BOOL state)
{
    custom.intena = INTF_BLIT;
    SetIntVector(INTB_BLIT, is);
    if (state) custom.intena = INTF_SETCLR|INTF_BLIT;
    FreeSignal(((struct blitterData *)is->is_Data)->signal);
}

/*
void test(struct blitterData *bd)
{
    SetSignal(0L, bd->signalmask);
    Wait(bd->signalmask|SIGBREAKF_CTRL_C);
}
*/

int main()
{
    struct BitMap *bm, *rasbm;
    struct Screen *s;
    struct Interrupt blitis, *prevblitis;
    struct blitterData blitdata;

    if (bm = allocBitMaps(&rasbm, 640, 512, 5, 640, 512))
    {
        if (s = openScreen(DBLPAL_MONITOR_ID|HIRESLACE_KEY, 640, 512, 5, bm))
        {
            if (prepBlitterData(&blitis, &blitdata, blitterHandler))
            {
                BOOL state = readBlitterIntState();
                prevblitis = setBlitterVector(&blitis);
                test(s, rasbm, blitdata.signalmask);
                resetBlitterVector(prevblitis, state);
                Delay(300);
            }
            CloseScreen(s);
        }
        FreeBitMap(rasbm);
        FreeBitMap(bm);
    }
    return(0);
}
