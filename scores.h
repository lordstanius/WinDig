/*
-----------------------------------------------------------------------
<copyright file="Scores.h" company="Petr Abdulin">
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

/// <summary>
/// The high score entry name length.
/// </summary>
#define HIGH_SCORE_NAME_LEN 3

/// <summary>
/// The high score places count.
/// </summary>
#define HIGH_SCORE_PLACES 10

/// <summary>
/// The high score entry size.
/// </summary>
#define HIGH_SCORE_ENTRY_SIZE 11

/// <summary>
/// Default bonus life score.
/// </summary>
#define BONUS_LIFE_SCORE 20000

void ReadScoresFile();
void WriteScoresFile();
void SaveScores();
void getinitials();
void flashywait(short n);
short getinitial(short x, short y);
void shufflehigh();
void DrawNumber(int n, short x, short y, short w, short c);
void NumberToString(char *p, int n);

void LoadGameScores();
void DrawHighScores();
void zeroscores();
void writecurscore(int col);
void drawscores();
void initscores();
void endofgame();
void scorekill(int n);
void scorekill2();
void scoreemerald(int n);
void scoreoctave(int n);
void scoregold(int n);
void scorebonus(int n);
void scoreeatm(int n, int msc);
void AddScore(int n, short score);


extern ushort g_BonusScore;
extern int scoret;
extern char g_ScoreInitials[(HIGH_SCORE_PLACES + 1)][(HIGH_SCORE_NAME_LEN + 1)];
