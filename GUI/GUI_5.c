
/* Init empty image of given size and color */

void emptyImage(struct Image *img, WORD width, WORD height, UBYTE color)
{
    img->ImageData  = NULL;
    img->LeftEdge   = 0;
    img->TopEdge    = 0;
    img->Width      = width;
    img->Height     = height;
    img->Depth      = 0;
    img->PlanePick  = 0;
    img->PlaneOnOff = color;
    img->NextImage  = NULL;
}


