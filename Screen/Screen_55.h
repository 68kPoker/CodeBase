
#include <exec/types.h>
#include <exec/interrupts.h>

#define COPERLIST_LEN 3
#define COPERINTS_PRI 0

typedef struct copper_data
{
    struct ViewPort     *vp;
    UWORD               signal;
    struct Task         *task;

} copper_data;

typedef struct screen
{
    struct Screen       *s;
    struct ScreenBuffer *sb[2];
    struct MsgPort      *sp;
    BOOL                safe;
    UWORD               frame;

    struct Interrupt    cis; /* Copper server */
    copper_data         cis_data;

} screen;

extern void my_copper(void);

extern BOOL open_screen (screen *s);
extern void close_screen(screen *s);
