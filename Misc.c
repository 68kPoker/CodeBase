
/*
** Magazyn (Warehouse)
** Misc.c
*/

#include "Screen.h"
#include "Windows.h"

int main(void)
{
	struct screenInfo si;
	
	if (openScreen(&si))
	{
		struct Window *w;
		
		if (w = openWindow(si.s))
		{	
			mainLoop(w, &si);
			CloseWindow(w);
		}	
		closeScreen(&si);
	}
	return(0);	
}
