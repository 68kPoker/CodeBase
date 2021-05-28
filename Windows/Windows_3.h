
/* Window UserData */

#include <exec/types.h>
#include <exec/interrupts.h>

#define DEPTH  5
#define MODEID LORES_KEY

enum
{
	MTYPE_IDCMP,
	MTYPE_VBLANK,
	MTYPE_GAMEPORT,
	MTYPES
};

union message
{
	struct IntuiMessage *msg;
	struct InputEvent	*ie;
	struct RastPort		*rp;
};

struct windowUD
{
	/* Dispatcher - called upon IDCMP, gameport, VBlank etc. */
	LONG (*dispatch)(struct Window *w, WORD type, union message *wm);
};

struct copperInfo
{
	struct ViewPort *vp;
	WORD signal;
	struct Task *task;
};

struct screenUD
{
	struct Interrupt is;
	struct copperInfo ci;
};

/* Protos */

/* Open main backdrop window */

struct Window *openMainWindow(struct Screen *s, struct windowUD *wud);

/* Open screen and install copper */

struct Screen *openScreen(struct screenUD *sud);
void closeScreen(struct Screen *s);

/* Main event loop */

LONG mainLoop(struct Window *w);
