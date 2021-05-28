
#include <intuition/intuition.h>
#include <clib/intuition_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>

#include "ILBM.h"

struct Screen *openScreen(struct TextFont **tf, struct ScreenBuffer *sb[])
{
	static struct TextAttr ta =
	{
		"centurion.font",
		9,
		FS_NORMAL,
		FPF_DISKFONT|FPF_DESIGNED
	};

	if (*tf = OpenDiskFont(&ta))
	{
		struct Screen *s;
		struct Rectangle dclip = { 0, 0, 319, 255 };

		if (s = OpenScreenTags(NULL,
			SA_Font,		&ta,
			SA_DClip,		&dclip,
			SA_Depth,		5,
			SA_DisplayID,	LORES_KEY,
			SA_Quiet,		TRUE,
			SA_ShowTitle,	FALSE,
			SA_Exclusive,	TRUE,
			SA_BackFill,	LAYERS_NOBACKFILL,
			SA_Interleaved,	TRUE,
			TAG_DONE))
		{
			if (sb[0] = AllocScreenBuffer(s, NULL, SB_SCREEN_BITMAP))
			{
				if (sb[1] = AllocScreenBuffer(s, NULL, 0))
				{
					return(s);
				}
				FreeScreenBuffer(s, sb[0]);
			}
			CloseScreen(s);
		}
		CloseFont(*tf);
	}
	return(NULL);
}

void closeScreen(struct Screen *s, struct TextFont *tf, struct ScreenBuffer *sb[])
{
	FreeScreenBuffer(s, sb[1]);
	FreeScreenBuffer(s, sb[0]);
	CloseScreen(s);
	CloseFont(tf);
}

void test()
{
	struct Screen *s;
	struct TextFont *tf;
	struct ScreenBuffer *sb[2];

	if (s = openScreen(&tf, sb))
	{
		struct BitMap *gfx;
		struct ColorMap *cm = s->ViewPort.ColorMap;

		if (loadILBM("Data/Graphics.iff", &gfx, &cm))
		{
			MakeScreen(s);
			RethinkDisplay();
			BltBitMap(gfx, 0, 0, s->RastPort.BitMap, 0, 0, 320, 256, 0xc0, 0xff, NULL);

			Delay(300);
			freeILBM(gfx, NULL);
		}
		closeScreen(s, tf, sb);
	}
}

main()
{
	test();
}
