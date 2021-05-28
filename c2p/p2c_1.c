
#include <stdlib.h>
#include <stdio.h>
#include <exec/types.h>

void p2ctest(ULONG *dest)
{
    static ULONG data[4]=
    {
        0x55555555,
        0x33333333,
        0x0F0F0F0F,
        0x00FF00FF
    };

    register ULONG mask = 0x88888888;
    register ULONG work1, work2, work3, work4, temp;
    register WORD i;

    work1 = data[0];
    work2 = data[1];
    work3 = data[2];
    work4 = data[3];

    for (i = 0; i < 4; i++)
    {
        *dest++ = ((((((work1 & mask) >> 1) | (work2 & mask)) >> 1) | (work3 & mask)) >> 1) | (work4 & mask);

        work1 <<= 1;
        work2 <<= 1;
        work3 <<= 1;
        work4 <<= 1;
    }
}

void c2ptest(ULONG *data, ULONG *plane0, ULONG *plane1, ULONG *plane2, ULONG *plane3)
{
    register ULONG mask = 0x88888888;
    register ULONG work1, work2, work3, work4, temp;
    register WORD i;

    work1 = data[0]; /* Pixels 0, 8, 16, 24 */
    work2 = data[1];
    work3 = data[2];
    work4 = data[3];

    *plane3++ = ((((((work4 & mask) >> 1) | (work3 & mask)) >> 1) | (work2 & mask)) >> 1) | (work1 & mask);

    work1 <<= 1;
    work2 <<= 1;
    work3 <<= 1;
    work4 <<= 1;

    *plane2++ = ((((((work4 & mask) >> 1) | (work3 & mask)) >> 1) | (work2 & mask)) >> 1) | (work1 & mask);

    work1 <<= 1;
    work2 <<= 1;
    work3 <<= 1;
    work4 <<= 1;

    *plane1++ = ((((((work4 & mask) >> 1) | (work3 & mask)) >> 1) | (work2 & mask)) >> 1) | (work1 & mask);

    work1 <<= 1;
    work2 <<= 1;
    work3 <<= 1;
    work4 <<= 1;

    *plane0++ = ((((((work4 & mask) >> 1) | (work3 & mask)) >> 1) | (work2 & mask)) >> 1) | (work1 & mask);
}

int main()
{
    WORD i, j;
    static ULONG *pixels;
    ULONG plane0, plane1, plane2, plane3;

    if (pixels = calloc(20480, sizeof(ULONG)))
    {
        printf("Start\n");
        p2ctest(pixels);
        printf("Stop\n");
        c2ptest(pixels, &plane0, &plane1, &plane2, &plane3);

        printf("$%x $%x $%x $%x\n", plane0, plane1, plane2, plane3);

        free(pixels);
    }
    return(0);
}
