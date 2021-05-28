
/* Gearwork Libraries */

/* Use these to open required libraries */

#include <exec/types.h>

#define GWprint printf

/* It's configured on compile-time. Just include proper module headers,
   like GWScreen.h etc. */

/* You can provide min. OS version */

BOOL GWopenLibs(ULONG minVersion); /* Open all required libraries */

VOID GWerror(STRPTR errorDesc);
VOID GWbailout(STRPTR errorDesc);

VOID GWcleanup(VOID);
