
#ifndef SYSTEM_H
#define SYSTEM_H

#include <exec/interrupts.h>
#include "Screen.h"

struct systemInfo
{
    struct TextFont *tf;
    struct Screen *s;
    struct Interrupt is;
    struct copperInfo ci;
};

BOOL openScreen(struct systemInfo *si, ULONG tag1, ...);
void closeScreen(struct systemInfo *si);

#endif /* SYSTEM_H */
