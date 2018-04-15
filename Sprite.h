/*
-----------------------------------------------------------------------
<copyright file="" company="Petr Abdulin">
Digger Remastered portions copyright 1983-2012 Windmill Software Inc.
Digger Remastered is copyright 1998-2012 Andrew Jenner.
Digger Remastered is copyright 2011-2012 Petr Abdulin.
Digger Remastered is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation. Either version 2 of the License, or
(at your option) any later version. See the GNU General Public License
for more details: http://www.gnu.org/licenses/gpl.html
</copyright>
-----------------------------------------------------------------------
*/

#include "monster.h"

// -4 is 1111 1111 1111 1100, thus masking int with it effectively
// rounds value to to 4 pixel precision
#define SPRITE_X_MASK -4 

void SetWaitForVerticalRetraceFlag(bool f);
void MoveAndRedrawSprite(short n, short x, short y);
void EraseSprite(short n);
void CreateSprite(short n, short ch, byte *mov, short wid, short hei, short bwid,
	short bhei);
void InitializeSprite(short spriteIndex, short ch, short width, short height, short bwid, short bhei);
void drawspr(short n, short x, short y);
void initmiscspr(short x, short y, short wid, short hei);
void getis();
void drawmiscspr(short x, short y, short ch, short wid, short hei);

extern void(*ginit)();
extern void(*gpal)(short pal);
extern void(*ginten)(short inten);
extern void(*gputim)(short x, short y, short ch, short w, short h);
extern short(*ggetpix)(short x, short y);
extern void(*gtitle)();

/// <summary>
/// Functon pointer which if actually used to draw the text.
/// </summary>
/// <param name="x">The x coordinate.</param>
/// <param name="y">The y coordinate.</param>
/// <param name="symbol">The symbol to draw.</param>
/// <param name="color">The color to use.</param>
extern void(*g_DrawSymbolFunc)(short x, short y, short symbol, short color);


extern int first[SPRITE_TYPES], g_Collisions[SPRITES];
