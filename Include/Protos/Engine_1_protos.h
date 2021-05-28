
#ifndef ENGINE_PROTOS_H
#define ENGINE_PROTOS_H

#include <exec/types.h>

int enter( struct board *bd, struct field *dest, struct field *src );
int leave( struct board *bd, struct field *src );
BOOL convertBoard(struct board *bd, struct editBoard *eb);

#endif /* ENGINE_PROTOS_H */
