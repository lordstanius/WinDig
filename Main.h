/*
-----------------------------------------------------------------------
<copyright file="Main.h" company="Petr Abdulin">
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

#include "Types.h"

#ifndef MAIN_H
#define MAIN_H

/// <summary>
/// Maximum number of players.
/// </summary>
#define MAX_PLAYERS 2

/// <summary>
/// The game time multipliers.
/// </summary>
#define GAME_TIME_TICK_MULTIPLIER 1193L
#define GAME_TIME_SECOND_MULTIPLIER 1193181L

// current game data
typedef struct
{
    // current level
    short level;

    // is level completed
    bool levdone;
} _game;

short GetLevelSymbol(short bp6,short bp8,short bpa);
void IncrementPenalty();

void setdead(bool df);
void cleartopline();
void ReleaseResources();
short randno(short n);
void GameCycle();
void GameEngineInitialize();

/// <summary>
/// Main function that actually runs the game.
/// </summary>
void GameRun();

void DrawGameMode();
void switchnplayers();
void DrawScreen();
void InitializeCharacters();
void checklevdone();
void testpause();
void ParseCommandLine(int argc,char *argv[]);
void patchcga();
void initlevel();
void inir();
void redefkeyb(bool allf);
int getalllives();

extern short g_PlayersCount,g_DiggersCount,g_CurrentPlayer,g_StartingLevel;
extern bool g_LevelFileFlag;
extern char g_LevelFileName[];
extern char pldispbuf[];
extern int g_RandomValue;
extern sbyte g_LevelsData[8][10][15];
extern int g_OptionGauntletTimeInSeconds;
extern bool g_IsGauntletMode,g_IsTimeout,unlimlives;

#endif