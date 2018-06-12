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
#include <stdio.h>
#include "def.h"
#include "Types.h"
#include "monster.h"
#include "Main.h"
#include "Sprite.h"
#include "digger.h"
#include "Drawing.h"
#include "Bags.h"
#include "sound.h"
#include "Scores.h"
#include "record.h"
#include "Level.h"

struct monster
{
	/// <summary> The coordinates. </summary>
	short x, y, h, v, xr, yr;

	/// <summary> The directions. </summary>
	short dir, hdir;

	short t, hnt, death, bag, dtime, stime, chase;

	/// <summary> TRUE if monster is active (on screen) ?</summary>
	bool flag;

	/// <summary> TRUE if monster is a Nobbin. </summary>
	bool nob;

	/// <summary> TRUE if monster is alive. </summary>
	bool alive;
} mondat[MONSTERS];

static short nextmonster = 0;
extern FILE *dbgLog;

/// <summary> The total monsters on level. </summary>
static short g_TotalMonstersOnLevel = 0;

/// <summary> The maximum monsters on screen. </summary>
static short g_MaxMonstersOnScreen = 0;

static short nextmontime = 0;

/// <summary> Time of the gap between monsters appearence. </summary>
static short g_MonsterGapTime = 0;

static short chase = 0;

static bool unbonusflag = FALSE;

/// <summary>
/// Initializes the monsters structure and globals.
/// </summary>
void InitializeMonsters()
{
	short i;

	for (i = 0; i < MONSTERS; i++)
	{
		mondat[i].flag = FALSE;
	}

	// ???
	nextmonster = 0;

	// setup gap between next monster appearence
	g_MonsterGapTime = MONSTER_GAP_TIME - (GetLevelDifficultyModifier(g_GameData[g_CurrentPlayer].level) << 1);

	// total monsters on level
	g_TotalMonstersOnLevel = GetLevelDifficultyModifier(g_GameData[g_CurrentPlayer].level) + 5;

	// maximum monsters on screen at time
	switch (GetLevelDifficultyModifier(g_GameData[g_CurrentPlayer].level))
	{
	case 1:
	{
		g_MaxMonstersOnScreen = 3;
		break;
	}
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	{
		g_MaxMonstersOnScreen = 4;
		break;
	}
	case 8:
	case 9:
	case 10:
	{
		g_MaxMonstersOnScreen = 5;
		break;
	}
	}

	// next time before monster appears
	nextmontime = 10;

	// not in bonus mode?
	unbonusflag = TRUE;
}

/// <summary>
/// Erase monster sprites.
/// </summary>
void EraseMonsterSprites()
{
	int i;

	for (i = 0; i < MONSTERS; i++)
	{
		if (mondat[i].flag)
		{
			EraseSprite(i + FIRSTMONSTER);
		}
	}
}

void domonsters()
{
	short i;

	if (nextmontime > 0)
	{
		nextmontime--;
	}
	else
	{
		if (nextmonster < g_TotalMonstersOnLevel && nmononscr() < g_MaxMonstersOnScreen && isalive() && !bonusmode)
		{
			createmonster();
		}

		if (unbonusflag && nextmonster == g_TotalMonstersOnLevel && nextmontime == 0)
		{
			if (isalive())
			{
				unbonusflag = FALSE;
				createbonus();
			}
		}
	}

	for (i = 0; i < MONSTERS; i++)
	{
		if (mondat[i].flag)
		{
            //fprintf(dbgLog, "Monster%d (%d, %d)\n", i, mondat[i].x, mondat[i].y); //!!DEBUG
            if (mondat[i].hnt > 10 - GetLevelDifficultyModifier(g_GameData[g_CurrentPlayer].level))
			{
				if (mondat[i].nob)
				{
					mondat[i].nob = FALSE;
					mondat[i].hnt = 0;
				}
			}

			if (mondat[i].alive)
			{
				if (mondat[i].t == 0)
				{
					monai(i);

					/* Need to split for determinism */
					if (randno(15 - GetLevelDifficultyModifier(g_GameData[g_CurrentPlayer].level)) == 0)
					{
						if (mondat[i].nob && mondat[i].alive)
						{
							monai(i);
						}
					}
				}
				else
				{
					mondat[i].t--;
				}
			}
			else
			{
				mondie(i);
			}
		}
	}
}

void createmonster()
{
	short i;
	for (i = 0; i < MONSTERS; i++)
		if (!mondat[i].flag) {
			mondat[i].flag = TRUE;
			mondat[i].alive = TRUE;
			mondat[i].t = 0;
			mondat[i].nob = TRUE;
			mondat[i].hnt = 0;
			mondat[i].h = 14;
			mondat[i].v = 0;
			mondat[i].x = 292;
			mondat[i].y = 18;
			mondat[i].xr = 0;
			mondat[i].yr = 0;
			mondat[i].dir = DIR_LEFT;
			mondat[i].hdir = DIR_LEFT;
			mondat[i].chase = chase + g_CurrentPlayer;
			chase = (chase + 1) % g_DiggersCount;
			nextmonster++;
			nextmontime = g_MonsterGapTime;
			mondat[i].stime = 5;
			MoveAndRedrawSprite(i + FIRSTMONSTER, mondat[i].x, mondat[i].y);
			break;
		}
}

bool g_MonsterGotGold = FALSE;

void monai(short mon)
{
	short monox, monoy, dir, mdirp1, mdirp2, mdirp3, mdirp4, t;
	int clcoll[SPRITES], clfirst[SPRITE_TYPES], i, m, dig;
	bool push, bagf;
	monox = mondat[mon].x;
	monoy = mondat[mon].y;
	if (mondat[mon].xr == 0 && mondat[mon].yr == 0) {

		/* If we are here the monster needs to know which way to turn next. */

		/* Turn hobbin back into nobbin if it's had its time */

		if (mondat[mon].hnt > 30 + (GetLevelDifficultyModifier(g_GameData[g_CurrentPlayer].level) << 1))
			if (!mondat[mon].nob) {
				mondat[mon].hnt = 0;
				mondat[mon].nob = TRUE;
			}

		/* Set up monster direction properties to chase Digger */

		dig = mondat[mon].chase;
		if (!digalive(dig))
			dig = (g_DiggersCount - 1) - dig;

		if (abs(diggery(dig) - mondat[mon].y) > abs(diggerx(dig) - mondat[mon].x)) {
			if (diggery(dig) < mondat[mon].y) { mdirp1 = DIR_UP;    mdirp4 = DIR_DOWN; }
			else { mdirp1 = DIR_DOWN;  mdirp4 = DIR_UP; }
			if (diggerx(dig) < mondat[mon].x) { mdirp2 = DIR_LEFT;  mdirp3 = DIR_RIGHT; }
			else { mdirp2 = DIR_RIGHT; mdirp3 = DIR_LEFT; }
		}
		else {
			if (diggerx(dig) < mondat[mon].x) { mdirp1 = DIR_LEFT;  mdirp4 = DIR_RIGHT; }
			else { mdirp1 = DIR_RIGHT; mdirp4 = DIR_LEFT; }
			if (diggery(dig) < mondat[mon].y) { mdirp2 = DIR_UP;    mdirp3 = DIR_DOWN; }
			else { mdirp2 = DIR_DOWN;  mdirp3 = DIR_UP; }
		}

		/* In bonus mode, run away from Digger */

		if (bonusmode) {
			t = mdirp1; mdirp1 = mdirp4; mdirp4 = t;
			t = mdirp2; mdirp2 = mdirp3; mdirp3 = t;
		}

		/* Adjust priorities so that monsters don't reverse direction unless they
		   really have to */

		dir = reversedir(mondat[mon].dir);
		if (dir == mdirp1) {
			mdirp1 = mdirp2;
			mdirp2 = mdirp3;
			mdirp3 = mdirp4;
			mdirp4 = dir;
		}
		if (dir == mdirp2) {
			mdirp2 = mdirp3;
			mdirp3 = mdirp4;
			mdirp4 = dir;
		}
		if (dir == mdirp3) {
			mdirp3 = mdirp4;
			mdirp4 = dir;
		}

		/* Introduce a random element on levels <6 : occasionally swap p1 and p3 */

		if (randno(GetLevelDifficultyModifier(g_GameData[g_CurrentPlayer].level) + 5) == 1) /* Need to split for determinism */
			if (GetLevelDifficultyModifier(g_GameData[g_CurrentPlayer].level) < 6) {
				t = mdirp1;
				mdirp1 = mdirp3;
				mdirp3 = t;
			}

		/* Check g_Field and find direction */

		if (fieldclear(mdirp1, mondat[mon].h, mondat[mon].v))
			dir = mdirp1;
		else
			if (fieldclear(mdirp2, mondat[mon].h, mondat[mon].v))
				dir = mdirp2;
			else
				if (fieldclear(mdirp3, mondat[mon].h, mondat[mon].v))
					dir = mdirp3;
				else
					if (fieldclear(mdirp4, mondat[mon].h, mondat[mon].v))
						dir = mdirp4;

		/* Hobbins don't care about the g_Field: they go where they want. */

		if (!mondat[mon].nob)
			dir = mdirp1;

		/* Monsters take a time g_Penalty for changing direction */

		if (mondat[mon].dir != dir)
			mondat[mon].t++;

		/* Save the new direction */

		mondat[mon].dir = dir;
	}

	/* If monster is about to go off edge of screen, stop it. */

	if ((mondat[mon].x == 292 && mondat[mon].dir == DIR_RIGHT) ||
		(mondat[mon].x == 12 && mondat[mon].dir == DIR_LEFT) ||
		(mondat[mon].y == 180 && mondat[mon].dir == DIR_DOWN) ||
		(mondat[mon].y == 18 && mondat[mon].dir == DIR_UP))
		mondat[mon].dir = DIR_NONE;

	/* Change hdir for hobbin */

	if (mondat[mon].dir == DIR_LEFT || mondat[mon].dir == DIR_RIGHT)
		mondat[mon].hdir = mondat[mon].dir;

	/* Hobbins dig */

	if (!mondat[mon].nob)
		eatfield(mondat[mon].x, mondat[mon].y, mondat[mon].dir);

	/* (Draw new tunnels) and move monster */

	switch (mondat[mon].dir) {
	case DIR_RIGHT:
		if (!mondat[mon].nob)
			drawrightblob(mondat[mon].x, mondat[mon].y);
		mondat[mon].x += 4;
		break;
	case DIR_UP:
		if (!mondat[mon].nob)
			drawtopblob(mondat[mon].x, mondat[mon].y);
		mondat[mon].y -= 3;
		break;
	case DIR_LEFT:
		if (!mondat[mon].nob)
			drawleftblob(mondat[mon].x, mondat[mon].y);
		mondat[mon].x -= 4;
		break;
	case DIR_DOWN:
		if (!mondat[mon].nob)
			drawbottomblob(mondat[mon].x, mondat[mon].y);
		mondat[mon].y += 3;
		break;
	}

	/* Hobbins can eat emeralds */

	if (!mondat[mon].nob)
		hitemerald((mondat[mon].x - 12) / 20, (mondat[mon].y - 18) / 18,
		(mondat[mon].x - 12) % 20, (mondat[mon].y - 18) % 18,
			mondat[mon].dir);

	/* If Digger's gone, don't bother */

	if (!isalive()) {
		mondat[mon].x = monox;
		mondat[mon].y = monoy;
	}

	/* If monster's just started, don't move yet */

	if (mondat[mon].stime != 0) {
		mondat[mon].stime--;
		mondat[mon].x = monox;
		mondat[mon].y = monoy;
	}

	/* Increase time counter for hobbin */

	if (!mondat[mon].nob && mondat[mon].hnt < 100)
		mondat[mon].hnt++;

	/* Draw monster */

	push = TRUE;
	drawmon(mon, mondat[mon].nob, mondat[mon].hdir, mondat[mon].x, mondat[mon].y);
	for (i = 0; i < SPRITE_TYPES; i++)
		clfirst[i] = first[i];
	for (i = 0; i < SPRITES; i++)
		clcoll[i] = g_Collisions[i];
	IncrementPenalty();

	/* Collision with another monster */

	if (clfirst[2] != -1) {
		mondat[mon].t++; /* Time g_Penalty */
		/* Ensure both aren't moving in the same dir. */
		i = clfirst[2];
		do {
			m = i - FIRSTMONSTER;
			if (mondat[mon].dir == mondat[m].dir && mondat[m].stime == 0 &&
				mondat[mon].stime == 0)
				mondat[m].dir = reversedir(mondat[m].dir);
			/* The kludge here is to preserve playback for a bug in previous
			   versions. */
			if (!kludge)
				IncrementPenalty();
			else
				if (!(m & 1))
					IncrementPenalty();
			i = clcoll[i];
		} while (i != -1);
		if (kludge)
			if (clfirst[0] != -1)
				IncrementPenalty();
	}

	/* Check for collision with bag */

	i = clfirst[1];
	bagf = FALSE;
	while (i != -1) {
		if (bagexist(i - FIRSTBAG)) {
			bagf = TRUE;
			break;
		}
		i = clcoll[i];
	}

	if (bagf) {
		mondat[mon].t++; /* Time g_Penalty */
		g_MonsterGotGold = FALSE;
		if (mondat[mon].dir == DIR_RIGHT || mondat[mon].dir == DIR_LEFT) {
			push = IsPushBags(mondat[mon].dir, clfirst, clcoll);      /* Horizontal push */
			mondat[mon].t++; /* Time g_Penalty */
		}
		else
			if (!IsPushBagsVertical(clfirst, clcoll)) /* Vertical push */
				push = FALSE;
		if (g_MonsterGotGold) /* No time g_Penalty if monster eats gold */
			mondat[mon].t = 0;
		if (!mondat[mon].nob && mondat[mon].hnt > 1)
			removebags(clfirst, clcoll); /* Hobbins eat bags */
	}

	/* Increase hobbin cross counter */

	if (mondat[mon].nob && clfirst[2] != -1 && isalive())
		mondat[mon].hnt++;

	/* See if bags push monster back */

	if (!push) {
		mondat[mon].x = monox;
		mondat[mon].y = monoy;
		drawmon(mon, mondat[mon].nob, mondat[mon].hdir, mondat[mon].x, mondat[mon].y);
		IncrementPenalty();
		if (mondat[mon].nob) /* The other way to create hobbin: stuck on h-bag */
			mondat[mon].hnt++;
		if ((mondat[mon].dir == DIR_UP || mondat[mon].dir == DIR_DOWN) &&
			mondat[mon].nob)
			mondat[mon].dir = reversedir(mondat[mon].dir); /* If vertical, give up */
	}

	/* Collision with Digger */

	if (clfirst[4] != -1 && isalive())
		if (bonusmode) {
			killmon(mon);
			i = clfirst[4];
			while (i != -1) {
				if (digalive(i - FIRSTDIGGER + g_CurrentPlayer))
					sceatm(i - FIRSTDIGGER + g_CurrentPlayer);
				i = clcoll[i];
			}
			soundeatm(); /* Collision in bonus mode */
		}
		else {
			i = clfirst[4];
			while (i != -1) {
				if (digalive(i - FIRSTDIGGER + g_CurrentPlayer))
					killdigger(i - FIRSTDIGGER + g_CurrentPlayer, 3, 0); /* Kill Digger */
				i = clcoll[i];
			}
		}

		/* Update co-ordinates */

		mondat[mon].h = (mondat[mon].x - 12) / 20;
		mondat[mon].v = (mondat[mon].y - 18) / 18;
		mondat[mon].xr = (mondat[mon].x - 12) % 20;
		mondat[mon].yr = (mondat[mon].y - 18) % 18;
}

void mondie(short mon)
{
	switch (mondat[mon].death) {
	case 1:
		if (bagy(mondat[mon].bag) + 6 > mondat[mon].y)
			mondat[mon].y = bagy(mondat[mon].bag);
		drawmondie(mon, mondat[mon].nob, mondat[mon].hdir, mondat[mon].x,
			mondat[mon].y);
		IncrementPenalty();
		if (GetBagMovementDirection(mondat[mon].bag) == DIR_NONE) {
			mondat[mon].dtime = 1;
			mondat[mon].death = 4;
		}
		break;
	case 4:
		if (mondat[mon].dtime != 0)
			mondat[mon].dtime--;
		else {
			killmon(mon);
			if (g_DiggersCount == 2)
				scorekill2();
			else
				scorekill(g_CurrentPlayer);
		}
	}
}

bool fieldclear(short dir, short x, short y)
{
	switch (dir) {
	case DIR_RIGHT:
		if (x < 14)
			if ((GetField(x + 1, y) & 0x2000) == 0)
				if ((GetField(x + 1, y) & 1) == 0 || (GetField(x, y) & 0x10) == 0)
					return TRUE;
		break;
	case DIR_UP:
		if (y > 0)
			if ((GetField(x, y - 1) & 0x2000) == 0)
				if ((GetField(x, y - 1) & 0x800) == 0 || (GetField(x, y) & 0x40) == 0)
					return TRUE;
		break;
	case DIR_LEFT:
		if (x > 0)
			if ((GetField(x - 1, y) & 0x2000) == 0)
				if ((GetField(x - 1, y) & 0x10) == 0 || (GetField(x, y) & 1) == 0)
					return TRUE;
		break;
	case DIR_DOWN:
		if (y < 9)
			if ((GetField(x, y + 1) & 0x2000) == 0)
				if ((GetField(x, y + 1) & 0x40) == 0 || (GetField(x, y) & 0x800) == 0)
					return TRUE;
	}
	return FALSE;
}

void checkmonscared(short h)
{
	short m;
	for (m = 0; m < MONSTERS; m++)
		if (h == mondat[m].h && mondat[m].dir == DIR_UP)
			mondat[m].dir = DIR_DOWN;
}

void killmon(short mon)
{
	if (mondat[mon].flag) {
		mondat[mon].flag = mondat[mon].alive = FALSE;
		EraseSprite(mon + FIRSTMONSTER);
		if (bonusmode)
			g_TotalMonstersOnLevel++;
	}
}

void squashmonsters(short bag, int *clfirst, int *clcoll)
{
	int next = clfirst[2], m;
	while (next != -1) {
		m = next - FIRSTMONSTER;
		if (mondat[m].y >= bagy(bag))
			squashmonster(m, 1, bag);
		next = clcoll[next];
	}
}

short killmonsters(int *clfirst, int *clcoll)
{
	int next = clfirst[2], m, n = 0;
	while (next != -1) {
		m = next - FIRSTMONSTER;
		killmon(m);
		n++;
		next = clcoll[next];
	}
	return n;
}

void squashmonster(short mon, short death, short bag)
{
	mondat[mon].alive = FALSE;
	mondat[mon].death = death;
	mondat[mon].bag = bag;
}

// returns number of monsters on level
short MonstersLeft()
{
	return nmononscr() + g_TotalMonstersOnLevel - nextmonster;
}

short nmononscr()
{
	short i, n = 0;
	for (i = 0; i < MONSTERS; i++)
		if (mondat[i].flag)
			n++;
	return n;
}

void incmont(short n)
{
	short m;
	if (n > MONSTERS)
		n = MONSTERS;
	for (m = 1; m < n; m++)
		mondat[m].t++;
}

/// <summary>
/// Gets a field value.
/// </summary>
/// <param name="x">The x coordinate.</param>
/// <param name="y">The y coordinate.</param>
/// <returns>
/// The field value at given coordinates.
/// </returns>
short GetField(short x, short y)
{
	return g_Field[y * MWIDTH + x];
}
