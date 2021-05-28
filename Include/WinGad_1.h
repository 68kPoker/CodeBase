
#ifndef WINGAD_H
#define WINGAD_H

#include <intuition/classes.h>
#include <intuition/classusr.h>

#define LEFT_KEY    0x4f
#define RIGHT_KEY   0x4e
#define DOWN_KEY    0x4d
#define UP_KEY      0x4c

#define IA_BitMap (TAG_USER + 1)
#define IA_Points (TAG_USER + 2)

enum /* Images */
{
    IID_CLOSE,
    IID_DEPTH,
    IID_PANEL,
    IID_EDITOR,
    IID_OPTIONS,
    IID_TILES,
    IID_NEW,
    IID_LOAD,
    IID_SAVE,
    IID_TEST,
    IID_COUNT
};

enum /* Main menu gadgets */
{
    GID_CLOSE,
    GID_DEPTH,
    GID_PANEL,
    GID_EDITOR,
    GID_OPTIONS,
    GID_COUNT
};

enum /* Auxilliary menu gadgets */
{
    MID_CLOSE,
    MID_BUTTON1,
    MID_BUTTON2,
    MID_BUTTON3,
    MID_BUTTON4,
    MID_COUNT
};

typedef struct Window WIN;
typedef struct windowInfo WININFO;
typedef struct menuInfo MENUINFO;

struct windowInfo
{
    struct Window *window;
    struct DrawInfo *dri;
    struct Object *buttons[GID_COUNT];
};

struct menuInfo
{
    struct Window *window;
    struct DrawInfo *dri;
    struct Object *buttons[MID_COUNT];
};

#endif /* WINGAD_H */
