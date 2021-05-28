
/* param: chunky pixels: a0
** param: interleaved bitplanes: a1
** param: bytes per scanline: d0
** param: bytes to write: d1
*/

#include <exec/types.h>

void WritePixelLineLo(register __a0 UBYTE *cp, register __a1 UBYTE *plane, register __d0 UWORD bpr, register __d1 UWORD bytes);
void WritePixelLineHi(register __a0 UBYTE *cp, register __a1 UBYTE *plane, register __d0 UWORD bpr, register __d1 UWORD bytes);
