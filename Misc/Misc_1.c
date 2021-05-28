
/*
** Magazyn (Warehouse)
** Misc.c
*/

#include "Screen.h"
#include "Windows.h"

#include "iffp/ilbmapp.h"
#include "IFF.h"

#include <clib/graphics_protos.h>

int main(void)
{
	struct screenInfo si;
	
	if (openScreen(&si))
	{
		struct Window *w;
		
		if (w = openWindow(si.s))
		{	
			static struct ILBMInfo ilbm = { 0 };
			
			if (loadILBM("Data1/Graphics.iff", &ilbm))
			{
				LoadRGB32(&si.s->ViewPort, (ULONG *)ilbm.colorrecord);
				mainLoop(w, &si, ilbm.brbitmap);
				unloadILBM(&ilbm);
			}	
			CloseWindow(w);
		}	
		closeScreen(&si);
	}
	return(0);	
}
