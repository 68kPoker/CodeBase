
#include <stdio.h>

#include <dos/dos.h>
#include <intuition/intuition.h>
#include <graphics/rpattr.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>
#include <clib/layers_protos.h>

#include "Main.h"

#include "debug.h"

/* Start z AmigaDOS i Workbencha */
int main(void);
int wbmain(struct WBStartup *wbs);

struct Library *IntuitionBase, *GfxBase, *DiskfontBase, *IFFParseBase;

void showMessage(STRPTR msg)
{
    struct EasyStruct es =
    {
        sizeof(struct EasyStruct),
        0,
        "Game Information",
        0,
        "OK"
    };

    es.es_TextFormat = msg;

    EasyRequestArgs(NULL, &es, NULL, NULL);
}

void closeRomLibs(void)
{
    CloseLibrary(GfxBase);
    CloseLibrary(IntuitionBase);
}

BOOL openRomLibs(void)
{
    if (!(IntuitionBase = OpenLibrary("intuition.library", 39)))
        showMessage("Couldn't open intuition.library V39!");
    else
    {
        if (!(GfxBase = OpenLibrary("graphics.library", 39)))
            showMessage("Couldn't open graphics.library V39!");
        else
        {
            return(TRUE);
        }
        CloseLibrary(IntuitionBase);
    }
    return(FALSE);
}

void closeDiskLibs(void)
{
    CloseLibrary(IFFParseBase);
    CloseLibrary(DiskfontBase);
}

BOOL openDiskLibs(void)
{
    if (!(DiskfontBase = OpenLibrary("diskfont.library", 39)))
        showMessage("Couldn't open diskfont.library V39!");
    else
    {
        if (!(IFFParseBase = OpenLibrary("iffparse.library", 39)))
            showMessage("Couldn't open iffparse.library V39!");
        else
        {
            return(TRUE);
        }
        CloseLibrary(DiskfontBase);
    }
    return(FALSE);
}

BOOL handleIDCMP(struct mainData *md, struct Window *w)
{
    struct IntuiMessage *msg;

    while (msg = GT_GetIMsg(w->UserPort))
    {
        ULONG class = msg->Class;
        UWORD code = msg->Code;
        WORD mx = msg->MouseX;
        WORD my = msg->MouseY;
        APTR iaddr = msg->IAddress;

        GT_ReplyIMsg(msg);

        if (class == IDCMP_RAWKEY)
        {
            if (code == ESC_KEY)
                return(TRUE);
        }
        else if (class == IDCMP_GADGETDOWN)
        {
            struct Gadget *gad = (struct Gadget *)iaddr;
            struct windowData *wd = &md->screen.wd[WID_BOARD];

            wd->gads[gad->GadgetID].Flags ^= GFLG_SELECTED;

            if (wd->selected >= 0)
            {
                wd->gads[wd->selected].Flags &= ~GFLG_SELECTED;

                closeWindow(&md->screen, md->screen.menuw);
                md->screen.menuw = NULL;

                md->screen.update = TRUE;
/*
                BeginUpdate(md->screen.bdw->WLayer);
                SetRast(md->screen.bdw->RPort, 0);
                EndUpdate(md->screen.bdw->WLayer, TRUE);
*/
            }

            RefreshGList(wd->gads + gad->GadgetID, w, NULL, 1);

            if (wd->selected >= 0)
            {
                RefreshGList(wd->gads + wd->selected, w, NULL, 1);
            }

            if (wd->gads[gad->GadgetID].Flags & GFLG_SELECTED)
            {
                wd->selected = gad->GadgetID;
            }
            else
            {
                wd->selected = -1;
            }

            if (wd->selected >= 0)
            {
                openMenuWindow(&md->screen, wd->selected);
            }
        }
    }
    return(FALSE);
}

void mainLoop(struct mainData *md)
{
    ULONG sig[SIGSRC_COUNT], total = 0L;
    WORD i;
    BOOL done = FALSE;

    sig[SIGSRC_COPPER] = 1L << md->screen.copData.signal;
    sig[SIGSRC_SAFE]   = 1L << md->screen.mp->mp_SigBit;
    sig[SIGSRC_IDCMP1] = 1L << md->screen.bdw->UserPort->mp_SigBit;

    for (i = 0; i < SIGSRC_COUNT; i++)
    {
        total |= sig[i];
    }

    while (!done)
    {
        ULONG result = Wait(total);

        if (result & sig[SIGSRC_COPPER])
        {
            if (md->screen.safe)
            {
                /* Przeîâcz (lub rysuj) */
                struct RastPort *rp = md->screen.bdw->RPort;
                UBYTE text[5];
                static WORD counter = 0;

                sprintf(text, "%4d", counter++);

                Move(rp, 260, rp->Font->tf_Baseline);
                SetABPenDrMd(rp, 2, 1, JAM2);
                Text(rp, text, 4);

                if (md->screen.update)
                {
                    if (md->screen.menuw)
                    {
                        struct RastPort *rp = md->screen.menuw->RPort;

                        SetOutlinePen(rp, 1);
                        SetAPen(rp, 3);
                        RectFill(rp, 0, 0, md->screen.menuw->Width - 1, md->screen.menuw->Height - 1);
                    }

                    if (md->screen.bdw->WLayer->Flags & LAYERREFRESH)
                    {
                        struct Rectangle rect;
                        GetRPAttrs(md->screen.bdw->RPort, RPTAG_DrawBounds, &rect, TAG_DONE);

                        BeginUpdate(md->screen.bdw->WLayer);
                        SetAPen(md->screen.bdw->RPort, 0);
                        RectFill(md->screen.bdw->RPort, rect.MinX, rect.MinY, rect.MaxX, rect.MaxY);
                        EndUpdate(md->screen.bdw->WLayer, TRUE);
                    }

                    md->screen.update = FALSE;
                }
            }
        }

        if (result & sig[SIGSRC_SAFE])
        {
            if (!md->screen.safe)
            {
                while (!GetMsg(md->screen.mp))
                    WaitPort(md->screen.mp);
                md->screen.safe = TRUE;
            }
            /* Rysuj */

        }

        if (result & sig[SIGSRC_IDCMP1])
        {
            done = handleIDCMP(md, md->screen.bdw);
        }
    }
}

void freeAll(struct mainData *md)
{
    if (md->screen.menuw)
    {
        closeWindow(&md->screen, md->screen.menuw);
    }
    closeScreenFont(&md->screen);
    closeDiskLibs();
    closeRomLibs();
}

BOOL initAll(struct mainData *md)
{
    if (openRomLibs())
    {
        if (openDiskLibs())
        {
            if (openScreenFont(md, &md->screen))
            {
                md->screen.menuw = NULL;
                md->screen.update = FALSE;
                return(TRUE);
            }
            closeDiskLibs();
        }
        closeRomLibs();
    }
    return(FALSE);
}

int main(void)
{
    static struct mainData mainData; /* Zasoby potrzebne na czas dziaîania programu */

    /* Alokacja zasobów potrzebnych na czas dziaîania programu */
    if (initAll(&mainData))
    {
        mainLoop(&mainData);
        freeAll(&mainData);
        return(RETURN_OK);
    }

    return(RETURN_FAIL);
}
