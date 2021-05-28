
#include <intuition/intuition.h>

#include "Intui.h"

void setupText(struct IntuiText *it, UBYTE fpen, UBYTE bpen, UBYTE drmd, WORD left, WORD top, struct TextAttr *ta, UBYTE *text, struct IntuiText *next)
{
	it->FrontPen  = fpen;
	it->BackPen	  = bpen;
	it->DrawMode  = drmd;
	it->LeftEdge  = left;
	it->TopEdge	  = top;
	it->ITextFont = ta;
	it->IText	  = text;
	it->NextText  = next;
}

void setupImage(struct Image *img, WORD left, WORD top, WORD width, WORD height, WORD depth, UWORD *data, UBYTE pick, UBYTE onoff, struct Image *next)
{
	img->LeftEdge 	= left;
	img->TopEdge  	= top;
	img->Width	  	= width;
	img->Height	  	= height;
	img->Depth	  	= depth;
	img->ImageData 	= data;
	img->PlanePick 	= pick;
	img->PlaneOnOff = onoff;
	img->NextImage	= next;
}

void setupGadget(struct Gadget *gad, struct Gadget *next, WORD left, WORD top, WORD width, WORD height, UWORD flags, UWORD act, UWORD type, APTR render, APTR select, struct IntuiText *it, APTR info, UWORD id, APTR user)
{
	gad->NextGadget 	= next;
	gad->LeftEdge 		= left;
	gad->TopEdge		= top;
	gad->Width			= width;
	gad->Height			= height;
	gad->Flags			= flags;
	gad->Activation		= act;
	gad->GadgetType		= type;
	gad->GadgetRender 	= render;
	gad->SelectRender 	= select;
	gad->GadgetText	  	= it;
	gad->MutualExclude 	= 0;
	gad->SpecialInfo  	= info;
	gad->GadgetID	  	= id;
	gad->UserData	  	= user;
}
