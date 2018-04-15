/*
-----------------------------------------------------------------------
<copyright file="Digger.c" company="Petr Abdulin">
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

#include "def.h"
#include "Types.h"
#include "Sprite.h"
#include "input.h"
#include "hardware.h"
#include "digger.h"
#include "Drawing.h"
#include "Main.h"
#include "sound.h"
#include "monster.h"
#include "Scores.h"
#include "Bags.h"
#include "win_dig.h"
#include "Level.h"

Digger g_DiggerData[DIGGERS];

short startbonustimeleft = 0, bonustimeleft;

short emmask = 0;

sbyte emfield[MSIZE];

bool bonusvisible = FALSE, bonusmode = FALSE, digvisible;

/// <summary>
/// Initializes the digger structure.
/// </summary>
void InitializeDigger()
{
	int dig;

	for (dig = g_CurrentPlayer; dig < g_DiggersCount + g_CurrentPlayer; dig++)
	{
		// skip digger with no lives
		if (g_DiggerData[dig].LivesCount == 0)
		{
			continue;
		}

		g_DiggerData[dig].HorizontalPosition = (g_DiggersCount == 1) ? 7 : (8 - dig * 2);
		g_DiggerData[dig].VerticalPositon = 9;
		g_DiggerData[dig].mdir = 4;

		g_DiggerData[dig].x = g_DiggerData[dig].HorizontalPosition * 20 + 12;
		g_DiggerData[dig].y = g_DiggerData[dig].VerticalPositon * 18 + 18;

		g_DiggerData[dig].Direction = (dig == 0) ? DIR_RIGHT : DIR_LEFT;

		g_DiggerData[dig].rx = 0;
		g_DiggerData[dig].ry = 0;
		g_DiggerData[dig].IsPushingBag = 0;

		g_DiggerData[dig].IsAlive = 1;

		g_DiggerData[dig].dead = 0; /* alive !=> !dead but dead => !alive */
		g_DiggerData[dig].invin = 0;
		g_DiggerData[dig].ivt = 0;

		g_DiggerData[dig].deathstage = 1;

		MoveAndRedrawSprite(dig + FIRSTDIGGER - g_CurrentPlayer, g_DiggerData[dig].x, g_DiggerData[dig].y);
		g_DiggerData[dig].notfiring = TRUE;
		g_DiggerData[dig].emocttime = 0;
		g_DiggerData[dig].firepressed = FALSE;
		g_DiggerData[dig].expsn = 0;
		g_DiggerData[dig].rechargetime = 0;
		g_DiggerData[dig].emn = 0;
		g_DiggerData[dig].MonsterEatScoreMultiplier = 1;
	}
	digvisible = TRUE;
	bonusvisible = bonusmode = FALSE;
}

uint g_CurrentGameTime, g_GameFrameTime;

/// <summary>
/// Framerate control function.
/// </summary>
void NextGameFrame()
{
	uint time;

	while (1)
	{
		ProcessWindowsEvents();

		/* Idle time */
		g_FillSoundBuffer();

		// get hi-res time
		time = GetHighResolutionTime();

		CheckKeyboard();

		if (suspend_game)
		{
			continue;
		}

		// if more time passed than game frame time, then exit to next frame
		if (time >= g_CurrentGameTime + g_GameFrameTime)
		{
			break;
		}

		// happens on GetTickCount owerflow (about once in an hour)
		if (time < g_CurrentGameTime)
		{
			break;
		}
	}

	// last game time
	g_CurrentGameTime = time;
}

uint g_CurrentGauntletGameTime;

void drawdig(int n, int d, int x, int y, bool f)
{
	drawdigger(n - g_CurrentPlayer, d, x, y, f);

	if (g_DiggerData[n].invin)
	{
		g_DiggerData[n].ivt--;

		if (g_DiggerData[n].ivt == 0)
		{
			g_DiggerData[n].invin = FALSE;
		}
		else
		{
			if (g_DiggerData[n].ivt % 10 < 5)
			{
				EraseSprite(FIRSTDIGGER + n - g_CurrentPlayer);
			}
		}
	}
}

void DoDiggerMove()
{
	int n;

	// process gauntlet mode part
	if (g_IsGauntletMode)
	{
		// update time
		DrawLivesAndGauntletTime();

		// check if no time is left
		if (g_CurrentGauntletGameTime < g_GameFrameTime)
		{
			g_IsTimeout = TRUE;
		}

		// reduce time left
		g_CurrentGauntletGameTime -= g_GameFrameTime;
	}

	// g_CurrentPlayer is either 0 or 1, g_DiggersCount is either 1 or 2, but
	// if g_DiggersCount is 2 (ID_GAME_PLAYERS_TWOSIMULTANEOUS), then g_CurrentPlayer is always 0 (since g_PlayersCount is set to 1)
	// So, this inredibly obfuscated code select either first, second, of both diggers to update. Duh!
	for (n = g_CurrentPlayer; n < g_DiggersCount + g_CurrentPlayer; n++)
	{
		if (g_DiggerData[n].expsn != 0)
		{
			drawexplosion(n);
		}
		else
		{
			updatefire(n);
		}

		if (digvisible)
		{
			if (g_DiggerData[n].IsAlive)
			{
				if (g_DiggerData[n].IsPushingBag != 0)
				{
					drawdig(n, g_DiggerData[n].mdir, g_DiggerData[n].x, g_DiggerData[n].y, g_DiggerData[n].notfiring && g_DiggerData[n].rechargetime == 0);

					IncrementPenalty();

					g_DiggerData[n].IsPushingBag--;
				}
				else
				{
					updatedigger(n);
				}
			}
			else
			{
				DiggerDeath(n);
			}
		}

		if (g_DiggerData[n].emocttime > 0)
		{
			g_DiggerData[n].emocttime--;
		}
	}

	if (bonusmode && isalive())
	{
		if (bonustimeleft != 0)
		{
			bonustimeleft--;

			if (startbonustimeleft != 0 || bonustimeleft < 20)
			{
				startbonustimeleft--;

				if (bonustimeleft & 1)
				{
					ginten(0);

					soundbonus();
				}
				else
				{
					ginten(1);

					soundbonus();
				}

				if (startbonustimeleft == 0)
				{
					music(BONUS_TUNE);

					soundbonusoff();

					ginten(1);
				}
			}
		}
		else
		{
			endbonusmode();

			soundbonusoff();

			music(NORMAL_TUNE);
		}
	}

	if (bonusmode && !isalive())
	{
		endbonusmode();

		soundbonusoff();

		music(NORMAL_TUNE);
	}
}

void updatefire(int n)
{
	short pix;
	int clfirst[SPRITE_TYPES], clcoll[SPRITES], i;
	bool clflag;
	if (g_DiggerData[n].notfiring) {
		if (g_DiggerData[n].rechargetime != 0)
			g_DiggerData[n].rechargetime--;
		else
			if (firepflag[n - g_CurrentPlayer])
				if (g_DiggerData[n].IsAlive) {
					g_DiggerData[n].rechargetime = GetLevelDifficultyModifier(g_GameData[g_CurrentPlayer].level) * 3 + 60;
					g_DiggerData[n].notfiring = FALSE;
					switch (g_DiggerData[n].Direction) {
					case DIR_RIGHT:
						g_DiggerData[n].fx = g_DiggerData[n].x + 8;
						g_DiggerData[n].fy = g_DiggerData[n].y + 4;
						break;
					case DIR_UP:
						g_DiggerData[n].fx = g_DiggerData[n].x + 4;
						g_DiggerData[n].fy = g_DiggerData[n].y;
						break;
					case DIR_LEFT:
						g_DiggerData[n].fx = g_DiggerData[n].x;
						g_DiggerData[n].fy = g_DiggerData[n].y + 4;
						break;
					case DIR_DOWN:
						g_DiggerData[n].fx = g_DiggerData[n].x + 4;
						g_DiggerData[n].fy = g_DiggerData[n].y + 8;
					}
					g_DiggerData[n].fdir = g_DiggerData[n].Direction;
					MoveAndRedrawSprite(FIRSTFIREBALL + n - g_CurrentPlayer, g_DiggerData[n].fx, g_DiggerData[n].fy);
					soundfire(n);
				}
	}
	else {
		switch (g_DiggerData[n].fdir) {
		case DIR_RIGHT:
			g_DiggerData[n].fx += 8;
			pix = ggetpix(g_DiggerData[n].fx, g_DiggerData[n].fy + 4) |
				ggetpix(g_DiggerData[n].fx + 4, g_DiggerData[n].fy + 4);
			break;
		case DIR_UP:
			g_DiggerData[n].fy -= 7;
			pix = 0;
			for (i = 0; i < 7; i++)
				pix |= ggetpix(g_DiggerData[n].fx + 4, g_DiggerData[n].fy + i);
			pix &= 0xc0;
			break;
		case DIR_LEFT:
			g_DiggerData[n].fx -= 8;
			pix = ggetpix(g_DiggerData[n].fx, g_DiggerData[n].fy + 4) |
				ggetpix(g_DiggerData[n].fx + 4, g_DiggerData[n].fy + 4);
			break;
		case DIR_DOWN:
			g_DiggerData[n].fy += 7;
			pix = 0;
			for (i = 0; i < 7; i++)
				pix |= ggetpix(g_DiggerData[n].fx, g_DiggerData[n].fy + i);
			pix &= 0x3;
			break;
		}
		drawfire(n - g_CurrentPlayer, g_DiggerData[n].fx, g_DiggerData[n].fy, 0);
		for (i = 0; i < SPRITE_TYPES; i++)
			clfirst[i] = first[i];
		for (i = 0; i < SPRITES; i++)
			clcoll[i] = g_Collisions[i];
		IncrementPenalty();
		i = clfirst[2];
		while (i != -1) {
			killmon(i - FIRSTMONSTER);
			scorekill(n);
			g_DiggerData[n].expsn = 1;
			i = clcoll[i];
		}
		i = clfirst[4];
		while (i != -1) {
			if (i - FIRSTDIGGER + g_CurrentPlayer != n && !g_DiggerData[i - FIRSTDIGGER + g_CurrentPlayer].invin
				&& g_DiggerData[i - FIRSTDIGGER + g_CurrentPlayer].IsAlive) {
				killdigger(i - FIRSTDIGGER + g_CurrentPlayer, 3, 0);
				g_DiggerData[n].expsn = 1;
			}
			i = clcoll[i];
		}
		if (clfirst[0] != -1 || clfirst[1] != -1 || clfirst[2] != -1 || clfirst[3] != -1 ||
			clfirst[4] != -1)
			clflag = TRUE;
		else
			clflag = FALSE;
		if (clfirst[0] != -1 || clfirst[1] != -1 || clfirst[3] != -1) {
			g_DiggerData[n].expsn = 1;
			i = clfirst[3];
			while (i != -1) {
				if (g_DiggerData[i - FIRSTFIREBALL + g_CurrentPlayer].expsn == 0)
					g_DiggerData[i - FIRSTFIREBALL + g_CurrentPlayer].expsn = 1;
				i = clcoll[i];
			}
		}
		switch (g_DiggerData[n].fdir) {
		case DIR_RIGHT:
			if (g_DiggerData[n].fx > 296)
				g_DiggerData[n].expsn = 1;
			else
				if (pix != 0 && !clflag) {
					g_DiggerData[n].expsn = 1;
					g_DiggerData[n].fx -= 8;
					drawfire(n - g_CurrentPlayer, g_DiggerData[n].fx, g_DiggerData[n].fy, 0);
				}
			break;
		case DIR_UP:
			if (g_DiggerData[n].fy < 15)
				g_DiggerData[n].expsn = 1;
			else
				if (pix != 0 && !clflag) {
					g_DiggerData[n].expsn = 1;
					g_DiggerData[n].fy += 7;
					drawfire(n - g_CurrentPlayer, g_DiggerData[n].fx, g_DiggerData[n].fy, 0);
				}
			break;
		case DIR_LEFT:
			if (g_DiggerData[n].fx < 16)
				g_DiggerData[n].expsn = 1;
			else
				if (pix != 0 && !clflag) {
					g_DiggerData[n].expsn = 1;
					g_DiggerData[n].fx += 8;
					drawfire(n - g_CurrentPlayer, g_DiggerData[n].fx, g_DiggerData[n].fy, 0);
				}
			break;
		case DIR_DOWN:
			if (g_DiggerData[n].fy > 183)
				g_DiggerData[n].expsn = 1;
			else
				if (pix != 0 && !clflag) {
					g_DiggerData[n].expsn = 1;
					g_DiggerData[n].fy -= 7;
					drawfire(n - g_CurrentPlayer, g_DiggerData[n].fx, g_DiggerData[n].fy, 0);
				}
		}
	}
}

void erasediggers()
{
	int i;
	for (i = 0; i < g_DiggersCount; i++)
		EraseSprite(FIRSTDIGGER + i);
	digvisible = FALSE;
}

void drawexplosion(int n)
{
	switch (g_DiggerData[n].expsn) {
	case 1:
		soundexplode(n);
	case 2:
	case 3:
		drawfire(n - g_CurrentPlayer, g_DiggerData[n].fx, g_DiggerData[n].fy, g_DiggerData[n].expsn);
		IncrementPenalty();
		g_DiggerData[n].expsn++;
		break;
	default:
		killfire(n);
		g_DiggerData[n].expsn = 0;
	}
}

void killfire(int n)
{
	if (!g_DiggerData[n].notfiring) {
		g_DiggerData[n].notfiring = TRUE;
		EraseSprite(FIRSTFIREBALL + n - g_CurrentPlayer);
		soundfireoff(n);
	}
}

// Move digger, dig tunnels, push bags, eat monsters etc.
void updatedigger(int n)
{
	short dir, ddir, diggerox, diggeroy, nmon;
	bool push = TRUE, bagf;

	int clfirst[SPRITE_TYPES], clcoll[SPRITES], i;

	// detect direction and moving direction
	ReadDiggerDirection(n - g_CurrentPlayer);
	dir = GetDiggerDirection(n - g_CurrentPlayer);
	if (dir == DIR_RIGHT || dir == DIR_UP || dir == DIR_LEFT || dir == DIR_DOWN)
		ddir = dir;
	else
		ddir = DIR_NONE;

	// we can turn up down only from cell aligned position
	if (g_DiggerData[n].rx == 0 && (ddir == DIR_UP || ddir == DIR_DOWN))
		g_DiggerData[n].Direction = g_DiggerData[n].mdir = ddir;

	// we can turn left-right only from cell aligned position
	if (g_DiggerData[n].ry == 0 && (ddir == DIR_RIGHT || ddir == DIR_LEFT))
		g_DiggerData[n].Direction = g_DiggerData[n].mdir = ddir;

	if (dir == DIR_NONE)
		g_DiggerData[n].mdir = DIR_NONE;
	else
		g_DiggerData[n].mdir = g_DiggerData[n].Direction;

	// at the borders of a screen
	if ((g_DiggerData[n].x == 292 && g_DiggerData[n].mdir == DIR_RIGHT) ||
		(g_DiggerData[n].x == 12 && g_DiggerData[n].mdir == DIR_LEFT) ||
		(g_DiggerData[n].y == 180 && g_DiggerData[n].mdir == DIR_DOWN) ||
		(g_DiggerData[n].y == 18 && g_DiggerData[n].mdir == DIR_UP))
	{
		g_DiggerData[n].mdir = DIR_NONE;
	}

	diggerox = g_DiggerData[n].x;
	diggeroy = g_DiggerData[n].y;

	// update field status
	if (g_DiggerData[n].mdir != DIR_NONE)
		eatfield(diggerox, diggeroy, g_DiggerData[n].mdir);

	// move and draw tunnel
	switch (g_DiggerData[n].mdir)
	{
	case DIR_RIGHT:
		drawrightblob(g_DiggerData[n].x, g_DiggerData[n].y);
		g_DiggerData[n].x += 4;
		break;
	case DIR_UP:
		drawtopblob(g_DiggerData[n].x, g_DiggerData[n].y);
		g_DiggerData[n].y -= 3;
		break;
	case DIR_LEFT:
		drawleftblob(g_DiggerData[n].x, g_DiggerData[n].y);
		g_DiggerData[n].x -= 4;
		break;
	case DIR_DOWN:
		drawbottomblob(g_DiggerData[n].x, g_DiggerData[n].y);
		g_DiggerData[n].y += 3;
		break;
	}

	if (hitemerald((g_DiggerData[n].x - 12) / 20, (g_DiggerData[n].y - 18) / 18,
		(g_DiggerData[n].x - 12) % 20, (g_DiggerData[n].y - 18) % 18,
		g_DiggerData[n].mdir)) {
		if (g_DiggerData[n].emocttime == 0)
			g_DiggerData[n].emn = 0;
		scoreemerald(n);
		soundem();
		soundemerald(g_DiggerData[n].emn);

		g_DiggerData[n].emn++;
		if (g_DiggerData[n].emn == 8) {
			g_DiggerData[n].emn = 0;
			scoreoctave(n);
		}
		g_DiggerData[n].emocttime = 9;
	}
	drawdig(n, g_DiggerData[n].Direction, g_DiggerData[n].x, g_DiggerData[n].y,
		g_DiggerData[n].notfiring && g_DiggerData[n].rechargetime == 0);

	// copy original to copy
	for (i = 0; i < SPRITE_TYPES; i++)
	{
		clfirst[i] = first[i];
	}

	for (i = 0; i < SPRITES; i++)
	{
		clcoll[i] = g_Collisions[i];
	}

	IncrementPenalty();

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
		if (g_DiggerData[n].mdir == DIR_RIGHT || g_DiggerData[n].mdir == DIR_LEFT) {
			push = IsPushBags(g_DiggerData[n].mdir, clfirst, clcoll);

			// TODO isn't it only 0 or 1?
			g_DiggerData[n].IsPushingBag++;
		}
		else
			if (!IsPushBagsVertical(clfirst, clcoll))
				push = FALSE;
		if (!push) { /* Strange, push not completely defined */
			g_DiggerData[n].x = diggerox;
			g_DiggerData[n].y = diggeroy;
			drawdig(n, g_DiggerData[n].mdir, g_DiggerData[n].x, g_DiggerData[n].y,
				g_DiggerData[n].notfiring && g_DiggerData[n].rechargetime == 0);
			IncrementPenalty();
			g_DiggerData[n].Direction = reversedir(g_DiggerData[n].mdir);
		}
	}
	if (clfirst[2] != -1 && bonusmode && g_DiggerData[n].IsAlive)
		for (nmon = killmonsters(clfirst, clcoll); nmon != 0; nmon--) {
			soundeatm();
			sceatm(n);
		}
	if (clfirst[0] != -1) {
		scorebonus(n);
		initbonusmode();
	}

	// recalculate cell and relative cell position
	g_DiggerData[n].HorizontalPosition = (g_DiggerData[n].x - 12) / 20;
	g_DiggerData[n].rx = (g_DiggerData[n].x - 12) % 20;

	g_DiggerData[n].VerticalPositon = (g_DiggerData[n].y - 18) / 18;
	g_DiggerData[n].ry = (g_DiggerData[n].y - 18) % 18;
}

void sceatm(int n)
{
	scoreeatm(n, g_DiggerData[n].MonsterEatScoreMultiplier);
	g_DiggerData[n].MonsterEatScoreMultiplier <<= 1;
}

short deatharc[GRAVE_ANIMATION_FRAMES] = { 3, 5, 6, 6, 5, 3, 0 };

void DiggerDeath(int n)
{
	int clfirst[SPRITE_TYPES], clcoll[SPRITES], i;

	bool isAllDiggersDead;

	switch (g_DiggerData[n].deathstage)
	{

	case DEATH_STAGE_BAG:
	{
		if (bagy(g_DiggerData[n].DeathBagIndex) + 6 > g_DiggerData[n].y)
		{
			g_DiggerData[n].y = bagy(g_DiggerData[n].DeathBagIndex) + 6;
		}

		drawdigger(n - g_CurrentPlayer, 15, g_DiggerData[n].x, g_DiggerData[n].y, FALSE);

		IncrementPenalty();

		if (GetBagMovementDirection(g_DiggerData[n].DeathBagIndex) + 1 == 0)
		{
			soundddie();
			g_DiggerData[n].deathtime = 5;
			g_DiggerData[n].deathstage = 2;
			g_DiggerData[n].deathani = 0;
			g_DiggerData[n].y -= 6;
		}

		break;
	}

	case DEATH_STAGE_GRAVE:
	{
		if (g_DiggerData[n].deathtime != 0)
		{
			g_DiggerData[n].deathtime--;
			break;
		}

		if (g_DiggerData[n].deathani == 0)
		{
			music(DEATH_TUNE);
		}

		drawdigger(n - g_CurrentPlayer, 14 - g_DiggerData[n].deathani, g_DiggerData[n].x, g_DiggerData[n].y, FALSE);

		for (i = 0; i < SPRITE_TYPES; i++)
		{
			clfirst[i] = first[i];
		}

		for (i = 0; i < SPRITES; i++)
		{
			clcoll[i] = g_Collisions[i];
		}

		IncrementPenalty();

		if (g_DiggerData[n].deathani == 0 && clfirst[2] != -1)
		{
			killmonsters(clfirst, clcoll);
		}

		if (g_DiggerData[n].deathani < 4)
		{
			g_DiggerData[n].deathani++;
			g_DiggerData[n].deathtime = 2;
		}
		else
		{
			g_DiggerData[n].deathstage = 4;
			if (musicflag || g_DiggersCount > 1)
			{
				g_DiggerData[n].deathtime = 100;   //Tim was 60
			}
			else
			{
				g_DiggerData[n].deathtime = 10;
			}
		}

		break;
	}

	case DEATH_STAGE_HIT:
	{
		g_DiggerData[n].deathstage = DEATH_STAGE_FLIP;
		g_DiggerData[n].deathani = 0;
		g_DiggerData[n].deathtime = 0;

		break;
	}

	case DEATH_STAGE_FLIP:
	{
		// draw death arc animation
		if (g_DiggerData[n].deathani >= 0 && g_DiggerData[n].deathani < GRAVE_ANIMATION_FRAMES)
		{
			drawdigger(n - g_CurrentPlayer, 15, g_DiggerData[n].x, g_DiggerData[n].y - deatharc[g_DiggerData[n].deathani], FALSE);

			if (g_DiggerData[n].deathani == 6 && !isalive())
			{
				musicoff();
			}

			IncrementPenalty();

			g_DiggerData[n].deathani++;

			if (g_DiggerData[n].deathani == 1)
			{
				soundddie();
			}

			if (g_DiggerData[n].deathani == 7)
			{
				g_DiggerData[n].deathtime = 5;
				g_DiggerData[n].deathani = 0;
				g_DiggerData[n].deathstage = 2;
			}
		}

		break;
	}

	case DEATH_STAGE_FINAL:
	{
		if (g_DiggerData[n].deathtime != 0)
		{
			g_DiggerData[n].deathtime--;
		}
		else
		{
			g_DiggerData[n].dead = TRUE;
			isAllDiggersDead = TRUE;

			for (i = 0; i < g_DiggersCount; i++)
			{
				if (!g_DiggerData[i].dead)
				{
					isAllDiggersDead = FALSE;
					break;
				}
			}


			if (isAllDiggersDead)
				setdead(TRUE);
			else
				if (isalive() && g_DiggerData[n].LivesCount > 0)
				{
					if (!g_IsGauntletMode)
						g_DiggerData[n].LivesCount--;

					DrawLivesAndGauntletTime();
					if (g_DiggerData[n].LivesCount > 0)
					{
						g_DiggerData[n].VerticalPositon = 9;
						g_DiggerData[n].mdir = 4;
						g_DiggerData[n].HorizontalPosition = (g_DiggersCount == 1) ? 7 : (8 - n * 2);
						g_DiggerData[n].x = g_DiggerData[n].HorizontalPosition * 20 + 12;
						g_DiggerData[n].Direction = (n == 0) ? DIR_RIGHT : DIR_LEFT;
						g_DiggerData[n].rx = 0;
						g_DiggerData[n].ry = 0;
						g_DiggerData[n].IsPushingBag = 0;
						g_DiggerData[n].IsAlive = TRUE;
						g_DiggerData[n].dead = FALSE;
						g_DiggerData[n].invin = TRUE;
						g_DiggerData[n].ivt = 50;
						g_DiggerData[n].deathstage = 1;
						g_DiggerData[n].y = g_DiggerData[n].VerticalPositon * 18 + 18;
						EraseSprite(n + FIRSTDIGGER - g_CurrentPlayer);
						MoveAndRedrawSprite(n + FIRSTDIGGER - g_CurrentPlayer, g_DiggerData[n].x, g_DiggerData[n].y);
						g_DiggerData[n].notfiring = TRUE;
						g_DiggerData[n].emocttime = 0;
						g_DiggerData[n].firepressed = FALSE;
						g_DiggerData[n].expsn = 0;
						g_DiggerData[n].rechargetime = 0;
						g_DiggerData[n].emn = 0;
						g_DiggerData[n].MonsterEatScoreMultiplier = 1;
					}

					clearfire(n);

					if (bonusmode)
					{
						music(BONUS_TUNE);
					}
					else
					{
						music(NORMAL_TUNE);
					}
				}
		}

		break;
	}
	}
}

void createbonus()
{
	bonusvisible = TRUE;
	drawbonus(292, 18);
}

void initbonusmode()
{
	int i;
	bonusmode = TRUE;
	erasebonus();
	ginten(1);
	bonustimeleft = 250 - GetLevelDifficultyModifier(g_GameData[g_CurrentPlayer].level) * 20;
	startbonustimeleft = 20;
	for (i = 0; i < g_DiggersCount; i++)
		g_DiggerData[i].MonsterEatScoreMultiplier = 1;
}

void endbonusmode()
{
	bonusmode = FALSE;
	ginten(0);
}

void erasebonus()
{
	if (bonusvisible) {
		bonusvisible = FALSE;
		EraseSprite(FIRSTBONUS);
	}
	ginten(0);
}

short reversedir(short dir)
{
	switch (dir) {
	case DIR_RIGHT: return DIR_LEFT;
	case DIR_LEFT: return DIR_RIGHT;
	case DIR_UP: return DIR_DOWN;
	case DIR_DOWN: return DIR_UP;
	}
	return dir;
}

bool checkdiggerunderbag(short h, short v)
{
	int n;
	for (n = g_CurrentPlayer; n < g_DiggersCount + g_CurrentPlayer; n++)
		if (g_DiggerData[n].IsAlive)
			if (g_DiggerData[n].mdir == DIR_UP || g_DiggerData[n].mdir == DIR_DOWN)
				if ((g_DiggerData[n].x - 12) / 20 == h)
					if ((g_DiggerData[n].y - 18) / 18 == v || (g_DiggerData[n].y - 18) / 18 + 1 == v)
						return TRUE;
	return FALSE;
}

void killdigger(int n, short stage, short bag)
{
	if (g_DiggerData[n].invin)
		return;
	if (g_DiggerData[n].deathstage < 2 || g_DiggerData[n].deathstage>4) {
		g_DiggerData[n].IsAlive = FALSE;
		g_DiggerData[n].deathstage = stage;
		g_DiggerData[n].DeathBagIndex = bag;
	}
}

// Fill "emfield" with 1 bits for each emerald, depending on emmask.
void makeemfield()
{
	short x, y;
	emmask = 1 << g_CurrentPlayer;
	for (x = 0; x < MWIDTH; x++)
		for (y = 0; y < MHEIGHT; y++)
			if (GetLevelSymbol(x, y, GetLevelMap(g_GameData[g_CurrentPlayer].level)) == 'C')
				emfield[y*MWIDTH + x] |= emmask;
			else
				emfield[y*MWIDTH + x] &= ~emmask;
}

void drawemeralds()
{
	short x, y;
	emmask = 1 << g_CurrentPlayer;
	for (x = 0; x < MWIDTH; x++)
		for (y = 0; y < MHEIGHT; y++)
			if (emfield[y*MWIDTH + x] & emmask)
				drawemerald(x * 20 + 12, y * 18 + 21);
}

short embox[8] = { 8,12,12,9,16,12,6,9 };

// Check to see if emerald was hit by something. Since the emeralds are not
// sprites like the bags, gold, bonuses, monsters, diggers and fireballs, this
// works differently.
bool hitemerald(short x, short y, short rx, short ry, short dir)
{
	bool hit = FALSE;
	short r;
	if (dir != DIR_RIGHT && dir != DIR_UP && dir != DIR_LEFT && dir != DIR_DOWN)
		return hit;
	if (dir == DIR_RIGHT && rx != 0)
		x++;
	if (dir == DIR_DOWN && ry != 0)
		y++;
	if (dir == DIR_RIGHT || dir == DIR_LEFT)
		r = rx;
	else
		r = ry;
	if (emfield[y*MWIDTH + x] & emmask) {
		if (r == embox[dir]) {
			drawemerald(x * 20 + 12, y * 18 + 21);
			IncrementPenalty();
		}
		if (r == embox[dir + 1]) {
			eraseemerald(x * 20 + 12, y * 18 + 21);
			IncrementPenalty();
			hit = TRUE;
			emfield[y*MWIDTH + x] &= ~emmask;
		}
	}
	return hit;
}

// return emeralds count left on game g_Field
short GetEmeraldsCount()
{
	short x, y, n = 0;
	for (x = 0; x < MWIDTH; x++)
		for (y = 0; y < MHEIGHT; y++)
			if (emfield[y*MWIDTH + x] & emmask)
				n++;
	return n;
}

void killemerald(short x, short y)
{
	if (emfield[(y + 1)*MWIDTH + x] & emmask) {
		emfield[(y + 1)*MWIDTH + x] &= ~emmask;
		eraseemerald(x * 20 + 12, (y + 1) * 18 + 21);
	}
}

int diggerx(int n)
{
	return g_DiggerData[n].x;
}

int diggery(int n)
{
	return g_DiggerData[n].y;
}

bool digalive(int n)
{
	return g_DiggerData[n].IsAlive;
}

// checks if at least one player is alive
bool isalive()
{
	int i;

	for (i = g_CurrentPlayer; i < g_DiggersCount + g_CurrentPlayer; i++)
	{
		if (g_DiggerData[i].IsAlive)
		{
			// at least one player is alive
			return 1;
		}
	}

	return 0;
}

/// <summary>
/// Initialize lives of players.
/// </summary>
void InitializeLives()
{
	int i;

	for (i = 0; i < g_DiggersCount + g_PlayersCount - 1; i++)
	{
		g_DiggerData[i].LivesCount = 3;
	}
}

// get number of player lives
int getlives(int pl)
{
	return g_DiggerData[pl].LivesCount;
}

// add 1 life to player
void addlife(int pl)
{
	g_DiggerData[pl].LivesCount++;

	// play sound
	sound1up();
}

// decrease lives of player by 1
void declife(int pl)
{
	// decrease only in standard game mode
	if (!g_IsGauntletMode)
	{
		g_DiggerData[pl].LivesCount--;
	}
}
