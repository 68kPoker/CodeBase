
/* Magazyn */

/* Moje View */

#ifndef VIEW_H
#define VIEW_H

#include <graphics/view.h>

typedef struct View VIEW;
typedef struct ViewPort VPORT;
typedef struct RasInfo RASINFO;
typedef struct ColorMap *CMAP;
typedef struct BitMap *BITMAP;

typedef struct ViewExtra *VEXTRA;
typedef struct ViewPortExtra *VPEXTRA;
typedef struct MonitorSpec *MONSPEC;

typedef struct viewPack
    {
    VIEW view;
    VPORT vp;
    RASINFO ri;
    VEXTRA ve;
    VPEXTRA vpe;
    MONSPEC monspec;
    } VPACK;

BOOL initView(VPACK *p, ULONG modeID, WORD width, WORD height, UBYTE depth);
VIEW *saveView(VOID);
VOID freeView(VPACK *p);

#endif /* VIEW_H */
