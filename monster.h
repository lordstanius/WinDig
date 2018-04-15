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

#ifndef MONSTER_H
#define MONSTER_H

/// <summary>
/// Default monster gap time.
/// </summary>
#define MONSTER_GAP_TIME 45

/// <summary>
/// Maximum monsters.
/// </summary>
#define MONSTERS 6

//#define MONSTER_GAP_TIME_DIFFICULTY_MODIFIER 2

void domonsters();
void incmont(short n);
void EraseMonsterSprites();
void InitializeMonsters();
short MonstersLeft();
void killmon(short mon);
short killmonsters(int *clfirst, int *clcoll);
void checkmonscared(short h);
void squashmonsters(short bag, int *clfirst, int *clcoll);

void createmonster();
void monai(short mon);
void mondie(short mon);
bool fieldclear(short dir, short x, short y);
void squashmonster(short mon, short death, short bag);
short nmononscr();

short GetField(short x, short y);

#endif