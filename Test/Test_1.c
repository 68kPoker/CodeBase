
#define NEWCLIPRECTS_1_1

#include <intuition/intuition.h>
#include <exec/memory.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>

struct Screen *openScreen()
{
	struct Screen *s;

	s = OpenScreenTags(NULL,
		SA_LikeWorkbench, TRUE,
		SA_Title,	"Magazyn",
		TAG_DONE);

	return s;
}

void moveWindow(struct Window *w, WORD dx, WORD dy)
{
	/* Calc update region (DamageList) */
	struct Layer *l;
	struct Region *reg;
	struct Rectangle *rect = &w->WLayer->bounds;

	if (reg = NewRegion())
	{
		OrRectRegion(reg, rect);
		rect->MinX += dx;
		rect->MinY += dy;
		rect->MaxX += dx;
		rect->MaxY += dy;

		w->WLayer->ClipRect->bounds = *rect;
		w->LeftEdge += dx;
		w->TopEdge += dy;

		/* Old - new */
		ClearRectRegion(reg, rect);

		for (l = w->WLayer->back; l != NULL; l = l->back)
		{
			struct Region *aux;

			if (l == w->WScreen->BarLayer)
				continue;

			if (aux = NewRegion())
			{
				rect = &l->bounds;
				OrRectRegion(aux, rect);
				AndRegionRegion(reg, aux);
				XorRegionRegion(aux, reg);


				struct RegionRectangle *rr;

				aux->bounds.MinX -= l->bounds.MinX;
				aux->bounds.MinY -= l->bounds.MinY;
				aux->bounds.MaxX -= l->bounds.MinX;
				aux->bounds.MaxY -= l->bounds.MinY;

				for (rr = aux->RegionRectangle; rr != NULL; rr = rr->Next)
				{

				}

				OrRegionRegion(aux, l->DamageList);


				l->Flags |= LAYERREFRESH;
				DisposeRegion(aux);
			}
		}

		ClearRegion(reg);

		rect = &w->WLayer->bounds;
		OrRectRegion(reg, rect);

		for (l = w->WLayer->back; l != NULL; l = l->back)
		{
			struct Region *aux;

			if (l == w->WScreen->BarLayer)
				continue;

			OrRectRegion(reg, rect);

			if (aux = NewRegion())
			{
				struct Rectangle *back = &l->bounds;
				OrRectRegion(aux, back);

				AndRegionRegion(aux, reg);

				XorRegionRegion(reg, aux);

				struct RegionRectangle *rr;

				struct ClipRect *cr, *prev = NULL, *cr2, *next;

				for (rr = aux->RegionRectangle; rr != NULL; rr = rr->Next)
				{
					if (cr = AllocMem(sizeof(*cr), MEMF_PUBLIC|MEMF_CLEAR))
					{
						struct Rectangle r;

						r.MinX = aux->bounds.MinX + rr->bounds.MinX;
						r.MinY = aux->bounds.MinY + rr->bounds.MinY;
						r.MaxX = aux->bounds.MinX + rr->bounds.MaxX;
						r.MaxY = aux->bounds.MinY + rr->bounds.MaxY;

						cr->bounds = r;

						cr->Next = prev;
						prev = cr;
					}
				}

				for (cr2 = l->ClipRect; cr2 != NULL; cr2 = next)
				{
					next = cr2->Next;
					FreeMem(cr2, sizeof(*cr2));
				}

				l->ClipRect = cr;

				DisposeRegion(aux);
			}
		}
		DisposeRegion(reg);
	}
}

void test(struct Window *bw)
{
	struct Window *w;
	struct Screen *s = bw->WScreen;

	w = OpenWindowTags(NULL,
		WA_CustomScreen,	s,
		WA_Left,			0,
		WA_Top,				s->BarHeight + 1,
		WA_Width,			320,
		WA_Height,			160,
		WA_Title,			"Magazyn",
		WA_SimpleRefresh,	TRUE,
		TAG_DONE);

	if (w)
	{
		Delay(200);
		BltBitMap(w->RPort->BitMap, w->LeftEdge, w->TopEdge, w->RPort->BitMap, w->LeftEdge + 64, w->TopEdge + 64, w->Width, w->Height, 0xc0, 0xff, NULL);
		moveWindow(w, 64, 64);


		BeginUpdate(bw->WLayer);

		SetRast(bw->WLayer->rp, 0);

		EndUpdate(bw->WLayer, TRUE);

		Delay(400);


		CloseWindow(w);
	}


}



main()
{
	struct Screen *s;
	struct Window *w;

	if (s = openScreen())
	{
		if (w = OpenWindowTags(NULL,
			WA_CustomScreen,	s,
			WA_Left,			0,
			WA_Top,				s->BarHeight + 1,
			WA_Width,			s->Width,
			WA_Height,			s->Height - (s->BarHeight + 1),
			WA_Backdrop,		TRUE,
			WA_Borderless,		TRUE,
			WA_SimpleRefresh,	TRUE,
			TAG_DONE))
		{
			test(w);
			CloseWindow(w);
		}
		CloseScreen(s);
	}
	return 0;
}



