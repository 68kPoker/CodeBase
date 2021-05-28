
#ifndef GUI_H
#define GUI_H

#ifndef GRAPHICS_GFX_H
#include <graphics/gfx.h>
#endif

/* Structure used by Layer's BackFill Hook */
struct layerBackFill
{
    struct Layer     *layer;
    struct Rectangle bounds;
    LONG             x_offset, y_offset;
};

/* Structure used by LayerInfo's BackFill Hook */
struct layerInfoBackFill
{
    LONG             dummy;
    struct Rectangle bounds;
};

#endif /* GUI_H */
