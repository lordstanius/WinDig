/*
-----------------------------------------------------------------------
<copyright file="Scores.c" company="Petr Abdulin">
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
#include <stdio.h>
#include <stdlib.h>
#include "def.h"
#include "Types.h"
#include "Scores.h"
#include "Main.h"
#include "Drawing.h"
#include "hardware.h"
#include "sound.h"
#include "Sprite.h"
#include "input.h"
#include "digger.h"
#include "record.h"
#include "win_dig.h"


/// <summary>
/// Score data structure.
/// </summary>
struct ScoreDataStruct
{
	// current score
	int score;

	// next bonus score?
	int nextbs;
} g_ScoreData[DIGGERS]; // allocate for 2 diggers

/// <summary> 
/// The high score values.
/// </summary>
int g_ScoreValue[(HIGH_SCORE_PLACES + 2)] = { 0 };

/// <summary> 
/// Stores high score names (3 chars per name).
/// </summary>
char g_ScoreInitials[(HIGH_SCORE_PLACES + 1)][(HIGH_SCORE_NAME_LEN + 1)];

int scoret = 0;

/// <summary>
/// Digger scores file size.
/// </summary>
#define DIGGER_SCO_FILE_SIZE 512

/// <summary>
/// Digger scores file entry size.
/// </summary>
#define DIGGER_SCO_ENTRY_SIZE 111

/// <summary>
/// Gets the digger levels size in external levels file.
/// </summary>
#define DIGGER_LEVELS_FILE_SIZE 1202

char g_ScoreDataBuffer[DIGGER_SCO_FILE_SIZE];

/// <summary>
/// The score value to gain additional live.
/// </summary>
ushort g_BonusScore = BONUS_LIFE_SCORE;

/// <summary>
/// Scores file name.
/// </summary>
#define DIGGER_SCO_FILE_NAME "DIGGER.SCO"

/// <summary>
/// Reads the scores file.
/// </summary>
void ReadScoresFile()
{
	FILE *in;

	g_ScoreDataBuffer[0] = 0;

	// if external levels file is not used
	if (!g_LevelFileFlag)
	{
		if ((in = fopen(DIGGER_SCO_FILE_NAME, "rb")) != NULL)
		{
			fread(g_ScoreDataBuffer, DIGGER_SCO_FILE_SIZE, 1, in);
			fclose(in);
		}
	}
	else
	{
		if ((in = fopen(g_LevelFileName, "rb")) != NULL)
		{
			// seek to scores pos start
			fseek(in, DIGGER_LEVELS_FILE_SIZE, 0);

			fread(g_ScoreDataBuffer, DIGGER_SCO_FILE_SIZE, 1, in);
			fclose(in);
		}
	}
}

/// <summary>
/// Writes g_ScoreDataBuffer data to the appropriate the scores file.
/// </summary>
void WriteScoresFile()
{
	FILE *out;

	if (!g_LevelFileFlag)
	{
		if ((out = fopen(DIGGER_SCO_FILE_NAME, "wb")) != NULL)
		{
			fwrite(g_ScoreDataBuffer, DIGGER_SCO_FILE_SIZE, 1, out);
			fclose(out);
		}
	}
	else
	{
		if ((out = fopen(g_LevelFileName, "r+b")) != NULL)
		{
			// seek to score save pos
			fseek(out, DIGGER_LEVELS_FILE_SIZE, 0);

			fwrite(g_ScoreDataBuffer, DIGGER_SCO_FILE_SIZE, 1, out);
			fclose(out);
		}
	}
}

void initscores()
{
	int i;
	for (i = 0; i < g_DiggersCount; i++)
		AddScore(i, 0);
}

/// <summary>
/// Loads the game scores.
/// </summary>
/// <remarks>
/// The entries in score file are as follows
/// 1: single player
/// 2: single player gautlet mode
/// 3: two players
/// 4: two players gauntlet mode
/// </remarks>
void LoadGameScores()
{
	char highbuf[10];

	// indexer
	short i;

	// 
	short x;

	// score data buffer pointer
	short p = 0;

	ReadScoresFile();

	// gauntlet mode data
	if (g_IsGauntletMode)
	{
		p = DIGGER_SCO_ENTRY_SIZE;
	}

	// second digger
	if (g_DiggersCount == 2)
	{
		p += DIGGER_SCO_ENTRY_SIZE * 2;
	}

	// if no data found
	if (g_ScoreDataBuffer[p++] != 's')
	{
		for (i = 0; i < (HIGH_SCORE_PLACES + 1); i++)
		{
			g_ScoreValue[i + 1] = 0;

			strcpy(g_ScoreInitials[i], "...");
		}
	}
	else
	{
		// read data
		for (i = 1; i < (HIGH_SCORE_PLACES + 1); i++)
		{
			for (x = 0; x < 3; x++)
			{
				g_ScoreInitials[i][x] = g_ScoreDataBuffer[p++];
			}

			p += 2;

			for (x = 0; x < 6; x++)
			{
				highbuf[x] = g_ScoreDataBuffer[p++];
			}

			g_ScoreValue[i + 1] = atol(highbuf);
		}
	}
}

void zeroscores()
{
	g_ScoreData[0].score = g_ScoreData[1].score = 0;
	g_ScoreData[0].nextbs = g_ScoreData[1].nextbs = g_BonusScore;
	scoret = 0;
}

void writecurscore(int col)
{
	if (g_CurrentPlayer == 0)
		DrawNumber(g_ScoreData[0].score, 0, 0, 6, col);
	else
		if (g_ScoreData[1].score < 100000L)
			DrawNumber(g_ScoreData[1].score, 236, 0, 6, col);
		else
			DrawNumber(g_ScoreData[1].score, 248, 0, 6, col);
}

void drawscores()
{
	DrawNumber(g_ScoreData[0].score, 0, 0, 6, 3);
	if (g_PlayersCount == 2 || g_DiggersCount == 2)
		if (g_ScoreData[1].score < 100000L)
			DrawNumber(g_ScoreData[1].score, 236, 0, 6, 3);
		else
			DrawNumber(g_ScoreData[1].score, 248, 0, 6, 3);
}

/// <summary>
/// Adds a score to a total score.
/// </summary>
/// <param name="n">The player number.</param>
/// <param name="score">The score to add.</param>
void AddScore(int n, short score)
{
	g_ScoreData[n].score += score;

	if (g_ScoreData[n].score > 999999L)
	{
		g_ScoreData[n].score = 0;
	}

	if (n == 0)
	{
		DrawNumber(g_ScoreData[n].score, 0, 0, 6, 1);
	}
	else
	{
		if (g_ScoreData[n].score < 100000L)
		{
			DrawNumber(g_ScoreData[n].score, 236, 0, 6, 1);
		}
		else
		{
			DrawNumber(g_ScoreData[n].score, 248, 0, 6, 1);
		}
	}

	/* +n to reproduce original bug */
	if (g_ScoreData[n].score >= g_ScoreData[n].nextbs + n && g_ScoreData[n].score < 1000000L)
	{
		if (getlives(n) < 5 || unlimlives)
		{
			if (g_IsGauntletMode)
			{
				g_CurrentGauntletGameTime += (GAME_TIME_SECOND_MULTIPLIER * 15L); /* 15 second time bonus instead of the life */
			}
			else
			{
				addlife(n);
			}

			DrawLivesAndGauntletTime();
		}

		g_ScoreData[n].nextbs += g_BonusScore;
	}

	// wtf ???
	IncrementPenalty();
	IncrementPenalty();
	IncrementPenalty();
}

void endofgame()
{
	short i;
	bool initflag = FALSE;
	for (i = 0; i < g_DiggersCount; i++)
		AddScore(i, 0);
	if (g_IsRecordingPlayback || !g_IsDrfValid)
		return;
	if (g_IsGauntletMode) {
		cleartopline();
		DrawGameText("TIME UP", 120, 0, 3);
		for (i = 0; i < 50 && !g_IsGameCycleEnd; i++)
			NextGameFrame();
		DrawGameText("       ", 120, 0, 3);
	}
	for (i = g_CurrentPlayer; i < g_CurrentPlayer + g_DiggersCount; i++) {
		scoret = g_ScoreData[i].score;
		if (scoret > g_ScoreValue[(HIGH_SCORE_PLACES + 1)]) {
			GraphicsClear();
			drawscores();
			strcpy(pldispbuf, "PLAYER ");
			if (i == 0)
				strcat(pldispbuf, "1");
			else
				strcat(pldispbuf, "2");
			DrawGameText(pldispbuf, 108, 0, 2);
			DrawGameText(" NEW HIGH SCORE ", 64, 40, 2);
			getinitials();
			shufflehigh();
			SaveScores();
			initflag = TRUE;
		}
	}
	if (!initflag && !g_IsGauntletMode) {
		cleartopline();
		DrawGameText("GAME OVER", 104, 0, 3);
		for (i = 0; i < 50 && !g_IsGameCycleEnd; i++)
			NextGameFrame();
		DrawGameText("         ", 104, 0, 3);
		SetWaitForVerticalRetraceFlag(TRUE);
	}
}

/// <summary>
/// Draws high scores table in main game menu.
/// </summary>
void DrawHighScores()
{
	char scoreBuffer[10];
	char lineBuffer[36];
	short i;
	short color = COLOR_2;

	DrawGameText("HIGH SCORES", 16, 25, COLOR_3);

	for (i = 1; i < (HIGH_SCORE_PLACES + 1); i++)
	{
		strcpy(lineBuffer, "");
		strcat(lineBuffer, g_ScoreInitials[i]);
		strcat(lineBuffer, " ");
		NumberToString(scoreBuffer, g_ScoreValue[i + 1]);
		strcat(lineBuffer, scoreBuffer);

		DrawGameText(lineBuffer, 16, 31 + 13 * i, color);

		// all places except first are other color
		color = COLOR_1;
	}
}

/// <summary>
/// Writes score data to buffer, then saves score data to file using WriteScoresFile().
/// </summary>
void SaveScores()
{
	// temp buffers
	char highbuf[10];
	char hsbuf[36];

	// indices
	short i, j;

	// offset in buffer
	short p = 0;

	// set offset in buffer
	if (g_IsGauntletMode)
	{
		p = DIGGER_SCO_ENTRY_SIZE;
	}

	// set offset in buffer
	if (g_DiggersCount == 2)
	{
		p += DIGGER_SCO_ENTRY_SIZE * 2;
	}

	// set marker
	strcpy(g_ScoreDataBuffer + p, "s");

	// for each high score
	for (i = 1; i < (HIGH_SCORE_PLACES + 1); i++)
	{
		strcpy(hsbuf, "");

		// save initials
		strcat(hsbuf, g_ScoreInitials[i]);

		strcat(hsbuf, " ");

		NumberToString(highbuf, g_ScoreValue[i + 1]);

		// save score
		strcat(hsbuf, highbuf);

		// copy actual data
		for (j = 0; j < HIGH_SCORE_ENTRY_SIZE; j++)
		{
			// pointer + symbol + entry offset + const "s" offset
			g_ScoreDataBuffer[p + j + ((i - 1)*HIGH_SCORE_ENTRY_SIZE) + 1] = hsbuf[j];
		}
	}

	// write to file
	WriteScoresFile();
}

void getinitials()
{
	short k, i;

	pause_windows_sound_playback();

	NextGameFrame();
	DrawGameText("ENTER YOUR", 100, 70, 3);
	DrawGameText(" INITIALS", 100, 90, 3);
	DrawGameText("_ _ _", 128, 130, 3);
	strcpy(g_ScoreInitials[0], "...");
	killsound();
	for (i = 0; i < 3; i++) {
		k = 0;
		while (k == 0) {
			k = getinitial(i * 24 + 128, 130);
			if (k == 8 || k == 127) {
				if (i > 0)
					i--;
				k = 0;
			}
		}
		if (k != 0) {
			g_DrawSymbolFunc(i * 24 + 128, 130, k, 3);
			g_ScoreInitials[0][i] = (char)k;
		}
	}
	for (i = 0; i < 20; i++)

		flashywait(2);

	setupsound();
	GraphicsClear();
	gpal(0);
	ginten(0);
	SetWaitForVerticalRetraceFlag(TRUE);
	recputinit(g_ScoreInitials[0]);

	resume_windows_sound_playback();

}

void flashywait(short n)
{
	short i, gt, cx, p = 0;
	sbyte gap = 19;
	SetWaitForVerticalRetraceFlag(FALSE);
	for (i = 0; i < (n << 1); i++)
	{
		for (cx = 0; cx < g_Volume; cx++)
		{
			gpal(p = 1 - p);

			for (gt = 0; gt < gap; gt++)
			{
				ProcessWindowsEvents();
			}

		}
	}
}

short getinitial(short x, short y)
{
	// short i; unreferenced

	UINT c;


	g_DrawSymbolFunc(x, y, '_', 3);
	do {

		ProcessWindowsEvents();
		if (kbhit())
		{
			c = LOWORD(MapVirtualKey(getkey(), 2));
			if (IsCharAlphaNumeric(c) || c == '.' || c == '\b' || c == ' ')
				return (c);
		}
		flashywait(5);

	} while (1);
}

void shufflehigh()
{
	short i, j;
	for (j = 10; j > 1; j--)
		if (scoret < g_ScoreValue[j])
			break;
	for (i = 10; i > j; i--) {
		g_ScoreValue[i + 1] = g_ScoreValue[i];
		strcpy(g_ScoreInitials[i], g_ScoreInitials[i - 1]);
	}
	g_ScoreValue[j + 1] = scoret;
	strcpy(g_ScoreInitials[j], g_ScoreInitials[0]);
}

void scorekill(int n)
{
	AddScore(n, 250);
}

void scorekill2()
{
	AddScore(0, 125);
	AddScore(1, 125);
}

void scoreemerald(int n)
{
	AddScore(n, 25);
}

void scoreoctave(int n)
{
	AddScore(n, 250);
}

void scoregold(int n)
{
	AddScore(n, 500);
}

void scorebonus(int n)
{
	AddScore(n, 1000);
}

void scoreeatm(int n, int multiplier)
{
	AddScore(n, multiplier * 200);
}

/// <summary>
/// Draw number.
/// </summary>
/// <param name="n">The number to draw.</param>
/// <param name="x">The x coordinate.</param>
/// <param name="y">The y coordinate.</param>
/// <param name="width">The width of number (for padding).</param>
/// <param name="color">The color.</param>
void DrawNumber(int n, short x, short y, short width, short color)
{
	short d;
	short xPos = (width - 1) * SYMBOL_WIDTH + x;

	// reduce to 6 digits
	// TODO is width argument necessary then?
	n %= 1000000L;

	while (width > 0)
	{
		// get next digit
		d = (short)(n % 10);

		// don't draw leading zeroes
		if (width > 1 || d > 0)
		{
			g_DrawSymbolFunc(xPos, y, d + '0', color);
		}

		// reduce by 10 to get next digit
		n /= 10;

		// shift position
		xPos -= SYMBOL_WIDTH;

		// cycle counter
		width--;
	}
}

/// <summary>
/// Converts number to padded string.
/// </summary>
/// <param name="outBuf">[in,out] Buffer for string data.</param>
/// <param name="n">The number to convert.</param>
void NumberToString(char *outBuf, int n)
{
	int x;

	// 7 digits
	for (x = 0; x < 7; x++)
	{
		// get next char
		outBuf[6 - x] = (sbyte)(n % 10L) + '0';

		n /= 10L;

		// no more positions
		if (n == 0L)
		{
			// end cycle
			x++;
			break;
		}
	}

	// pad remaining places with spaces
	for (; x < 7; x++)
	{
		outBuf[6 - x] = ' ';
	}

	// zero terminator
	outBuf[7] = 0;
}
