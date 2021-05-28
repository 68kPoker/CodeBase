
/*
 * Magazyn
 * Menu.c - Funkcje menu gry
 */

#include <setjmp.h>
#include <stdio.h>

#include <exec/types.h>

#define MAX_MENUS 5
#define MAX_ITEMS 10

enum
{
	MAGAZYN,
	KAFELEK
};

enum
{
	ROZPOCZNIJ
};

jmp_buf buf;

static WORD menuid = 0;
static STRPTR names[MAX_MENUS];
static WORD images[MAX_MENUS];
static BOOL disabled[MAX_MENUS];
static WORD itemids[MAX_MENUS] = { 0 };
static STRPTR itemnames[MAX_MENUS][MAX_ITEMS];
static WORD itemimages[MAX_MENUS][MAX_ITEMS];
static BOOL itemdisabled[MAX_MENUS][MAX_ITEMS];

void error(STRPTR err)
{
	puts(err);
	longjmp(buf, -1);
}

/* newMenu(name, image) - Utwórz nowe menu */

WORD newMenu(STRPTR name, WORD image, BOOL enable)
{
	extern WORD menuid;
	extern STRPTR names[];
	extern WORD images[];
	extern BOOL disabled[];

	if (menuid >= MAX_MENUS)
		error("Too many menus!\n");

	names[menuid] = name;
	images[menuid] = image;

	if (!enable)
		disabled[menuid] = TRUE;

	return(menuid++);
}

/* enableMenu(id) - Wîâcz/wyîâcz menu */

void enableMenu(WORD id, BOOL enable)
{
	extern WORD menuid;
	extern BOOL disabled[];

	if (id >= menuid)
		error("Invalid menu ID!\n");

	disabled[id] = !enable;
}

/* newMenuItem(menuid, itemname, itemimage, enable) - Dodaj element menu */

WORD newMenuItem(WORD id, STRPTR itemname, WORD itemimage, BOOL enable)
{
	extern WORD menuid;
	extern WORD itemids[MAX_MENUS];
	extern STRPTR itemnames[][MAX_ITEMS];
	extern WORD itemimages[][MAX_ITEMS];
	extern BOOL itemdisabled[][MAX_ITEMS];
	WORD itemid;

	if (id >= menuid)
		error("Invalid menu ID!\n");

	itemid = itemids[id];

	if (itemid >= MAX_ITEMS)
		error("Too many menu items!\n");

	itemnames[id][itemid] = itemname;
	itemimages[id][itemid] = itemimage;

	if (!enable)
		itemdisabled[id][itemid] = TRUE;

	return(itemids[id]++);
}

void enableMenuItem(WORD mid, WORD id, BOOL enable)
{
	itemdisabled[mid][id] = !enable;
}

void showMenu(void)
{
	WORD i;

	for (i = 0; i < menuid; i++)
	{
		WORD j;

		printf("Menu %s\n", names[i]);

		for (j = 0; j < itemids[i]; j++)
		{
			printf("[%s]\n", itemnames[i][j]);
		}
	}
}

int main(void)
{
	int result;

	if ((result = setjmp(buf)) == 0)
	{
		newMenu("Magazyn", MAGAZYN, FALSE);
		newMenu("Kafelek", KAFELEK, TRUE);

		newMenuItem(MAGAZYN, "Rozpocznij", ROZPOCZNIJ, TRUE);

		showMenu();

		longjmp(buf, 1);
	}
	printf("Result = %d\n", result);
	return(0);
}
