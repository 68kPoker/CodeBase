
#include "ILBM.h"

#include <intuition/screens.h>
#include <intuition/gadgetclass.h>
#include <libraries/gadtools.h>
#include <graphics/scale.h>

#include <clib/gadtools_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/diskfont_protos.h>

#define DEPTH 5

void bltTileRastPort(struct BitMap *src, WORD sx, WORD sy, struct RastPort *rp, WORD dx, WORD dy, WORD width, WORD height);

struct TextAttr ta =
{
	"helvetica.font",
	11,
	FS_NORMAL,
	FPF_DISKFONT|FPF_DESIGNED
};

UWORD pens[NUMDRIPENS + 1] = { 0 };

struct Screen *openScreen(struct BitMap **bm, struct TextFont **tf)
{
	if (*bm = AllocBitMap(640, 512, DEPTH, BMF_DISPLAYABLE|BMF_INTERLEAVED|BMF_CLEAR, NULL))
	{
		struct Screen *s;
		struct Rectangle dclip = { 0, 0, 639, 511 };

		if (*tf = OpenDiskFont(&ta))
		{
			pens[BACKGROUNDPEN] = 22;
			pens[SHINEPEN] = 24;
			pens[SHADOWPEN] = 25;
			pens[TEXTPEN] = 25;
			pens[FILLPEN] = 20;
			pens[FILLTEXTPEN] = 24;
			pens[NUMDRIPENS] = ~0;

			if (s = OpenScreenTags(NULL,
				SA_BitMap,		*bm,
				SA_Font,		&ta,
				SA_DisplayID,	DBLPAL_MONITOR_ID|DBLPALHIRESFF_KEY,
				SA_DClip,		&dclip,
				SA_BackFill,	LAYERS_NOBACKFILL,
				SA_Quiet,		TRUE,
				SA_ShowTitle,	FALSE,
				SA_Exclusive,	TRUE,
				SA_Title,		"Game Screen",
				SA_Pens,		pens,
				TAG_DONE))
			{
				return(s);
			}
			CloseFont(*tf);
		}
		FreeBitMap(*bm);
	}
	return(NULL);
}

void closeScreen(struct Screen *s, struct BitMap *bm, struct TextFont *tf)
{
	CloseScreen(s);
	CloseFont(tf);
	FreeBitMap(bm);
}

void drawFrame(struct Gadget *prev, struct Window *w)
{
	SetAPen(w->RPort, 20);
	Move(w->RPort, prev->LeftEdge - 1, prev->TopEdge - 1);
	Draw(w->RPort, prev->LeftEdge + prev->Width, prev->TopEdge - 1);
	Draw(w->RPort, prev->LeftEdge + prev->Width, prev->TopEdge + prev->Height);
	Draw(w->RPort, prev->LeftEdge - 1, prev->TopEdge + prev->Height);
	Draw(w->RPort, prev->LeftEdge - 1, prev->TopEdge);
}

void game(struct Screen *s, struct BitMap *gfx)
{
	struct Window *w;

	if (w = OpenWindowTags(NULL,
		WA_CustomScreen,	s,
		WA_Left,			0,
		WA_Top,				0,
		WA_Width,			s->Width,
		WA_Height,			s->Height,
		WA_Backdrop,		TRUE,
		WA_Borderless,		TRUE,
		WA_Activate,		TRUE,
		WA_SmartRefresh,	TRUE,
		WA_IDCMP,			IDCMP_RAWKEY,
		TAG_DONE))
	{
		struct VisualInfo *vi;

		if (vi = GetVisualInfoA(s, NULL))
		{
			struct NewGadget ng;
			struct Gadget *prev, *glist;

			ng.ng_LeftEdge = 120;
			ng.ng_TopEdge = 40;
			ng.ng_Width = 160;
			ng.ng_Height = 14;
			ng.ng_GadgetText = "Nowa gra";
			ng.ng_VisualInfo = vi;
			ng.ng_TextAttr = &ta;
			ng.ng_Flags = PLACETEXT_IN;
			ng.ng_GadgetID = 1;
			ng.ng_UserData = NULL;

			prev = CreateContext(&glist);
			prev = CreateGadget(BUTTON_KIND, prev, &ng, TAG_DONE);

			ng.ng_TopEdge += ng.ng_Height + 1;
			ng.ng_GadgetText = "Wczytaj stan";

			prev = CreateGadget(BUTTON_KIND, prev, &ng, TAG_DONE);

			ng.ng_TopEdge += ng.ng_Height + 1;
			ng.ng_GadgetText = "Zapisz stan";

			prev = CreateGadget(BUTTON_KIND, prev, &ng,
				GA_Disabled,	TRUE,
				TAG_DONE);

			AddGList(w, glist, -1, -1, NULL);

			struct BitScaleArgs bsa = { 0 };

			bsa.bsa_SrcBitMap = gfx;
			bsa.bsa_DestBitMap = s->RastPort.BitMap;
			bsa.bsa_SrcX = 0;
			bsa.bsa_SrcY = 16;
			bsa.bsa_SrcWidth = 32;
			bsa.bsa_SrcHeight = 16;
			bsa.bsa_XSrcFactor = bsa.bsa_YSrcFactor = 1;
			bsa.bsa_XDestFactor = bsa.bsa_YDestFactor = 2;

			WORD x, y;

			bltTileRastPort(gfx, 0, 0, w->RPort, 0, 0, 640, 64);
			bltTileRastPort(gfx, 0, 64, w->RPort, 0, 64, 16 << 5, 14 << 5);
			bltTileRastPort(gfx, 16 << 5, 64, w->RPort, 16 << 5, 64, 4 << 5, 14 << 5);

			Delay(50);

			WaitPort(w->UserPort);

			RemoveGList(w, glist, -1);
			FreeGadgets(glist);
			FreeVisualInfo(vi);
		}


		CloseWindow(w);
	}
}

int main()
{
	struct Screen *s;
	struct BitMap *bm;
	struct TextFont *tf;

	if (s = openScreen(&bm, &tf))
	{
		struct BitMap *gfx;
		struct IFFHandle *iff;
		BOOL clip;
		struct BitMapHeader *bmhd;
		LONG err;

		if (iff = openILBM("Data/Game.iff", &clip, &err, &bmhd))
		{
			if (loadColorMap(iff, s->ViewPort.ColorMap))
			{
				if (gfx = loadBitMap(iff, bmhd))
				{
					MakeScreen(s);
					RethinkDisplay();
					game(s, gfx);
					FreeBitMap(gfx);
				}
			}
			closeIFF(iff);
		}
		closeScreen(s, bm, tf);
	}
	return(0);
}
