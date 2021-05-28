
struct screenInfo
{
    struct BitMap *bm[2];
    struct ScreenBuffer *sb[2];
};

struct Screen *otworzEkran(void);
void zamknijEkran(struct Screen *s);
