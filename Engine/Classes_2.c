
/* My BOOPSI classes */

#include <intuition/imageclass.h>
#include <clib/intuition_protos.h>
#include <clib/alib_protos.h>
#include <clib/utility_protos.h>

#include "MyClasses.h"

extern ULONG HookEntry();

/* An Image with BitMap source and Selected and Disabled states */

struct myImage
{
    struct BitMap *bm;
    Point point[3];
};

__saveds ULONG dispatchMyImage(Class *cl, Object *o, Msg msg)
{
    struct myImage *data;
    APTR retval = NULL;

    switch (msg->MethodID)
    {
        case OM_NEW:
            if (retval = (APTR)DoSuperMethodA(cl, o, msg))
            {
                struct TagItem *tag, *taglist = ((struct opSet *)msg)->ops_AttrList;
                data = (struct myImage *)INST_DATA(cl, retval);

                while (tag = NextTagItem(&taglist))
                {
                    switch (tag->ti_Tag)
                    {
                        case IA_BitMap:
                            data->bm = (struct BitMap *)tag->ti_Data;
                            break;
                        case IA_Points:
                            Point *p = (Point *)tag->ti_Data;
                            data->point[IDS_NORMAL] = *p++;
                            data->point[IDS_SELECTED] = *p++;
                            data->point[IDS_DISABLED] = *p;
                            break;
                    }
                }
            }
            break;
        case IM_DRAW:
            struct impDraw *id = (struct impDraw *)msg;
            data = (struct myImage *)INST_DATA(cl, o);
            WORD state = id->imp_State;

            if (state >= IDS_NORMAL && state <= IDS_DISABLED)
            {
                BltBitMapRastPort(data->bm, data->point[state].x, data->point[id->imp_State].y, id->imp_RPort, id->imp_Offset.X + ((struct Image *)o)->LeftEdge, id->imp_Offset.Y + ((struct Image *)o)->TopEdge, ((struct Image *)o)->Width, ((struct Image *)o)->Height, 0xc0);
            }
            else if (state >= IDS_INACTIVENORMAL && state <= IDS_INACTIVEDISABLED)
            {
                state -= IDS_INACTIVENORMAL;
                BltBitMapRastPort(data->bm, data->point[state].x, data->point[id->imp_State].y, id->imp_RPort, id->imp_Offset.X + ((struct Image *)o)->LeftEdge, id->imp_Offset.Y + ((struct Image *)o)->TopEdge, ((struct Image *)o)->Width, ((struct Image *)o)->Height, 0xc0);
            }
            else if (state == IDS_SELECTEDDISABLED)
            {
                BltBitMapRastPort(data->bm, data->point[IDS_DISABLED].x, data->point[id->imp_State].y, id->imp_RPort, id->imp_Offset.X + ((struct Image *)o)->LeftEdge, id->imp_Offset.Y + ((struct Image *)o)->TopEdge, ((struct Image *)o)->Width, ((struct Image *)o)->Height, 0xc0);
            }
            break;
        default:
            retval = (APTR)DoSuperMethodA(cl, o, msg);
            break;
    }
    return((ULONG)retval);
}

Class *makeImage()
{
    Class *class;

    if (class = MakeClass(NULL, "imageclass", NULL, sizeof(struct myImage), 0))
    {
        /* Class made */

        /* Put dispatcher */
        class->cl_Dispatcher.h_Entry = HookEntry;
        class->cl_Dispatcher.h_SubEntry = (HOOKFUNC)dispatchMyImage;

        return(class);
    }
    return(NULL);
}
