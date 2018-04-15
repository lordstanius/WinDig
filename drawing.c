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

#include <stdio.h>
#include <string.h>
#include "def.h"
#include "Types.h"
#include "Drawing.h"
#include "Main.h"
#include "hardware.h"
#include "Sprite.h"
#include "digger.h"
#include "sound.h"
#include "Level.h"

short field1[MSIZE], field2[MSIZE];

// Data indicating which paths have been dug
// on bit indicates dirt, 0 indicates tunnel
// no dug bits are -1 (1111 1111 1111 1111)
//                 ++++ ++
// V dug bits 1101 0000 0011 1111 (6 dug positions on vertical 18/3=6, top to bottom)
//                         + ++++
// H dug bits 1101 1111 1110 0000 (5 dug positions on horizontal 20/4=5, left to right)
// S dug bits 1101 0000 0010 0000
short g_Field[MSIZE];

byte monbufs[MONSTERS][960], bagbufs[BAGS][960], bonusbufs[BONUSES][960], diggerbufs[DIGGERS][960], firebufs[FIREBALLS][256];

// Bit masks for field calculations
ushort bitmasks[12] = { 0xfffe,0xfffd,0xfffb,0xfff7,0xffef,0xffdf,0xffbf,0xff7f, 0xfeff,0xfdff,0xfbff,0xf7ff };

// Sprite number for monsters
short monspr[MONSTERS];

// Monster animation direction (1,2,3 or 3,2,1)
short monspd[MONSTERS];

short digspr[DIGGERS], digspd[DIGGERS], firespr[FIREBALLS];

void drawlife(short t, short x, short y);
void CreateDiggerBonusAndFireballSprites();
void InitializeDiggerBonusAndFireballSprites();
void drawbackg(short l);
void drawfield();

/// <summary>
/// Draw game text.
/// </summary>
/// <param name="text">Text to draw.</param>
/// <param name="x">The x coordinate.</param>
/// <param name="y">The y coordinate.</param>
/// <param name="color">The color of the text.</param>
void DrawGameText(char *text, short x, short y, short color)
{
	short i;

	// draw symbol by symbol
	for (i = 0; text[i]; i++)
	{
		g_DrawSymbolFunc(x, y, text[i], color);

		// shift to next position
		x += SYMBOL_WIDTH;
	}
}

// Calculates the "field" array from the level plan
void makefield()
{
	short c, x, y;
	for (x = 0; x < MWIDTH; x++)
	{
		for (y = 0; y < MHEIGHT; y++)
		{
			// set all ones, no dugs
			g_Field[y*MWIDTH + x] = -1;

			// get cell type
			c = GetLevelSymbol(x, y, GetLevelMap(g_GameData[g_CurrentPlayer].level));

			if (c == 'S' || c == 'V')
			{
				// V dug bits, 1101 0000 0011 1111
				g_Field[y*MWIDTH + x] &= 0xd03f;
			}

			if (c == 'S' || c == 'H')
			{
				// H dug bits, 1101 1111 1110 0000
				// S will be   1101 0000 0010 0000
				g_Field[y*MWIDTH + x] &= 0xdfe0;
			}

			if (g_CurrentPlayer == 0)
			{
				field1[y*MWIDTH + x] = g_Field[y*MWIDTH + x];
			}
			else
			{
				field2[y*MWIDTH + x] = g_Field[y*MWIDTH + x];
			}
		}
	}
}

//  Draws the background and tunnels
void drawstatics()
{
	short x, y;
	for (x = 0; x < MWIDTH; x++)
		for (y = 0; y < MHEIGHT; y++)
			if (g_CurrentPlayer == 0)
				g_Field[y*MWIDTH + x] = field1[y*MWIDTH + x];
			else
				g_Field[y*MWIDTH + x] = field2[y*MWIDTH + x];
	SetWaitForVerticalRetraceFlag(TRUE);
	gpal(0);
	ginten(0);

	// draw background
	drawbackg(GetLevelMap(g_GameData[g_CurrentPlayer].level));
	drawfield();
}

void savefield()
{
	short x, y;
	for (x = 0; x < MWIDTH; x++)
		for (y = 0; y < MHEIGHT; y++)
			if (g_CurrentPlayer == 0)
				field1[y*MWIDTH + x] = g_Field[y*MWIDTH + x];
			else
				field2[y*MWIDTH + x] = g_Field[y*MWIDTH + x];
}

// Draws the tunnels
// V dug bits, 1101 0000 0011 1111
// H dug bits, 1101 1111 1110 0000
// S will be   1101 0000 0010 0000
void drawfield()
{
	short x, y, xp, yp;

	// scan all field cells
	for (x = 0; x < MWIDTH; x++)
	{
		for (y = 0; y < MHEIGHT; y++)
		{
			// if it's a hole
			// 0x2000 is 0010 0000 0000 0000
			if ((g_Field[y*MWIDTH + x] & 0x2000) == 0)
			{
				// get start pos shift
				xp = x * 20 + 12;
				yp = y * 18 + 18;

				// V dug
				if ((g_Field[y*MWIDTH + x] & 0xfc0) != 0xfc0)
				{
					g_Field[y*MWIDTH + x] &= 0xd03f;
					drawbottomblob(xp, yp - 15);
					drawbottomblob(xp, yp - 12);
					drawbottomblob(xp, yp - 9);
					drawbottomblob(xp, yp - 6);
					drawbottomblob(xp, yp - 3);
					drawtopblob(xp, yp + 3);
				}

				// H dug
				if ((g_Field[y*MWIDTH + x] & 0x1f) != 0x1f)
				{
					g_Field[y*MWIDTH + x] &= 0xdfe0;
					drawrightblob(xp - 16, yp);
					drawrightblob(xp - 12, yp);
					drawrightblob(xp - 8, yp);
					drawrightblob(xp - 4, yp);
					drawleftblob(xp + 4, yp);
				}

				// if not right edge cell
				if (x < 14)
				{
					// H or V dug
					if ((g_Field[y*MWIDTH + x + 1] & 0xfdf) != 0xfdf)
					{
						drawrightblob(xp, yp);
					}
				}

				// if not bottom edge cell
				if (y < 9)
				{
					// H or V dug
					if ((g_Field[(y + 1)*MWIDTH + x] & 0xfdf) != 0xfdf)
					{
						drawbottomblob(xp, yp);
					}
				}
			}
		}
	}
}

// Digs part of a tunnel
void eatfield(short x, short y, short dir)
{
	// cell and relative position
	short h = (x - 12) / 20;
	short xr = ((x - 12) % 20) / 4;
	short v = (y - 18) / 18;
	short yr = ((y - 18) % 18) / 3;

	// 
	IncrementPenalty();

	// store field data using masks
	switch (dir)
	{
	case DIR_RIGHT:
		h++;
		g_Field[v*MWIDTH + h] &= bitmasks[xr];
		if (g_Field[v*MWIDTH + h] & 0x1f)
			break;
		g_Field[v*MWIDTH + h] &= 0xdfff;
		break;

	case DIR_UP:
		yr--;
		if (yr < 0) {
			yr += 6;
			v--;
		}
		g_Field[v*MWIDTH + h] &= bitmasks[6 + yr];
		if (g_Field[v*MWIDTH + h] & 0xfc0)
			break;
		g_Field[v*MWIDTH + h] &= 0xdfff;
		break;

	case DIR_LEFT:
		xr--;
		if (xr < 0) {
			xr += 5;
			h--;
		}
		g_Field[v*MWIDTH + h] &= bitmasks[xr];
		if (g_Field[v*MWIDTH + h] & 0x1f)
			break;
		g_Field[v*MWIDTH + h] &= 0xdfff;
		break;

	case DIR_DOWN:
		v++;
		g_Field[v*MWIDTH + h] &= bitmasks[6 + yr];
		if (g_Field[v*MWIDTH + h] & 0xfc0)
			break;
		g_Field[v*MWIDTH + h] &= 0xdfff;
		break;
	}
}

void CreateMonsterAndBagSprites()
{
	short i;
	for (i = 0; i < BAGS; i++)
		CreateSprite(FIRSTBAG + i, 62, bagbufs[i], 4, 15, 0, 0);

	for (i = 0; i < MONSTERS; i++)
		CreateSprite(FIRSTMONSTER + i, 71, monbufs[i], 4, 15, 0, 0);

	CreateDiggerBonusAndFireballSprites();

	for (i = 0; i < MONSTERS; i++)
	{
		monspr[i] = 0;
		monspd[i] = 1;
	}
}

// ? initialize monster and bags sprites
void InitializeMonsterAndBagSprites()
{
	int i;
	for (i = 0; i < BAGS; i++)
	{
		InitializeSprite(FIRSTBAG + i, 62, 4, 15, 0, 0);
	}

	for (i = 0; i < MONSTERS; i++)
	{
		InitializeSprite(FIRSTMONSTER + i, 71, 4, 15, 0, 0);
	}

	InitializeDiggerBonusAndFireballSprites();
}

/// <summary>
/// Draws monster "n"
/// </summary>
/// <param name="n">Monster id.</param>
/// <param name="nobf">Is nobbin?</param>
/// <param name="dir">Monster direction.</param>
/// <param name="x">The x coordinate.</param>
/// <param name="y">The y coordinate.</param>
/// <remarks> Used for collision detection.</remarks>
void drawmon(short n, bool nobf, short dir, short x, short y)
{
	// set next sprite frame
	monspr[n] += monspd[n];

	// if come to limit
	if (monspr[n] == 2 || monspr[n] == 0)
	{
		// revert order
		monspd[n] = -monspd[n];
	}

	// proof check
	if (monspr[n] > 2)
		monspr[n] = 2;
	if (monspr[n] < 0)
		monspr[n] = 0;

	// if nobbin
	if (nobf)
	{
		InitializeSprite(FIRSTMONSTER + n, monspr[n] + 69, 4, 15, 0, 0);
	}
	else
	{
		// hobbins have direction
		switch (dir)
		{
		case DIR_RIGHT:
		{
			InitializeSprite(FIRSTMONSTER + n, monspr[n] + 73, 4, 15, 0, 0);
			break;
		}
		case DIR_LEFT:
		{
			InitializeSprite(FIRSTMONSTER + n, monspr[n] + 77, 4, 15, 0, 0);
			break;
		}
		}
	}

	drawspr(FIRSTMONSTER + n, x, y);
}

void drawmondie(short n, bool nobf, short dir, short x, short y)
{
	if (nobf)
		InitializeSprite(FIRSTMONSTER + n, 72, 4, 15, 0, 0);
	else
		switch (dir) {
		case DIR_RIGHT:
			InitializeSprite(FIRSTMONSTER + n, 76, 4, 15, 0, 0);
			break;
		case DIR_LEFT:
			InitializeSprite(FIRSTMONSTER + n, 80, 4, 14, 0, 0);
		}
	drawspr(FIRSTMONSTER + n, x, y);
}

void drawgold(short n, short t, short x, short y)
{
	InitializeSprite(FIRSTBAG + n, t + 62, 4, 15, 0, 0);
	drawspr(FIRSTBAG + n, x, y);
}

void drawlife(short t, short x, short y)
{
	drawmiscspr(x, y, t + 110, 4, 12);
}

void drawemerald(short x, short y)
{
	initmiscspr(x, y, 4, 10);
	drawmiscspr(x, y, 108, 4, 10);
	getis();
}

void eraseemerald(short x, short y)
{
	initmiscspr(x, y, 4, 10);
	drawmiscspr(x, y, 109, 4, 10);
	getis();
}

void CreateDiggerBonusAndFireballSprites()
{
	int i;
	for (i = 0; i < DIGGERS; i++) {
		digspd[i] = 1;
		digspr[i] = 0;
	}
	for (i = 0; i < FIREBALLS; i++)
		firespr[i] = 0;
	for (i = FIRSTDIGGER; i < LASTDIGGER; i++)
		CreateSprite(i, 0, diggerbufs[i - FIRSTDIGGER], 4, 15, 0, 0);
	for (i = FIRSTBONUS; i < LASTBONUS; i++)
		CreateSprite(i, 81, bonusbufs[i - FIRSTBONUS], 4, 15, 0, 0);
	for (i = FIRSTFIREBALL; i < LASTFIREBALL; i++)
		CreateSprite(i, 82, firebufs[i - FIRSTFIREBALL], 2, 8, 0, 0);
}

void InitializeDiggerBonusAndFireballSprites()
{
	int i;

	for (i = 0; i < DIGGERS; i++)
	{
		digspd[i] = 1;
		digspr[i] = 0;
	}

	for (i = 0; i < FIREBALLS; i++)
	{
		firespr[i] = 0;
	}

	for (i = FIRSTDIGGER; i < LASTDIGGER; i++)
	{
		InitializeSprite(i, 0, 4, 15, 0, 0);
	}

	for (i = FIRSTBONUS; i < LASTBONUS; i++)
	{
		InitializeSprite(i, 81, 4, 15, 0, 0);
	}

	for (i = FIRSTFIREBALL; i < LASTFIREBALL; i++)
	{
		InitializeSprite(i, 82, 2, 8, 0, 0);
	}
}

void drawrightblob(short x, short y)
{
	initmiscspr(x + 16, y - 1, 2, 18);
	drawmiscspr(x + 16, y - 1, 102, 2, 18);
	getis();
}

void drawleftblob(short x, short y)
{
	initmiscspr(x - 8, y - 1, 2, 18);
	drawmiscspr(x - 8, y - 1, 104, 2, 18);
	getis();
}

void drawtopblob(short x, short y)
{
	initmiscspr(x - 4, y - 6, 6, 6);
	drawmiscspr(x - 4, y - 6, 103, 6, 6);
	getis();
}

void drawbottomblob(short x, short y)
{
	initmiscspr(x - 4, y + 15, 6, 6);
	drawmiscspr(x - 4, y + 15, 105, 6, 6);
	getis();
}

void drawfurryblob(short x, short y)
{
	initmiscspr(x - 4, y + 15, 6, 8);
	drawmiscspr(x - 4, y + 15, 107, 6, 8);
	getis();
}

void drawsquareblob(short x, short y)
{
	initmiscspr(x - 4, y + 17, 6, 6);
	drawmiscspr(x - 4, y + 17, 106, 6, 6);
	getis();
}

// Draw the background for level "l".
void drawbackg(short l)
{
	short x, y;
	for (y = 14; y < 200; y += 4) {
		g_FillSoundBuffer();
		for (x = 0; x < 320; x += 20)
			drawmiscspr(x, y, 93 + l, 5, 4);
	}
}

void drawfire(int n, short x, short y, short t)
{
	int nn = (n == 0) ? 0 : 32;
	if (t == 0) {
		firespr[n]++;
		if (firespr[n] > 2)
			firespr[n] = 0;
		InitializeSprite(FIRSTFIREBALL + n, 82 + firespr[n] + nn, 2, 8, 0, 0);
	}
	else
		InitializeSprite(FIRSTFIREBALL + n, 84 + t + nn, 2, 8, 0, 0);
	drawspr(FIRSTFIREBALL + n, x, y);
}

void drawbonus(short x, short y)
{
	int n = 0;
	InitializeSprite(FIRSTBONUS + n, 81, 4, 15, 0, 0);
	MoveAndRedrawSprite(FIRSTBONUS + n, x, y);
}

void drawdigger(int n, short t, short x, short y, bool f)
{

	int nn = (n == 0) ? 0 : 31;

	digspr[n] += digspd[n];

	if (digspr[n] == 2 || digspr[n] == 0)
	{
		digspd[n] = -digspd[n];
	}

	if (digspr[n] > 2)
	{
		digspr[n] = 2;
	}

	if (digspr[n] < 0)
	{
		digspr[n] = 0;
	}

	if (t >= 0 && t <= 6 && !(t & 1))
	{
		InitializeSprite(FIRSTDIGGER + n, (t + (f ? 0 : 1)) * 3 + digspr[n] + 1 + nn, 4, 15, 0, 0);

		drawspr(FIRSTDIGGER + n, x, y);

		return;
	}

	if (t >= 10 && t <= 15)
	{
		InitializeSprite(FIRSTDIGGER + n, 40 + nn - t, 4, 15, 0, 0);

		drawspr(FIRSTDIGGER + n, x, y);

		return;
	}

	first[0] = first[1] = first[2] = first[3] = first[4] = -1;
}

void DrawLivesAndGauntletTime()
{
	short l, n, g;
	char buf[10];
	if (g_IsGauntletMode) {
		g = (short)(g_CurrentGauntletGameTime / GAME_TIME_SECOND_MULTIPLIER);
		sprintf(buf, "%3i:%02i", g / 60, g % 60);
		DrawGameText(buf, 124, 0, 3);
		return;
	}
	n = getlives(0) - 1;
	DrawGameText("     ", 96, 0, 2);
	if (n > 4) {
		drawlife(0, 80, 0);
		sprintf(buf, "X%i", n);
		DrawGameText(buf, 100, 0, 2);
	}
	else
		for (l = 1; l < 5; l++) {
			drawlife(n > 0 ? 0 : 2, l * 20 + 60, 0);
			n--;
		}
	if (g_PlayersCount == 2) {
		DrawGameText("     ", 164, 0, 2);
		n = getlives(1) - 1;
		if (n > 4) {
			sprintf(buf, "%iX", n);
			DrawGameText(buf, 220 - strlen(buf) * 12, 0, 2);
			drawlife(1, 224, 0);
		}
		else
			for (l = 1; l < 5; l++) {
				drawlife(n > 0 ? 1 : 2, 244 - l * 20, 0);
				n--;
			}
	}
	if (g_DiggersCount == 2) {
		DrawGameText("     ", 164, 0, 1);
		n = getlives(1) - 1;
		if (n > 4) {
			sprintf(buf, "%iX", n);
			DrawGameText(buf, 220 - strlen(buf) * 12, 0, 1);
			drawlife(3, 224, 0);
		}
		else
			for (l = 1; l < 5; l++) {
				drawlife(n > 0 ? 3 : 2, 244 - l * 20, 0);
				n--;
			}
	}
}
