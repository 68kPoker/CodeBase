
#define B_WIDTH  20
#define B_HEIGHT 16

enum {
    T_FLOOR,
    T_WALL,
    T_OBJECT
};

enum {
    F_NORMAL,
    F_FLAGSTONE
};

enum {
    W_SOLID,
    W_DOOR
};

enum {
    O_BOX,
    O_HERO,
    O_SCROLL,
    O_KEY
};

enum {
    R_ENTER,
    R_BLOCK
};

struct field {
    int type;
    union {
        int floor, wall, object;
    } sub;
    int floor;
};

struct board {
    int placedBoxes, allBoxes, keys;
    struct field array[ B_HEIGHT ][ B_WIDTH ];
};

int enter( struct board *bd, struct field *dest, struct field *src );
int leave( struct board *bd, struct field *src );
