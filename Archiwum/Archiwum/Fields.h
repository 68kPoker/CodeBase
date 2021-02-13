
/* Fields */

#ifndef FIELDS_H
#define FIELDS_H

/* Floor main types */

enum floorTypes
{
	FLOOR_NONE,			/* Background */
	FLOOR_NORMAL,
	FLOOR_WALL,
	FLOOR_FLAGSTONE,
	FLOOR_DOOR,			/* Openable door */
	FLOOR_SLIDER,		/* Moves box */
	FLOOR_TYPES
};

/* Flagstone (and box) types */

enum flagstoneTypes
{
	FLAGSTONE_SQUARE,
	FLAGSTONE_TRIANGLE,
	FLAGSTONE_CIRCLE,
	FLAGSTONE_TYPES
};

/* Directions */

enum
{
	DIR_LEFT,
	DIR_RIGHT,
	DIR_UP,
	DIR_DOWN
};

enum objectTypes
{
	OBJECT_NONE,
	OBJECT_BOX,
	OBJECT_ITEM, 		/* Collectable item */
	OBJECT_PLAYER,
	OBJECT_SKULL,
	OBJECT_TYPES
};

enum itemTypes
{
	ITEM_COINS,
	ITEM_FRUITS,
	ITEM_KEY, 			/* Key to unlock door */
	ITEM_TYPES
};

/* Object appearance */

enum
{
	APPEAR_START,
	APPEAR_BOXES_PLACED,	/* Boxes of given type are placed */
	APPEAR_TYPES
};

/* Floor behavior */

enum
{
	BEHAVIOR_NORMAL,
	BEHAVIOR_CHANGEABLE,
	BEHAVIOR_TYPES
};

/* Box placing actions */

enum
{
	ACTION_NONE,
	ACTION_APPEAR_OBJECT,
	ACTION_REMOVE_WALL,
	ACTION_TYPES
};

/* Key (and door) types */

enum keyTypes
{
	KEY_GOLD,
	KEY_SILVER,
	KEY_BRONZE,
	KEY_TYPES
};

#endif /* FIELDS_H */
