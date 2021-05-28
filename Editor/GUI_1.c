
#include <intuition/intuition.h>
#include <datatypes/pictureclass.h>

#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/datatypes_protos.h>

#include "Checkmark.h"
#include "Podloga.h"
#include "PodlogaS.h"
#include "Sciana.h"
#include "ScianaS.h"
#include "Skrzynia.h"
#include "SkrzyniaS.h"
#include "Skarb.h"
#include "SkarbS.h"
#include "Tlo.h"
#include "TloS.h"
#include "Bohater.h"
#include "BohaterS.h"

#define SZER 80
#define WYS  20
#define PRZES 4
#define PRZESX 4
#define ZNACZEK 16

#define DEPTH 5

struct IntuiText otworzTekst = {
    24, 0,
    JAM1,
    PRZESX, PRZES,
    NULL,
    "Otworz"
};

struct IntuiText zapiszTekst = {
    24, 0,
    JAM1,
    PRZESX, PRZES,
    NULL,
    "Zapisz"
};

struct IntuiText nowyTekst = {
    24, 0,
    JAM1,
    PRZESX, PRZES,
    NULL,
    "Nowy"
};

struct IntuiText zakonczTekst = {
    24, 1,
    JAM1,
    PRZESX, PRZES,
    NULL,
    "Zakoncz"
};

struct IntuiText tloTekst = {
    24, 0,
    JAM1,
    ZNACZEK, PRZES,
    NULL,
    "Tîo"
};

struct IntuiText podlogaTekst = {
    24, 0,
    JAM1,
    ZNACZEK, PRZES,
    NULL,
    "Podîoga"
};

struct IntuiText scianaTekst = {
    24, 0,
    JAM1,
    ZNACZEK, PRZES,
    NULL,
    "Ôciana"
};

struct IntuiText skrzyniaTekst = {
    24, 0,
    JAM1,
    ZNACZEK, PRZES,
    NULL,
    "Skrzynia"
};

struct IntuiText miejsceTekst = {
    24, 0,
    JAM1,
    ZNACZEK, PRZES,
    NULL,
    "Miejsce"
};

struct IntuiText bohaterTekst = {
    24, 0,
    JAM1,
    ZNACZEK, PRZES,
    NULL,
    "Bohater"
};

struct MenuItem projektZakoncz = {
    NULL,
    0, WYS * 4, SZER, WYS,
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,
    &zakonczTekst,
    NULL,
    0,
    NULL,
};

struct MenuItem projektZapisz = {
    &projektZakoncz,
    0, WYS * 2, SZER, WYS,
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,
    &zapiszTekst,
    NULL,
    0,
    NULL,
};

struct MenuItem projektOtworz = {
    &projektZapisz,
    0, WYS, SZER, WYS,
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,
    &otworzTekst,
    NULL,
    0,
    NULL,
};

struct MenuItem projektNowy = {
    &projektOtworz,
    0, 0, SZER, WYS,
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,
    &nowyTekst,
    NULL,
    0,
    NULL,
};

struct MenuItem planszaBohater = {
    NULL,
    0, WYS * 5, SZER, WYS,
    ITEMENABLED | CHECKIT | HIGHIMAGE,
    ~32,
    &Bohater,
    &BohaterS,
    0,
    NULL,
};

struct MenuItem planszaMiejsce = {
    &planszaBohater,
    0, WYS * 4, SZER, WYS,
    ITEMENABLED | CHECKIT | HIGHIMAGE,
    ~16,
    &Skarb,
    &SkarbS,
    0,
    NULL,
};

struct MenuItem planszaSkrzynia = {
    &planszaMiejsce,
    0, WYS * 3, SZER, WYS,
    ITEMENABLED | CHECKIT | HIGHIMAGE,
    ~8,
    &Skrzynia,
    &SkrzyniaS,
    0,
    NULL,
};

struct MenuItem planszaSciana = {
    &planszaSkrzynia,
    0, WYS * 2, SZER, WYS,
    ITEMENABLED | CHECKIT | HIGHIMAGE,
    ~4,
    &Sciana,
    &ScianaS,
    0,
    NULL,
};

struct MenuItem planszaPodloga = {
    &planszaSciana,
    0, WYS, SZER, WYS,
    ITEMENABLED | CHECKIT | CHECKED | HIGHIMAGE,
    ~2,
    &Podloga,
    &PodlogaS,
    0,
    NULL,
};

struct MenuItem planszaTlo = {
    &planszaPodloga,
    0, 0, SZER, WYS,
    ITEMENABLED | CHECKIT | HIGHIMAGE,
    ~1,
    &Tlo,
    &TloS,
    0,
    NULL,
};

struct Menu menuPlansza = {
    NULL,
    SZER + 4, 0,
    SZER, WYS,
    MENUENABLED,
    "Plansza",
    &planszaTlo
};

struct Menu menuProjekt = {
    &menuPlansza,
    4, 0,
    SZER, WYS,
    MENUENABLED,
    "Projekt",
    &projektNowy
};

struct Window *otworzOkno( struct TextAttr *ta )
{
    struct Screen *scr;
    UWORD pens[] = { ~0 };

    if( scr = OpenScreenTags( NULL,
        SA_DisplayID,   LORES_KEY,
        SA_Font,        ta,
        SA_Quiet,       TRUE,
        SA_ShowTitle,   FALSE,
        SA_Exclusive,   TRUE,
        SA_SharePens,   TRUE,
        SA_Depth,       DEPTH,
        SA_Pens,        pens,
        SA_Title,       "Magazyn (C)'20 Robert Szacki",
        SA_Interleaved, TRUE,
        SA_BackFill,    LAYERS_NOBACKFILL,
        TAG_DONE ) ) {
        struct Window *win;

        if( win = OpenWindowTags( NULL,
            WA_CustomScreen,    scr,
            WA_Left,    0,
            WA_Top,     0,
            WA_Width,   scr->Width,
            WA_Height,  scr->Height,
            WA_SmartRefresh,   TRUE,
            WA_Backdrop,    TRUE,
            WA_Borderless,  TRUE,
            WA_IDCMP,   IDCMP_RAWKEY,
            WA_DetailPen,   24,
            WA_BlockPen,    22,
            WA_Activate,    TRUE,
            WA_BackFill,    LAYERS_NOBACKFILL,
            WA_Checkmark,   &Checkmark,
            TAG_DONE ) ) {
            SetMenuStrip( win, &menuProjekt );

            return( win );
        }
        CloseScreen( scr );
    }
    return( NULL );
}

void zamknijOkno( struct Window *win )
{
    struct Screen *scr = win->WScreen;

    CloseWindow( win );
    CloseScreen( scr );
}

Object *wczytajGrafike( STRPTR nazwa, struct Window *w )
{
    Object *o;

    if( o = NewDTObject( nazwa,
        DTA_GroupID,    GID_PICTURE,
        PDTA_Screen,    w->WScreen,
        PDTA_Remap,     FALSE,
        TAG_DONE ) ) {
        struct BitMap *bm;
        ULONG *cregs, numColors;
        WORD color;

        DoDTMethod( o, NULL, NULL, DTM_PROCLAYOUT, NULL, TRUE );
        GetDTAttrs( o,
            PDTA_CRegs,  &cregs,
            PDTA_BitMap, &bm,
            PDTA_NumColors, &numColors,
            TAG_DONE );

        for( color = 0; color < numColors; color++ ) {
            SetRGB32CM( w->WScreen->ViewPort.ColorMap, color, cregs[ 0 ], cregs[ 1 ], cregs[ 2 ] );
            cregs += 3;
        }
        MakeScreen( w->WScreen );
        RethinkDisplay();

        BltBitMapRastPort( bm, 0, 0, w->RPort, w->BorderLeft, w->BorderTop, 320, 256, 0xc0);
        return( o );
    }
    return( NULL );
}

void zwolnijGrafike( Object *o )
{
    DisposeDTObject( o );
}

int main()
{
    struct TextFont *tf;
    struct TextAttr ta = {
        "tny.font",
        8,
        FS_NORMAL,
        FPF_DISKFONT | FPF_DESIGNED
    };

    struct Window *w;
    Object *o;

    if( tf = OpenDiskFont( &ta ) ) {
        if( w = otworzOkno( &ta ) ) {
            if( o = wczytajGrafike( "Dane/Grafika.pic", w ) ) {
                Delay(1000);
                WaitPort( w->UserPort );
                zwolnijGrafike( o );
            }
            zamknijOkno( w );
        }
        CloseFont( tf );
    }
    return( 0 );

}
