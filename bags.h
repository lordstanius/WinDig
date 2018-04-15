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

#include "Main.h"
#include "digger.h"

/// <summary>
/// The default bags gold timeout.
/// </summary>
#define BAGS_GOLD_TIMEOUT 150

/// <summary>
/// The default bags wobble timeout.
/// </summary>
#define BAGS_WOBBLE_TIMEOUT 15

/// <summary>
/// The bags gold timeout difficulty modifier.
/// </summary>
#define BAGS_GOLD_TIMEOUT_DIFFICULTY_MODIFIER 10

void dobags();
void updatebag(short bag);
void baghitground(short bag);
bool pushbag(short bag, short dir);
void RemoveBag(short bn);
void CollectGold(short bag);

/// <summary>
/// Gets the moving bags count.
/// </summary>
/// <returns>
/// The moving bags count.
/// </returns>
short GetMovingBagsCount();

void cleanupbags();
void InitBags();
void DrawBags();
bool IsPushBags(short dir, int *clfirst, int *clcoll);
bool IsPushBagsVertical(int *clfirst, int *clcoll);
short bagy(short bag);
short GetBagMovementDirection(short bag);
void removebags(int *clfirst, int *clcoll);
bool bagexist(int bag);
void UpdateBagsForPlayer(int player);
void UpdateBagsFromPlayer(int player);

/// <summary> Information describing the game </summary>
extern _game g_GameData[MAX_PLAYERS];

extern bool g_MonsterGotGold;

extern Digger g_DiggerData[DIGGERS];
