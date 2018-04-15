/*
-----------------------------------------------------------------------
<copyright file="Bags.c" company="Petr Abdulin">
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

#include <string.h>
#include "def.h"
#include "Types.h"
#include "Bags.h"
#include "Main.h"
#include "Sprite.h"
#include "sound.h"
#include "Drawing.h"
#include "monster.h"
#include "digger.h"
#include "Scores.h"
#include "Map.h"
#include "Level.h"

struct BagStruct
{
	/// <summary> 
	/// Coordinates.
	/// </summary>
	short x, y, h, v;

	short xr, yr, WobbleTime, fallh;

	/// <summary> 
	/// The bag movement direction.
	/// </summary>
	short dir;

	/// <summary> 
	/// if equials 0, then gold disappers (or bag is not cracked yet?),
	/// overwise stores time left for gold before it disappears.
	/// </summary>
	short GoldTime;

	/// <summary>
	/// Is bag in wobbling (shaking before fall) state.
	/// </summary>
	bool IsWobbling;

	bool unfallen;

	// does bag exist
	bool exist;
} bagdat1[BAGS], bagdat2[BAGS], g_BagsData[BAGS];

/// <summary> 
/// The wobble animation frames.
/// </summary>
short g_WobbleAnimation[4] = { 2, 0, 1, 0 };

/// <summary> 
/// Number of pushed bags.
/// </summary>
short g_PushedBagsCount = 0;

/// <summary> 
/// The gold disappearence timeout value.
/// </summary>
short g_GoldTimeout = 0;

/// <summary>
/// Initialises the gold bags structures.
/// </summary>
void InitBags()
{
	// indexes
	short bag, x, y;

	g_PushedBagsCount = 0;
	g_GoldTimeout = BAGS_GOLD_TIMEOUT -
		GetLevelDifficultyModifier(g_GameData[g_CurrentPlayer].level) * BAGS_GOLD_TIMEOUT_DIFFICULTY_MODIFIER;

	// reset data
	for (bag = 0; bag < BAGS; bag++)
	{
		g_BagsData[bag].exist = FALSE;
	}

	// data pointer
	bag = 0;

	// scan map for bags
	for (x = 0; x < MWIDTH; x++)
	{
		for (y = 0; y < MHEIGHT; y++)
		{
			// found bag
			if (IsBag(GetLevelSymbol(x, y, GetLevelMap(g_GameData[g_CurrentPlayer].level))))
			{
				// check if bags storage available
				if (bag < BAGS)
				{
					// set coordinates
					g_BagsData[bag].x = x * BLOCK_WIDTH + BLOCK_WIDTH_OFFSET;
					g_BagsData[bag].y = y * BLOCK_HEIGHT + BLOCK_HEIGHT_OFFSET;
					g_BagsData[bag].h = x;
					g_BagsData[bag].v = y;

					// ???
					g_BagsData[bag].xr = 0;
					g_BagsData[bag].yr = 0;

					// set bag props..
					g_BagsData[bag].dir = DIR_NONE;
					g_BagsData[bag].WobbleTime = BAGS_WOBBLE_TIMEOUT;
					g_BagsData[bag].GoldTime = 0;
					g_BagsData[bag].fallh = 0;
					g_BagsData[bag].IsWobbling = FALSE;
					g_BagsData[bag].unfallen = TRUE;
					g_BagsData[bag].exist = TRUE;

					// next bag
					bag++;
				}
			}
		}
	}

	UpdateBagsForPlayer(g_CurrentPlayer);
}

/// <summary>
/// Draws gold bags.
/// </summary>
void DrawBags()
{
	short bag;

	// copy back to data
	UpdateBagsFromPlayer(g_CurrentPlayer);

	for (bag = 0; bag < BAGS; bag++)
	{
		if (g_BagsData[bag].exist)
		{
			MoveAndRedrawSprite(bag + FIRSTBAG, g_BagsData[bag].x, g_BagsData[bag].y);
		}
	}
}

/// <summary>
/// Updates the bags data for player.
/// </summary>
/// <param name="player">The player to update bags data for.</param>
void UpdateBagsForPlayer(int player)
{
	// copy bags data to corresponding player data copy
	if (player == 0)
	{
		memcpy(bagdat1, g_BagsData, BAGS * sizeof(struct BagStruct));
	}
	else
	{
		memcpy(bagdat2, g_BagsData, BAGS * sizeof(struct BagStruct));
	}
}

/// <summary>
/// Updates the bags data from player.
/// </summary>
/// <param name="player">The player to update bags data from.</param>
void UpdateBagsFromPlayer(int player)
{
	// copy corresponding player bags data
	if (player == 0)
	{
		memcpy(g_BagsData, bagdat1, BAGS * sizeof(struct BagStruct));
	}
	else
	{
		memcpy(g_BagsData, bagdat2, BAGS * sizeof(struct BagStruct));
	}
}

void cleanupbags()
{
	short bag;
	soundfalloff();
	for (bag = 0; bag < BAGS; bag++) {
		if (g_BagsData[bag].exist && ((g_BagsData[bag].h == 7 && g_BagsData[bag].v == 9) ||
			g_BagsData[bag].xr != 0 || g_BagsData[bag].yr != 0 || g_BagsData[bag].GoldTime != 0 ||
			g_BagsData[bag].fallh != 0 || g_BagsData[bag].IsWobbling)) {
			g_BagsData[bag].exist = FALSE;
			EraseSprite(bag + FIRSTBAG);
		}
		if (g_CurrentPlayer == 0)
			memcpy(&bagdat1[bag], &g_BagsData[bag], sizeof(struct BagStruct));
		else
			memcpy(&bagdat2[bag], &g_BagsData[bag], sizeof(struct BagStruct));
	}
}

/// <summary>
/// Main gold bags processing.
/// </summary>
void dobags()
{
	short bag;

	bool soundfalloffflag = TRUE;
	bool soundwobbleoffflag = TRUE;

	for (bag = 0; bag < BAGS; bag++)
	{
		if (g_BagsData[bag].exist)
		{
			if (g_BagsData[bag].GoldTime != 0)
			{
				// bag just bursted
				if (g_BagsData[bag].GoldTime == 1)
				{
					// bag hit sound
					soundbreak();

					drawgold(bag, 4, g_BagsData[bag].x, g_BagsData[bag].y);

					IncrementPenalty();
				}

				// bag break next sprite
				if (g_BagsData[bag].GoldTime == 3)
				{
					drawgold(bag, 5, g_BagsData[bag].x, g_BagsData[bag].y);
					IncrementPenalty();
				}

				// bag break next sprite
				if (g_BagsData[bag].GoldTime == 5)
				{
					drawgold(bag, 6, g_BagsData[bag].x, g_BagsData[bag].y);
					IncrementPenalty();
				}

				// advance to next sprite
				g_BagsData[bag].GoldTime++;

				// check gold timeout end
				if (g_BagsData[bag].GoldTime == g_GoldTimeout)
				{
					// gold disappears
					RemoveBag(bag);
				}
				else
				{
					// ???
					if (g_BagsData[bag].v < (MHEIGHT - 1) && g_BagsData[bag].GoldTime < (g_GoldTimeout - 10))
					{
						// ???
						if ((GetField(g_BagsData[bag].h, g_BagsData[bag].v + 1) & 0x2000) == 0)
						{
							// ???
							g_BagsData[bag].GoldTime = g_GoldTimeout - 10;
						}
					}
				}
			}
			else
			{
				// update status
				updatebag(bag);
			}
		}
	}

	// update sound flags
	for (bag = 0; bag < BAGS; bag++)
	{
		if (g_BagsData[bag].exist)
		{
			// falling sound
			if (g_BagsData[bag].dir == DIR_DOWN)
			{
				soundfalloffflag = FALSE;
			}

			// wobbling sound
			if (g_BagsData[bag].dir != DIR_DOWN && g_BagsData[bag].IsWobbling)
			{
				soundwobbleoffflag = FALSE;
			}
		}
	}

	// process sound state
	if (soundfalloffflag)
	{
		soundfalloff();
	}

	// process sound state
	if (soundwobbleoffflag)
	{
		soundwobbleoff();
	}
}

/// <summary>
/// Updates bags status.
/// </summary>
/// <param name="bag">The bag to update.</param>
void updatebag(short bag)
{
	short x, h, xr, y, v, yr, wbl;

	x = g_BagsData[bag].x;
	h = g_BagsData[bag].h;
	xr = g_BagsData[bag].xr;

	y = g_BagsData[bag].y;
	v = g_BagsData[bag].v;
	yr = g_BagsData[bag].yr;

	switch (g_BagsData[bag].dir)
	{
	case DIR_NONE:
	{
		if (y < 180 && xr == 0)
		{
			// if bag is in wobbling state
			if (g_BagsData[bag].IsWobbling)
			{
				// wobble timeout ended, bag starts to fall
				if (g_BagsData[bag].WobbleTime == 0)
				{
					// update direction to down 
					g_BagsData[bag].dir = DIR_DOWN;

					// make fall sound
					soundfall();

					// exit
					break;
				}

				// decrease wobble time
				g_BagsData[bag].WobbleTime--;

				// wbl is not larger then 7
				wbl = g_BagsData[bag].WobbleTime % 8;

				// only for even
				if (!(wbl & 1))
				{
					drawgold(bag, g_WobbleAnimation[wbl >> 1], x, y);

					IncrementPenalty();

					// make wobble sound
					soundwobble();
				}
			}
			else
			{
				if ((GetField(h, v + 1) & 0xfdf) != 0xfdf)
				{
					if (!checkdiggerunderbag(h, v + 1))
					{
						g_BagsData[bag].IsWobbling = TRUE;
					}
				}
			}
		}
		else
		{
			g_BagsData[bag].WobbleTime = BAGS_WOBBLE_TIMEOUT;
			g_BagsData[bag].IsWobbling = FALSE;
		}

		break;
	}

	case DIR_RIGHT:
	case DIR_LEFT:
	{
		if (xr == 0)
		{
			// if pushed to hole
			if (y < 180 && (GetField(h, v + 1) & 0xfdf) != 0xfdf)
			{
				// instant fall (no wobble)
				g_BagsData[bag].dir = DIR_DOWN;
				g_BagsData[bag].WobbleTime = 0;

				// make falling sound
				soundfall();
			}
			else
			{
				baghitground(bag);
			}
		}

		break;
	}

	case DIR_DOWN:
	{
		if (yr == 0)
		{
			g_BagsData[bag].fallh++;
		}

		if (y >= 180)
		{
			baghitground(bag);
		}
		else
		{
			if ((GetField(h, v + 1) & 0xfdf) == 0xfdf)
			{
				if (yr == 0)
				{
					baghitground(bag);
				}
			}
		}

		// check is monster scared by falling bag ?
		checkmonscared(g_BagsData[bag].h);

		break;
	}
	}

	if (g_BagsData[bag].dir != DIR_NONE)
	{
		if (g_BagsData[bag].dir != DIR_DOWN && g_PushedBagsCount != 0)
		{
			g_PushedBagsCount--;
		}
		else
		{
			pushbag(bag, g_BagsData[bag].dir);
		}
	}
}

void baghitground(short bag)
{
	int clfirst[SPRITE_TYPES], clcoll[SPRITES], i;
	if (g_BagsData[bag].dir == DIR_DOWN && g_BagsData[bag].fallh > 1)
		g_BagsData[bag].GoldTime = 1;
	else
		g_BagsData[bag].fallh = 0;
	g_BagsData[bag].dir = DIR_NONE;
	g_BagsData[bag].WobbleTime = 15;
	g_BagsData[bag].IsWobbling = FALSE;
	drawgold(bag, 0, g_BagsData[bag].x, g_BagsData[bag].y);
	for (i = 0; i < SPRITE_TYPES; i++)
		clfirst[i] = first[i];
	for (i = 0; i < SPRITES; i++)
		clcoll[i] = g_Collisions[i];
	IncrementPenalty();
	i = clfirst[1];
	while (i != -1) {
		RemoveBag(i - FIRSTBAG);
		i = clcoll[i];
	}
}

bool pushbag(short bag, short dir)
{
	short x, y;

	// original x, y?
	short ox, oy;

	// horizontal, vertical (what?)
	short h, v;

	int clfirst[SPRITE_TYPES], clcoll[SPRITES], i;

	bool push = TRUE, digf;

	// save current position
	ox = x = g_BagsData[bag].x;
	oy = y = g_BagsData[bag].y;
	h = g_BagsData[bag].h;
	v = g_BagsData[bag].v;

	// if bag is in "gold" state 
	if (g_BagsData[bag].GoldTime != 0)
	{
		// then digger/monster collects gold
		CollectGold(bag);

		return TRUE;
	}

	if (g_BagsData[bag].dir == DIR_DOWN && (dir == DIR_RIGHT || dir == DIR_LEFT)) {
		drawgold(bag, 3, x, y);
		for (i = 0; i < SPRITE_TYPES; i++)
			clfirst[i] = first[i];
		for (i = 0; i < SPRITES; i++)
			clcoll[i] = g_Collisions[i];
		IncrementPenalty();
		i = clfirst[4];
		while (i != -1) {
			if (diggery(i - FIRSTDIGGER + g_CurrentPlayer) >= y)
				killdigger(i - FIRSTDIGGER + g_CurrentPlayer, 1, bag);
			i = clcoll[i];
		}
		if (clfirst[2] != -1)
			squashmonsters(bag, clfirst, clcoll);
		return 1;
	}
	if ((x == 292 && dir == DIR_RIGHT) || (x == 12 && dir == DIR_LEFT) ||
		(y == 180 && dir == DIR_DOWN) || (y == 18 && dir == DIR_UP))
		push = FALSE;
	if (push) {
		switch (dir) {
		case DIR_RIGHT:
			x += 4;
			break;
		case DIR_LEFT:
			x -= 4;
			break;
		case DIR_DOWN:
			if (g_BagsData[bag].unfallen) {
				g_BagsData[bag].unfallen = FALSE;
				drawsquareblob(x, y);
				drawtopblob(x, y + 21);
			}
			else
				drawfurryblob(x, y);
			eatfield(x, y, dir);
			killemerald(h, v);
			y += 6;
		}
		switch (dir) {
		case DIR_DOWN:
			drawgold(bag, 3, x, y);
			for (i = 0; i < SPRITE_TYPES; i++)
				clfirst[i] = first[i];
			for (i = 0; i < SPRITES; i++)
				clcoll[i] = g_Collisions[i];
			IncrementPenalty();
			i = clfirst[4];
			while (i != -1) {
				if (diggery(i - FIRSTDIGGER + g_CurrentPlayer) >= y)
					killdigger(i - FIRSTDIGGER + g_CurrentPlayer, 1, bag);
				i = clcoll[i];
			}
			if (clfirst[2] != -1)
				squashmonsters(bag, clfirst, clcoll);
			break;
		case DIR_RIGHT:
		case DIR_LEFT:
			g_BagsData[bag].WobbleTime = 15;
			g_BagsData[bag].IsWobbling = FALSE;
			drawgold(bag, 0, x, y);
			for (i = 0; i < SPRITE_TYPES; i++)
				clfirst[i] = first[i];
			for (i = 0; i < SPRITES; i++)
				clcoll[i] = g_Collisions[i];
			IncrementPenalty();
			g_PushedBagsCount = 1;
			if (clfirst[1] != -1)
				if (!IsPushBags(dir, clfirst, clcoll)) {
					x = ox;
					y = oy;
					drawgold(bag, 0, ox, oy);
					IncrementPenalty();
					push = FALSE;
				}
			i = clfirst[4];
			digf = FALSE;
			while (i != -1) {
				if (digalive(i - FIRSTDIGGER + g_CurrentPlayer))
					digf = TRUE;
				i = clcoll[i];
			}
			if (digf || clfirst[2] != -1) {
				x = ox;
				y = oy;
				drawgold(bag, 0, ox, oy);
				IncrementPenalty();
				push = FALSE;
			}
		}
		if (push)
			g_BagsData[bag].dir = dir;
		else
			g_BagsData[bag].dir = reversedir(dir);
		g_BagsData[bag].x = x;
		g_BagsData[bag].y = y;
		g_BagsData[bag].h = (x - 12) / 20;
		g_BagsData[bag].v = (y - 18) / 18;
		g_BagsData[bag].xr = (x - 12) % 20;
		g_BagsData[bag].yr = (y - 18) % 18;
	}
	return push;
}

bool IsPushBags(short dir, int *clfirst, int *clcoll)
{
	bool push = TRUE;

	int next = clfirst[SPRITE_TYPE_BAG];

	while (next != -1)
	{
		if (!pushbag(next - FIRSTBAG, dir))
		{
			push = FALSE;
		}

		next = clcoll[next];
	}

	return push;
}

bool IsPushBagsVertical(int *clfirst, int *clcoll)
{
	bool push = TRUE;

	int next = clfirst[SPRITE_TYPE_BAG];

	while (next != -1)
	{
		if (g_BagsData[next - FIRSTBAG].GoldTime != 0)
		{
			CollectGold(next - FIRSTBAG);
		}
		else
		{
			push = FALSE;
		}

		next = clcoll[next];
	}

	return push;
}

/// <summary>
/// Removes the bag, both logically and from screen, by it's index.
/// </summary>
/// <param name="bag">The bag index.</param>
void RemoveBag(short bagIndex)
{
	if (g_BagsData[bagIndex].exist)
	{
		g_BagsData[bagIndex].exist = FALSE;

		EraseSprite(bagIndex + FIRSTBAG);
	}
}

bool bagexist(int bag)
{
	return g_BagsData[bag].exist;
}

short bagy(short bag)
{
	return g_BagsData[bag].y;
}

short GetBagMovementDirection(short bag)
{
	if (g_BagsData[bag].exist)
	{
		return g_BagsData[bag].dir;
	}

	return DIR_NONE;
}

void removebags(int *clfirst, int *clcoll)
{
	int next = clfirst[SPRITE_TYPE_BAG];

	while (next != -1)
	{
		RemoveBag(next - FIRSTBAG);

		next = clcoll[next];
	}
}

/// <summary>
/// Gets the moving bags count.
/// </summary>
/// <returns>
/// The moving bags count.
/// </returns>
/// <remarks>
/// Used only on level end, game will wait until all bags will fall.
/// </remarks>
short GetMovingBagsCount()
{
	short bag, n = 0;

	for (bag = 0; bag < BAGS; bag++)
	{
		if (g_BagsData[bag].exist
			&& g_BagsData[bag].GoldTime < 10
			&& (g_BagsData[bag].GoldTime != 0 || g_BagsData[bag].IsWobbling))
		{
			n++;
		}
	}

	return n;
}

/// <summary>
/// Collect gold for given bag.
/// </summary>
/// <param name="bag">The bag.</param>
void CollectGold(short bag)
{
	bool monsterEatsGold = TRUE;

	int i;

	drawgold(bag, 6, g_BagsData[bag].x, g_BagsData[bag].y);

	IncrementPenalty();

	// presumably, get some sprite index..
	i = first[SPRITE_TYPE_DIGGER];

	while (i != -1)
	{
		if (digalive(i - FIRSTDIGGER + g_CurrentPlayer))
		{
			scoregold(i - FIRSTDIGGER + g_CurrentPlayer);

			soundgold();

			// digger is not pushing bag now
			g_DiggerData[i - FIRSTDIGGER + g_CurrentPlayer].IsPushingBag = 0;

			monsterEatsGold = FALSE;
		}

		i = g_Collisions[i];
	}

	if (monsterEatsGold)
	{
		// monster eats gold
		g_MonsterGotGold = TRUE;
	}

	// remove bag from playfield
	RemoveBag(bag);
}
