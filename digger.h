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

#ifndef DIGGER_H
#define DIGGER_H

typedef struct DiggerStruct
{
	short x, y, HorizontalPosition, VerticalPositon, rx, ry, mdir, Direction, rechargetime, fx, fy, fdir, expsn,
		deathstage, DeathBagIndex, deathani, deathtime, emocttime, emn, MonsterEatScoreMultiplier, LivesCount, ivt;


	/// <summary> 
	/// Specifies whether is digger is in bag pushing state now.
	/// </summary>
	short IsPushingBag;

	bool notfiring, IsAlive, firepressed, dead, levdone, invin;
} Digger;


#define GRAVE_ANIMATION_FRAMES 7

#define DEATH_STAGE_BAG 1
#define DEATH_STAGE_GRAVE 2
#define DEATH_STAGE_HIT 3
#define DEATH_STAGE_FLIP 5
#define DEATH_STAGE_FINAL 4

void updatedigger(int n);
void DiggerDeath(int n);
void initbonusmode();
void endbonusmode();
void drawdig(int n, int d, int x, int y, bool f);

void DoDiggerMove();
void erasediggers();
void killfire(int n);
void erasebonus();
short GetEmeraldsCount();
void makeemfield();
void drawemeralds();
void InitializeDigger();
void drawexplosion(int n);
void updatefire(int n);
void createbonus();
short reversedir(short d);
bool hitemerald(short x, short y, short rx, short ry, short dir);
void killdigger(int n, short bp6, short bp8);
bool checkdiggerunderbag(short h, short v);
void killemerald(short bpa, short bpc);
void NextGameFrame();
int diggerx(int n);
int diggery(int n);
void sceatm(int n);
bool isalive();
bool digalive(int n);
int getlives(int pl);
void addlife(int pl);
void InitializeLives();
void declife(int pl);

extern bool bonusvisible, digonscr, bonusmode;
extern uint g_GameFrameTime, g_CurrentGameTime, g_CurrentGauntletGameTime;

#endif