
/* Wyliczenie elementów dla edytora plansz */
enum
{
    FLOOR_KIND,
    HERO_KIND,
    BOX_KIND,
    WALL_KIND,
    FLAGSTONE_KIND,
    KINDS
};

struct editBoard
{
    int kinds[16][20];
};
