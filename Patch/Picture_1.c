
#include <datatypes/pictureclass.h>
#include <clib/datatypes_protos.h>
#include <clib/graphics_protos.h>

struct BitMap *loadGfx(STRPTR name, struct Window *w)
{
	Object *o;

	if (o = NewDTObject(name,
		DTA_GroupID,	GID_PICTURE,
		PDTA_Screen,	w->WScreen,
		PDTA_Remap,		FALSE,
		TAG_DONE))
	{
		struct BitMap *bm, *gfx, *friend;
		WORD width, height;
		ULONG *cregs;
		WORD i;

		DoDTMethod(o, NULL, NULL, DTM_PROCLAYOUT, NULL, TRUE);
		GetDTAttrs(o,
			PDTA_BitMap,	&bm,
			PDTA_CRegs,	&cregs,
			TAG_DONE);

		for (i = 0; i < 16; i++)
		{
			SetRGB32CM(w->WScreen->ViewPort.ColorMap, i, cregs[0], cregs[1], cregs[2]);
			cregs += 3;
		}
		MakeScreen(w->WScreen);
		RethinkDisplay();

		width = GetBitMapAttr(bm, BMA_WIDTH);
		height = GetBitMapAttr(bm, BMA_HEIGHT);

		friend = w->RPort->BitMap;

		if (gfx = AllocBitMap(width, height, GetBitMapAttr(friend, BMA_DEPTH), 0, friend))
		{
			BltBitMap(bm, 0, 0, gfx, 0, 0, width, height, 0xc0, 0xff, NULL);
			WaitBlit();
			DisposeDTObject(o);
			return(gfx);
		}
		DisposeDTObject(o);
	}
	return(NULL);
}
