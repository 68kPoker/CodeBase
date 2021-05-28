
#include "iffp/ilbmapp.h"

#include <clib/iffparse_protos.h>

BOOL loadILBM(STRPTR name, struct ILBMInfo *ilbm)
{
	LONG props[] = 
	{
		ID_ILBM, ID_BMHD,
		ID_ILBM, ID_CMAP,
		0
	};
	
	LONG stops[] =
	{
		ID_ILBM, ID_BODY,
		0
	};
			
	if (ilbm->ParseInfo.iff = AllocIFF())
	{
		ilbm->ParseInfo.propchks = props;
		ilbm->ParseInfo.stopchks = stops;
		
		if (loadbrush(ilbm, name) == 0)
		{
			return(TRUE);
		}	
		FreeIFF(ilbm->ParseInfo.iff);
	}
	return(FALSE);
}

void unloadILBM(struct ILBMInfo *ilbm)
{
	unloadbrush(ilbm);
	FreeIFF(ilbm->ParseInfo.iff);
}	
