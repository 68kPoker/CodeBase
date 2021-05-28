
/* My patches */

#include <graphics/rastport.h>
#include <graphics/layers.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>

#include "Patch.h"

extern struct Library *GfxBase;

APTR origScrollRaster;

__saveds void myScrollRaster(register __a1 struct RastPort *rp,
	register __d0 WORD dx, register __d1 WORD dy,
	register __d2 WORD xMin, register __d3 WORD yMin,
	register __d4 WORD xMax, register __d5 WORD yMax)
{
	rp->Layer->BackFill = LAYERS_NOBACKFILL;

	ScrollRasterBF(rp, dx, dy, xMin, yMin, xMax, yMax);
}

void installPatch(void)
{
	origScrollRaster = SetFunction(GfxBase, -396, (ULONG(*)())myScrollRaster);
}

void removePatch(void)
{
	SetFunction(GfxBase, -396, origScrollRaster);
}
