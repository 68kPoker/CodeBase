
struct Screen *openScreen(struct BitMap *bm, ULONG *colors)
{
    struct Screen *s;
    struct Rectangle dclip =
    {
        0, 0, 319, 255
    };

    if (s = OpenScreenTags(NULL,
        SA_DClip,       &dclip,
        SA_BitMap,      bm,
        SA_Colors32,    colors,
        SA_DisplayID,   LORES_KEY,
        SA_BackFill,    LAYERS_NOBACKFILL,
        SA_Title,       "Magazyn",
        SA_ShowTitle,   FALSE,
        SA_Exclusive,   TRUE,
        SA_Quiet,       TRUE,
        TAG_DONE))
    {
        return(s);
    }
    return(NULL);
}
