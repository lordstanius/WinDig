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

#include <stdlib.h>
#include "def.h"
#include "Types.h"
#include "Sprite.h"
#include "hardware.h"

// Wait for vertical retrace on palette change flag
bool g_WaitForVerticalRetraceFlag = TRUE;

// Redraw sprite?
bool g_SpriteNeedsRedrawFlags[SPRITES + 1];

// Sprite recursion flag
bool g_SpriteRecursionFlags[SPRITES + 1];

// Sprite enabled?
bool g_SpriteEnabledFlags[SPRITES];

// Sprite buffer pointer (for geti and puti)
// contain pointers to (monbufs, bagbusf, etc) 
byte *g_SpriteBufferPointer[SPRITES];

// Sprite position
short g_SpriteX[SPRITES + 1];
short g_SpriteY[SPRITES + 1];

// Sprite character number
short g_SpriteCharacter[SPRITES + 1];

// Sprite size
short g_SpriteWidth[SPRITES + 1];
short g_SpriteHeight[SPRITES + 1];

// Sprite border size (only 0 is ever used in Digger)
short g_SpriteBorderWidth[SPRITES];
short g_SpriteBorderHeight[SPRITES];

// New sprite character
short g_NewSpriteCharacter[SPRITES];

// New sprite size
short g_NewSpriteWidth[SPRITES];
short g_NewSpriteHeight[SPRITES];

// New sprite border size
short g_NewSpriteBorderWidth[SPRITES];
short g_NewSpriteBorderHeight[SPRITES];

void ClearRedrawFlags();
void ClearRecursionFlags();
void SetSpriteRedrawFlags(short n);
bool CollisionCheck(short bx, short si);
bool BorderCollisionCheck(short bx, short si);
void DrawActualSprites();
void RedrawBackgroundImages();
void CreateLinkedCollisionsList(int bx);

void(*ginit)() = vgainit;
void(*gpal)(short pal) = vgapal;
void(*ginten)(short inten) = vgainten;
void(*gputim)(short x, short y, short ch, short w, short h) = vgaputim;
short(*ggetpix)(short x, short y) = vgagetpix;
void(*gtitle)() = vgatitle;

/// <summary>
/// Functon pointer which if actually used to draw the text.
/// </summary>
/// <param name="x">The x coordinate.</param>
/// <param name="y">The y coordinate.</param>
/// <param name="symbol">The symbol to draw.</param>
/// <param name="color">The color to use.</param>
void(*g_DrawSymbolFunc)(short x, short y, short symbol, short color) = 0;

void SetWaitForVerticalRetraceFlag(bool f)
{
	g_WaitForVerticalRetraceFlag = f;
}

void CreateSprite(short n, short ch, byte *mov, short width, short height, short bwid, short bhei)
{
	g_NewSpriteCharacter[n] = g_SpriteCharacter[n] = ch;
	g_SpriteBufferPointer[n] = mov;
	g_NewSpriteWidth[n] = g_SpriteWidth[n] = width;
	g_NewSpriteHeight[n] = g_SpriteHeight[n] = height;
	g_NewSpriteBorderWidth[n] = g_SpriteBorderWidth[n] = bwid;
	g_NewSpriteBorderHeight[n] = g_SpriteBorderHeight[n] = bhei;
	g_SpriteEnabledFlags[n] = FALSE;
}

/// <summary>
/// Move and redraw given sprite.
/// </summary>
/// <param name="spriteId">The sprite number (id).</param>
/// <param name="x">The x coordinate.</param>
/// <param name="y">The y coordinate.</param>
void MoveAndRedrawSprite(short spriteId, short x, short y)
{
	g_SpriteX[spriteId] = x & SPRITE_X_MASK;
	g_SpriteY[spriteId] = y;
	g_SpriteCharacter[spriteId] = g_NewSpriteCharacter[spriteId];

	g_SpriteWidth[spriteId] = g_NewSpriteWidth[spriteId];
	g_SpriteHeight[spriteId] = g_NewSpriteHeight[spriteId];

	g_SpriteBorderWidth[spriteId] = g_NewSpriteBorderWidth[spriteId];
	g_SpriteBorderHeight[spriteId] = g_NewSpriteBorderHeight[spriteId];

	ClearRedrawFlags();
	SetSpriteRedrawFlags(spriteId);
	RedrawBackgroundImages();

	// save 
	vgageti(g_SpriteX[spriteId], g_SpriteY[spriteId], g_SpriteBufferPointer[spriteId], g_SpriteWidth[spriteId], g_SpriteHeight[spriteId]);

	g_SpriteEnabledFlags[spriteId] = TRUE;
	g_SpriteNeedsRedrawFlags[spriteId] = TRUE;

	DrawActualSprites();
}

/// <summary>
/// Erases sprite from screen.
/// </summary>
/// <param name="n">The sprite index.</param>
void EraseSprite(short n)
{
	// if sprite not exist
	if (!g_SpriteEnabledFlags[n])
	{
		// then exit
		return;
	}

	vgaputi(g_SpriteX[n], g_SpriteY[n], g_SpriteBufferPointer[n], g_SpriteWidth[n], g_SpriteHeight[n]);

	g_SpriteEnabledFlags[n] = FALSE;

	ClearRedrawFlags();

	SetSpriteRedrawFlags(n);

	DrawActualSprites();
}

/// <summary>
/// Draw sprite n. 
/// Redraw any sprites which need redrawing. 
/// Set collision table.
/// </summary>
/// <param name="n">The sprite index.</param>
void drawspr(short n, short x, short y)
{
	short t1, t2, t3, t4;
	x &= SPRITE_X_MASK;

	ClearRedrawFlags();

	SetSpriteRedrawFlags(n);

	t1 = g_SpriteX[n];
	t2 = g_SpriteY[n];
	t3 = g_SpriteWidth[n];
	t4 = g_SpriteHeight[n];

	g_SpriteX[n] = x;
	g_SpriteY[n] = y;
	g_SpriteWidth[n] = g_NewSpriteWidth[n];
	g_SpriteHeight[n] = g_NewSpriteHeight[n];

	ClearRecursionFlags();

	SetSpriteRedrawFlags(n);

	g_SpriteHeight[n] = t4;
	g_SpriteWidth[n] = t3;
	g_SpriteY[n] = t2;
	g_SpriteX[n] = t1;

	g_SpriteNeedsRedrawFlags[n] = TRUE;

	RedrawBackgroundImages();

	g_SpriteEnabledFlags[n] = TRUE;

	g_SpriteX[n] = x;
	g_SpriteY[n] = y;
	g_SpriteCharacter[n] = g_NewSpriteCharacter[n];
	g_SpriteWidth[n] = g_NewSpriteWidth[n];
	g_SpriteHeight[n] = g_NewSpriteHeight[n];
	g_SpriteBorderWidth[n] = g_NewSpriteBorderWidth[n];
	g_SpriteBorderHeight[n] = g_NewSpriteBorderHeight[n];

	vgageti(g_SpriteX[n], g_SpriteY[n], g_SpriteBufferPointer[n], g_SpriteWidth[n], g_SpriteHeight[n]);

	DrawActualSprites();

	CreateLinkedCollisionsList(n);
}

/// <summary>
/// Set up (initialize) sprite.
/// </summary>
/// <param name="spriteId">Sprite id.</param>
/// <param name="ch">Sprite character.</param>
/// <param name="width">Sprite width.</param>
/// <param name="height">Sprite height.</param>
/// <param name="bwid">Sprite border width.</param>
/// <param name="bhei">Sprite border height.</param>
void InitializeSprite(short spriteId, short ch, short width, short height, short bwid, short bhei)
{
	g_NewSpriteCharacter[spriteId] = ch;
	g_NewSpriteWidth[spriteId] = width;
	g_NewSpriteHeight[spriteId] = height;
	g_NewSpriteBorderWidth[spriteId] = bwid;
	g_NewSpriteBorderHeight[spriteId] = bhei;
}

// Set up miscellaneous sprite (static objects).
void initmiscspr(short x, short y, short wid, short hei)
{
	g_SpriteX[SPRITES] = x;
	g_SpriteY[SPRITES] = y;
	g_SpriteWidth[SPRITES] = wid;
	g_SpriteHeight[SPRITES] = hei;
	ClearRedrawFlags();
	SetSpriteRedrawFlags(SPRITES);
	RedrawBackgroundImages();
}

// Get all the background images for all the sprites being redrawn.
void getis()
{
	short i;
	for (i = 0; i < SPRITES; i++)
		if (g_SpriteNeedsRedrawFlags[i])
			vgageti(g_SpriteX[i], g_SpriteY[i], g_SpriteBufferPointer[i], g_SpriteWidth[i], g_SpriteHeight[i]);
	DrawActualSprites();
}

// Draw static object
void drawmiscspr(short x, short y, short ch, short wid, short hei)
{
	g_SpriteX[SPRITES] = x & SPRITE_X_MASK;
	g_SpriteY[SPRITES] = y;
	g_SpriteCharacter[SPRITES] = ch;
	g_SpriteWidth[SPRITES] = wid;
	g_SpriteHeight[SPRITES] = hei;
	gputim(g_SpriteX[SPRITES], g_SpriteY[SPRITES], g_SpriteCharacter[SPRITES], g_SpriteWidth[SPRITES],
		g_SpriteHeight[SPRITES]);
}

/// <summary>
/// Clears redraw flags of sprites.
/// </summary>
void ClearRedrawFlags()
{
	short i;

	ClearRecursionFlags();

	for (i = 0; i < SPRITES + 1; i++)
	{
		g_SpriteNeedsRedrawFlags[i] = FALSE;
	}
}

/// <summary>
/// Clears recursion flags of sprites.
/// </summary>
void ClearRecursionFlags()
{
	short i;

	for (i = 0; i < SPRITES + 1; i++)
	{
		g_SpriteRecursionFlags[i] = FALSE;
	}
}

/// <summary>
/// Set drawing flags for sprite.
/// </summary>
/// <param name="n">The sprite id.</param>
void SetSpriteRedrawFlags(short n)
{
	short i;
	if (!g_SpriteRecursionFlags[n])
	{
		g_SpriteRecursionFlags[n] = TRUE;

		for (i = 0; i < SPRITES; i++)
		{
			if (g_SpriteEnabledFlags[i] && i != n)
			{
				if (CollisionCheck(i, n))
				{
					g_SpriteNeedsRedrawFlags[i] = TRUE;
					SetSpriteRedrawFlags(i);
				}
			}
		}
	}
}

/// <summary>
/// Check for collision between 2 sprites.
/// </summary>
/// <param name="bx">The sprite id.</param>
/// <param name="si">The sprite id.</param>
/// <remarks>Used for redraw detection.</remarks>
/// <returns>TRUE if sprites collide, FALSE otherwise.</returns>
bool CollisionCheck(short bx, short si)
{
	if (g_SpriteX[bx] >= g_SpriteX[si]) {
		if (g_SpriteX[bx] > (g_SpriteWidth[si] << 2) + g_SpriteX[si] - 1)
			return FALSE;
	}
	else
		if (g_SpriteX[si] > (g_SpriteWidth[bx] << 2) + g_SpriteX[bx] - 1)
			return FALSE;
	if (g_SpriteY[bx] >= g_SpriteY[si]) {
		if (g_SpriteY[bx] <= g_SpriteHeight[si] + g_SpriteY[si] - 1)
			return TRUE;
		return FALSE;
	}
	if (g_SpriteY[si] <= g_SpriteHeight[bx] + g_SpriteY[bx] - 1)
		return TRUE;
	return FALSE;
}

/// <summary>
/// Check for collision between 2 sprites, taking into account their borders.
/// </summary>
/// <param name="bx">The sprite id.</param>
/// <param name="si">The sprite id.</param>
/// <remarks> Used for collision detection.</remarks>
/// <returns>TRUE if sprites (including with their borders) collide, FALSE otherwise.</returns>
bool BorderCollisionCheck(short bx, short si)
{
	if (g_SpriteX[bx] >= g_SpriteX[si]) {
		if (g_SpriteX[bx] + g_SpriteBorderWidth[bx] > (g_SpriteWidth[si] << 2) + g_SpriteX[si] - g_SpriteBorderWidth[si] - 1)
			return FALSE;
	}
	else
		if (g_SpriteX[si] + g_SpriteBorderWidth[si] > (g_SpriteWidth[bx] << 2) + g_SpriteX[bx] - g_SpriteBorderWidth[bx] - 1)
			return FALSE;
	if (g_SpriteY[bx] >= g_SpriteY[si]) {
		if (g_SpriteY[bx] + g_SpriteBorderHeight[bx] <= g_SpriteHeight[si] + g_SpriteY[si] - g_SpriteBorderHeight[si] - 1)
			return TRUE;
		return FALSE;
	}
	if (g_SpriteY[si] + g_SpriteBorderHeight[si] <= g_SpriteHeight[bx] + g_SpriteY[bx] - g_SpriteBorderHeight[bx] - 1)
		return TRUE;
	return FALSE;
}

/// <summary>
/// Draw actual sprites.
/// </summary>
/// <remarks>'actual' stands for active, which are marked for redraw.</remarks>
void DrawActualSprites()
{
	int i;

	for (i = 0; i < SPRITES; i++)
	{
		if (g_SpriteNeedsRedrawFlags[i])
		{
			gputim(g_SpriteX[i], g_SpriteY[i], g_SpriteCharacter[i], g_SpriteWidth[i], g_SpriteHeight[i]);
		}
	}
}

/// <summary>
/// Redraw background images.
/// </summary>
void RedrawBackgroundImages()
{
	int i;

	for (i = 0; i < SPRITES; i++)
	{
		if (g_SpriteNeedsRedrawFlags[i])
		{
			vgaputi(g_SpriteX[i], g_SpriteY[i], g_SpriteBufferPointer[i], g_SpriteWidth[i], g_SpriteHeight[i]);
		}
	}
}

int first[SPRITE_TYPES];

int g_Collisions[SPRITES];

/// <summary>
/// Setup collision linked lists.
/// </summary>
void CreateLinkedCollisionsList(int spr)
{
	int firstSpriteIndex[SPRITE_TYPES] = { FIRSTBONUS, FIRSTBAG, FIRSTMONSTER, FIRSTFIREBALL, FIRSTDIGGER };
	int lastSpriteIndex[SPRITE_TYPES] = { LASTBONUS, LASTBAG, LASTMONSTER, LASTFIREBALL, LASTDIGGER };
	int spc, next, i;

	for (next = 0; next < SPRITE_TYPES; next++)
	{
		first[next] = -1;
	}

	for (next = 0; next < SPRITES; next++)
	{
		g_Collisions[next] = -1;
	}

	for (i = 0; i < SPRITE_TYPES; i++)
	{
		next = -1;

		for (spc = firstSpriteIndex[i]; spc < lastSpriteIndex[i]; spc++)
		{
			if (g_SpriteEnabledFlags[spc] && spc != spr)
			{
				if (BorderCollisionCheck(spr, spc))
				{
					if (next == -1)
					{
						first[i] = next = spc;
					}
					else
					{
						g_Collisions[next = (g_Collisions[next] = spc)] = -1;
					}
				}
			}
		}
	}
}
