/*
-----------------------------------------------------------------------
<copyright file="Main.c" company="Petr Abdulin">
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
#include <string.h>
#include <stdio.h>
#include "def.h"
#include "Types.h"
#include "hardware.h"
#include "sound.h"
#include "Sprite.h"
#include "input.h"
#include "Scores.h"
#include "Drawing.h"
#include "digger.h"
#include "monster.h"
#include "Bags.h"
#include "record.h"
#include "Main.h"
#include "newsnd.h"
#include "ini.h"
#include "win_dig.h"
#include "win_snd.h"
#include "win_vid.h"
#include "Map.h"

short g_GameFrame;

/// <summary> Information describing the game </summary>
_game g_GameData[MAX_PLAYERS];

// ???
char pldispbuf[14];

/// <summary> 
/// Current player number (0 - first, 1 - second).
/// </summary>
short g_CurrentPlayer = 0;

/// <summary> 
/// Number of simultaneous players.
/// </summary>
short g_PlayersCount = 1;

/// <summary> 
/// The global penalty value ?.
/// </summary>
short g_Penalty = 0;

/// <summary> 
/// Number of diggers in game. 
/// </summary>
/// <remarks>
/// This is different from g_PlayersCount since there 
/// can be one digger, but 2 players (hot seat).
/// </remarks>
short g_DiggersCount = 1;

// starting level
short g_StartingLevel = 1;

/// <summary> 
/// Value indicating whether the level in not drawn.
/// </summary>
bool g_IsLevelNotDrawn = FALSE;

// ? all monsters are dead
bool g_IsAllMonstersDead = FALSE;

// ? unlimited lives
bool unlimlives = FALSE;

// ? is game started
bool g_IsStarted;

/// <summary> 
/// External levels file name.
/// </summary>
char g_LevelFileName[STRBUF_512];

/// <summary> 
/// Level file flag.
/// True if external levels file is used.
/// </summary>
bool g_LevelFileFlag = FALSE;

// ???
bool biosflag = FALSE;

// ???
int delaytime = 0;

// ? game time
int g_OptionGauntletTimeInSeconds = 0;

// Is game in Gauntlet mode.
// Limited by time, but with unlimited lives.
// Default is off.
bool g_IsGauntletMode = FALSE;

/// <summary> 
/// Value indicating whether the timeout is reached in gauntlet mode.
/// </summary>
bool g_IsTimeout = FALSE;

/// <summary> 
/// Levels of the game layout.
/// </summary>
/// <remarks>
/// S - spot;
/// H - horizontal space;
/// V - vertical space;
/// C - crystal;
///   - (space) rock.
/// Also note that nonetheless strings are 16 bytes long because of terminating '\0' character,
/// this is not a bug, ignore intellisense warning.
/// </remarks>
sbyte g_LevelsData[MAX_LEVELS][MHEIGHT][MWIDTH] =
{
	{"S   B     HHHHS",
	"V  CC  C  V B  ",
	"VB CC  C  V    ",
	"V  CCB CB V CCC",
	"V  CC  C  V CCC",
	"HH CC  C  V CCC",
	" V    B B V    ",
	" HHHH     V    ",
	"C   V     V   C",
	"CC  HHHHHHH  CC"},

	{"SHHHHH  B B  HS",
	" CC  V       V ",
	" CC  V CCCCC V ",
	"BCCB V CCCCC V ",
	"CCCC V       V ",
	"CCCC V B  HHHH ",
	" CC  V CC V    ",
	" BB  VCCCCV CC ",
	"C    V CC V CC ",
	"CC   HHHHHH    "},

	{"SHHHHB B BHHHHS",
	"CC  V C C V BB ",
	"C   V C C V CC ",
	" BB V C C VCCCC",
	"CCCCV C C VCCCC",
	"CCCCHHHHHHH CC ",
	" CC  C V C  CC ",
	" CC  C V C     ",
	"C    C V C    C",
	"CC   C H C   CC"},

	{"SHBCCCCBCCCCBHS",
	"CV  CCCCCCC  VC",
	"CHHH CCCCC HHHC",
	"C  V  CCC  V  C",
	"   HHH C HHH   ",
	"  B  V B V  B  ",
	"  C  VCCCV  C  ",
	" CCC HHHHH CCC ",
	"CCCCC CVC CCCCC",
	"CCCCC CHC CCCCC"},

	{"SHHHHHHHHHHHHHS",
	"VBCCCCBVCCCCCCV",
	"VCCCCCCV CCBC V",
	"V CCCC VCCBCCCV",
	"VCCCCCCV CCCC V",
	"V CCCC VBCCCCCV",
	"VCCBCCCV CCCC V",
	"V CCBC VCCCCCCV",
	"VCCCCCCVCCCCCCV",
	"HHHHHHHHHHHHHHH"},

	{"SHHHHHHHHHHHHHS",
	"VCBCCV V VCCBCV",
	"VCCC VBVBV CCCV",
	"VCCCHH V HHCCCV",
	"VCC V CVC V CCV",
	"VCCHH CVC HHCCV",
	"VC V CCVCC V CV",
	"VCHHBCCVCCBHHCV",
	"VCVCCCCVCCCCVCV",
	"HHHHHHHHHHHHHHH"},

	{"SHCCCCCVCCCCCHS",
	" VCBCBCVCBCBCV ",
	"BVCCCCCVCCCCCVB",
	"CHHCCCCVCCCCHHC",
	"CCV CCCVCCC VCC",
	"CCHHHCCVCCHHHCC",
	"CCCCV CVC VCCCC",
	"CCCCHH V HHCCCC",
	"CCCCCV V VCCCCC",
	"CCCCCHHHHHCCCCC"},

	{"HHHHHHHHHHHHHHS",
	"V CCBCCCCCBCC V",
	"HHHCCCCBCCCCHHH",
	"VBV CCCCCCC VBV",
	"VCHHHCCCCCHHHCV",
	"VCCBV CCC VBCCV",
	"VCCCHHHCHHHCCCV",
	"VCCCC V V CCCCV",
	"VCCCCCV VCCCCCV",
	"HHHHHHHHHHHHHHH"}
};

/// <summary>
/// Gets a level symbol.
/// </summary>
/// <param name="x">The x coordinate.</param>
/// <param name="y">The y coordinate.</param>
/// <param name="map">The map.</param>
/// <returns>
/// The level symbol on given map, for given position.
/// </returns>
short GetLevelSymbol(short x, short y, short map)
{
	// some dirty hack:
	// for maps 3 or 4,
	// not on custom map
	// for 2 player game
	// always return hole for pos (6, 9) and (8, 9)
	// I assume it's for placing diggers
	if ((map == 3 || map == 4)
		&& !g_LevelFileFlag
		&& g_DiggersCount == 2
		&& y == 9
		&& (x == 6 || x == 8))
	{
		return MAP_SYMBOL_HOLE;
	}

	// normally just return map symbol
	return g_LevelsData[map - 1][y][x];
}

/// <summary>
/// Games instance cycle.
/// </summary>
void GameCycle()
{
	// ???
	short t, c, i;

	// ? flash player number screen
	bool flashplayer = FALSE;

	show_game_menu();

	if (g_IsGauntletMode)
	{
		g_CurrentGauntletGameTime = g_OptionGauntletTimeInSeconds * GAME_TIME_SECOND_MULTIPLIER;
		g_IsTimeout = FALSE;
	}

	// initialize lives
	InitializeLives();

	// set start level
	g_GameData[0].level = g_StartingLevel;
	if (g_PlayersCount == 2)
	{
		g_GameData[1].level = g_StartingLevel;
	}

	g_IsAllMonstersDead = FALSE;

	// clear screen
	GraphicsClear();

	// initialize levels
	g_CurrentPlayer = 0;

	// init static layout
	initlevel();
	g_CurrentPlayer = 1;
	initlevel();

	// init score
	zeroscores();

	// 
	bonusvisible = TRUE;

	if (g_PlayersCount == 2)
	{
		// signal player number
		flashplayer = TRUE;
	}

	// current player
	g_CurrentPlayer = 0;

	// all before it was still a setup, and here comes the mother of them all...
	// while at least one player have lives, and g_IsGameCycleEnd is not pressed ?, and not timeout in gauntlet
	while (getalllives() != 0 && !g_IsGameCycleEnd && !g_IsTimeout)
	{
		// this is a main cycle for current player
		// while ??? g_IsGameCycleEnd is not pressed ?, and not timeout in gauntlet
		while (!g_IsAllMonstersDead && !g_IsGameCycleEnd && !g_IsTimeout)
		{
			// this is a sprite init, not much logic inside (if some at all)
			InitializeMonsterAndBagSprites();

			// ? get some random init
			if (g_IsRecordingPlayback)
			{
				g_RandomValue = playgetrand();
			}
			else
			{
				g_RandomValue = GetHighResolutionTime();
			}

			// record stuff
			RecordPutRandom(g_RandomValue);

			if (g_IsLevelNotDrawn)
			{
				g_IsLevelNotDrawn = FALSE;

				// draw main game screen
				DrawScreen();

				// flash player name
				if (flashplayer)
				{
					flashplayer = FALSE;
					strcpy(pldispbuf, "PLAYER ");
					if (g_CurrentPlayer == 0)
						strcat(pldispbuf, "1");
					else
						strcat(pldispbuf, "2");
					cleartopline();
					for (t = 0; t < 15; t++)
						for (c = 1; c <= 3; c++) {
							DrawGameText(pldispbuf, 108, 0, c);
							writecurscore(c);
							NextGameFrame();
							if (g_IsGameCycleEnd)
								return;
						}
					drawscores();
					for (i = 0; i < g_DiggersCount; i++)
						AddScore(i, 0);
				}
			}
			else
			{
				InitializeCharacters();
			}

			DrawGameText("        ", 108, 0, 3);

			initscores();
			DrawLivesAndGauntletTime();
			music(NORMAL_TUNE);

			FlushKeyboardBuffer();

			for (i = 0; i < g_DiggersCount; i++)
			{
				ReadDiggerDirection(i);
			}

			// most important, per-life game cycle:
			// either all monsters are killed, or level is completed or digger is killed or timeout is reached
			while (!g_IsAllMonstersDead && !g_GameData[g_CurrentPlayer].levdone && !g_IsGameCycleEnd && !g_IsTimeout)
			{
				g_Penalty = 0;

				// framerate control
				NextGameFrame();

				// 
				DoDiggerMove();

				domonsters();

				dobags();

				if (g_Penalty > 8)
				{
					incmont(g_Penalty - 8);
				}

				// test for pause press
				testpause();

				// check if level is done
				checklevdone();
			}

			erasediggers();

			musicoff();

			// maximum game cycles in postprocess ?
			t = 20;

			// if there is something moving on the level left
			while ((GetMovingBagsCount() != 0 || t != 0) && !g_IsGameCycleEnd && !g_IsTimeout)
			{
				if (t != 0)
				{
					t--;
				}

				g_Penalty = 0;

				dobags();

				NextGameFrame();

				DoDiggerMove();

				domonsters();

				if (g_Penalty < 8)
				{
					t = 0;
				}
			}

			soundstop();

			for (i = 0; i < g_DiggersCount; i++)
			{
				killfire(i);
			}

			erasebonus();

			cleanupbags();

			savefield();

			EraseMonsterSprites();

			recputeol();

			if (g_IsRecordingPlayback)
			{
				playskipeol();
			}

			if (g_IsGameCycleEnd)
			{
				recputeog();
			}

			if (g_GameData[g_CurrentPlayer].levdone)
			{
				soundlevdone();
			}

			if (GetEmeraldsCount() == 0 || g_GameData[g_CurrentPlayer].levdone)
			{
				for (i = g_CurrentPlayer; i < g_DiggersCount + g_CurrentPlayer; i++)
				{
					if (getlives(i) > 0 && !digalive(i))
					{
						declife(i);
					}
				}

				DrawLivesAndGauntletTime();

				g_GameData[g_CurrentPlayer].level++;

				if (g_GameData[g_CurrentPlayer].level > 1000)
				{
					g_GameData[g_CurrentPlayer].level = 1000;
				}

				initlevel();
			}
			else
			{
				if (g_IsAllMonstersDead)
				{
					for (i = g_CurrentPlayer; i < g_CurrentPlayer + g_DiggersCount; i++)
					{
						if (getlives(i) > 0)
						{
							declife(i);
						}
					}

					DrawLivesAndGauntletTime();
				}

				if ((g_IsAllMonstersDead && getalllives() == 0 && !g_IsGauntletMode && !g_IsGameCycleEnd) || g_IsTimeout)
				{
					endofgame();
				}
			}
		} // while (!g_IsAllMonstersDead && !g_IsGameCycleEnd && !g_IsTimeout) 

		g_IsAllMonstersDead = FALSE;

		// if two players and other player have some lives then perform player switch
		if (g_PlayersCount == 2 && getlives(1 - g_CurrentPlayer) != 0)
		{
			// change current player
			g_CurrentPlayer = 1 - g_CurrentPlayer;

			// signal player change and redraw level
			flashplayer = g_IsLevelNotDrawn = TRUE;
		}
	} // while (getalllives() != 0 && !g_IsGameCycleEnd && !g_IsTimeout) 
}

void GameEngineInitialize()
{
	g_Volume = 1;

	ginit();

	gpal(0);

	SetWaitForVerticalRetraceFlag(TRUE);

	initkeyb();

	detectjoy();

	inir();

	initsound();

	InitializeRecordingBuffer();
}

/// <summary>
/// Main function that runs the game.
/// However, for game cycle see GameCycle function.
/// </summary>
void GameRun()
{
	// ???
	short t, x;

	LoadGameScores();

	g_IsGameCycleEnd = FALSE;

	// main menu cycle
	while (!g_IsGameCycleEnd)
	{
		show_main_menu();
		soundstop();

		// create sprites
		CreateMonsterAndBagSprites();

		// detect joystick
		detectjoy();

		// clear back buffer
		GraphicsClear();

		// display title bitmap
		gtitle();

		// draw game title
		DrawGameText("D I G G E R", 100, 0, COLOR_3);

		// draw game mode (one player, two players etc.)
		DrawGameMode();

		// high score table
		DrawHighScores();

		g_IsStarted = FALSE;

		g_GameFrame = 0;

		// game speed control
		NextGameFrame();

		CalibrateJoystick();
		reset_main_menu_screen = FALSE;

		// while game is not started, i.e. we at the title screen
		while (!g_IsStarted)
		{
			g_IsStarted = CalibrateJoystick();

			if ((getcommand() == 19) && !g_IsGauntletMode && g_DiggersCount == 1)
			{
				switchnplayers();
				DrawGameMode();
				akeypressed = 0;
			}

			// this and below draws right side title screen animation
			if (g_GameFrame == 0)
			{
				for (t = 54; t < 174; t += 12)
				{
					// erase all text
					DrawGameText("            ", 164, t, 0);
				}
			}

			// draw nobbin
			if (g_GameFrame == 50)
			{
				MoveAndRedrawSprite(FIRSTMONSTER, 292, 63);
				x = 292;
			}

			// move to the left
			if (g_GameFrame > 50 && g_GameFrame <= 77) {
				x -= 4;
				drawmon(0, 1, DIR_LEFT, x, 63);
			}

			// turn right
			if (g_GameFrame > 77)
				drawmon(0, 1, DIR_RIGHT, 184, 63);

			// draw text
			if (g_GameFrame == 83)
				DrawGameText("NOBBIN", 216, 64, 2);

			// draw hobbin
			if (g_GameFrame == 90) {
				MoveAndRedrawSprite(FIRSTMONSTER + 1, 292, 82);
				drawmon(1, 0, DIR_LEFT, 292, 82);
				x = 292;
			}

			// move to the left
			if (g_GameFrame > 90 && g_GameFrame <= 117) {
				x -= 4;
				drawmon(1, 0, DIR_LEFT, x, 82);
			}

			// turn right
			if (g_GameFrame > 117)
				drawmon(1, 0, DIR_RIGHT, 184, 82);

			// draw text
			if (g_GameFrame == 123)
				DrawGameText("HOBBIN", 216, 83, 2);

			// draw digger
			if (g_GameFrame == 130) {
				MoveAndRedrawSprite(FIRSTDIGGER, 292, 101);
				drawdigger(0, DIR_LEFT, 292, 101, 1);
				x = 292;
			}

			// move to the left
			if (g_GameFrame > 130 && g_GameFrame <= 157) {
				x -= 4;
				drawdigger(0, DIR_LEFT, x, 101, 1);
			}

			// turn right
			if (g_GameFrame > 157)
				drawdigger(0, DIR_RIGHT, 184, 101, 1);

			// draw text
			if (g_GameFrame == 163)
				DrawGameText("DIGGER", 216, 102, 2);


			if (g_GameFrame == 178) {
				MoveAndRedrawSprite(FIRSTBAG, 184, 120);
				drawgold(0, 0, 184, 120);
			}
			if (g_GameFrame == 183)
				DrawGameText("GOLD", 216, 121, 2);
			if (g_GameFrame == 198)
				drawemerald(184, 141);
			if (g_GameFrame == 203)
				DrawGameText("EMERALD", 216, 140, 2);
			if (g_GameFrame == 218)
				drawbonus(184, 158);
			if (g_GameFrame == 223)
				DrawGameText("BONUS", 216, 159, 2);
			NextGameFrame();
			g_GameFrame++;
			if (g_GameFrame > 250)
				g_GameFrame = 0;

			if (reset_main_menu_screen) {
				g_IsGameCycleEnd = FALSE;
				break;
			}
		} // while (!g_IsStarted) 

		// if user pressed to save game
		if (g_IsSaveGameRecordKeyPressed)
		{
			// and if game record is available
			if (g_IsGameRecordAvailable)
			{
				// show dialog to save file
				RecordSaveFileDialog();
			}

			// reset flag
			g_IsSaveGameRecordKeyPressed = FALSE;

			// continue game cycle
			continue;
		}

		if (reset_main_menu_screen)
		{
			continue;
		}

		if (g_IsGameCycleEnd)
		{
			// exit main game cycle
			break;
		}

		// start recording of the game
		RecordingStart();

		// game instance cycle
		GameCycle();

		// since game is ended, we have the recording
		g_IsGameRecordAvailable = TRUE;

		// automatically show dialog if save file name was specified via command line
		if (g_IsCmdLineSaveFileNameAvailable)
		{
			RecordSaveFileDialog();
		}

		// reset state
		g_IsSaveGameRecordKeyPressed = FALSE;

		// reset state
		g_IsGameCycleEnd = FALSE;
	}
}

/// <summary>
/// Performs cleanup before program shutdown.
/// </summary>
void ReleaseResources()
{
	killsound();

	soundoff();

	soundkillglob();

	restorekeyb();

	windows_finish();
}

/// <summary>
/// Shows current game mode (normal, gauntlet) and
/// number of players (one, two, two simultaneous).
/// </summary>
/// <remarks>
/// The game status is a bit confusing.
/// </remarks>
void DrawGameMode()
{
	if (g_DiggersCount == 2)
	{
		if (g_IsGauntletMode)
		{
			// gauntlet 2 players simultaneous
			DrawGameText("TWO PLAYER", 180, 25, COLOR_3);
			DrawGameText("GAUNTLET", 192, 39, COLOR_3);
		}
		else
		{
			// normal 2 players simultaneous
			DrawGameText("TWO PLAYER", 180, 25, COLOR_3);
			DrawGameText("SIMULTANEOUS", 170, 39, COLOR_3);
		}
	}
	else
	{
		if (g_IsGauntletMode)
		{
			// gauntlet mode
			DrawGameText("GAUNTLET", 192, 25, COLOR_3);
			DrawGameText("MODE", 216, 39, COLOR_3);
		}
		else
		{
			if (g_PlayersCount == 1)
			{
				// one player
				DrawGameText("ONE", 220, 25, COLOR_3);
				DrawGameText(" PLAYER ", 192, 39, COLOR_3);
			}
			else
			{
				// two players "hot seat"
				DrawGameText("TWO", 220, 25, COLOR_3);
				DrawGameText(" PLAYERS", 184, 39, COLOR_3);
			}
		}
	}
}

int getalllives()
{
	int t = 0, i;
	for (i = g_CurrentPlayer; i < g_DiggersCount + g_CurrentPlayer; i++)
		t += getlives(i);
	return t;
}

void switchnplayers()
{
	g_PlayersCount = 3 - g_PlayersCount;
}

// Initialise level before it has been played at all.
void initlevel()
{
	g_GameData[g_CurrentPlayer].levdone = FALSE;

	// set dug holes
	makefield();

	// set emeralds
	makeemfield();

	// set bags
	InitBags();

	g_IsLevelNotDrawn = TRUE;
}

//  Redraw screen for other player
void DrawScreen()
{
	CreateMonsterAndBagSprites();

	drawstatics();

	DrawBags();

	drawemeralds();

	InitializeDigger();

	InitializeMonsters();
}

void InitializeCharacters()
{
	InitializeMonsterAndBagSprites();

	InitializeDigger();

	InitializeMonsters();
}

void checklevdone()
{
	if ((GetEmeraldsCount() == 0 || MonstersLeft() == 0) && isalive())
	{
		// if no more emeralds, or no more monsters and is alive
		g_GameData[g_CurrentPlayer].levdone = TRUE;
	}
	else
	{
		g_GameData[g_CurrentPlayer].levdone = FALSE;
	}
}

/// <summary>
/// Increments global penalty by 1.
/// </summary>
void IncrementPenalty()
{
	g_Penalty++;
}

void cleartopline()
{
	DrawGameText("                          ", 0, 0, 3);
	DrawGameText(" ", 308, 0, 3);
}



void setdead(bool df)
{
	g_IsAllMonstersDead = df;
}

void testpause()
{
	int i;
	if (g_IsPaused) {

		soundpause();
		sett2val(40);
		setsoundt2();
		cleartopline();
		DrawGameText("PRESS ANY KEY", 80, 0, 1);
		getkey();
		g_IsPaused = FALSE;
		cleartopline();
		drawscores();
		for (i = 0; i < g_DiggersCount; i++)
			AddScore(i, 0);
		DrawLivesAndGauntletTime();


		g_CurrentGameTime = GetHighResolutionTime();

	}
	else
		soundpauseoff();
}

ushort sound_device, sound_port, sound_irq, sound_dma, g_SoundRate, sound_length;

/// <summary>
/// Parses command line.
/// </summary>
/// <param name="argc">The number of arguments.</param>
/// <param name="argv">[in,out] The arguments.</param>
void ParseCommandLine(int argc, char *argv[])
{
	char *word;
	short arg, i, j, speedmul;
	bool sf, gs = FALSE, norepf = FALSE;
	FILE *levf;

	for (arg = 1; arg < argc; arg++) {
		word = argv[arg];
		if (word[0] == '/' || word[0] == '-') {
			if (word[1] == 'L' || word[1] == 'l' || word[1] == 'R' || word[1] == 'r' ||
				word[1] == 'P' || word[1] == 'p' || word[1] == 'S' || word[1] == 's' ||
				word[1] == 'E' || word[1] == 'e' || word[1] == 'G' || word[1] == 'g' ||
				word[1] == 'A' || word[1] == 'a' || word[1] == 'I' || word[1] == 'i')
				if (word[2] == ':')
					i = 3;
				else
					i = 2;
			if (word[1] == 'L' || word[1] == 'l') {
				j = 0;
				while (word[i] != 0)
					g_LevelFileName[j++] = word[i++];
				g_LevelFileName[j] = word[i];
				g_LevelFileFlag = TRUE;
			}
			if (word[1] == 'R' || word[1] == 'r')
				recname(word + i);
			if (word[1] == 'P' || word[1] == 'p' || word[1] == 'E' || word[1] == 'e') {
				openplay(word + i);
				if (g_IsGameCycleEnd)
					norepf = TRUE;
			}
			if (word[1] == 'E' || word[1] == 'e') {
				ReleaseResources();
				if (g_IsGameCycleEnd)
					exit(0);
				exit(1);
			}
			if ((word[1] == 'O' || word[1] == 'o') && !norepf) {
				arg = 0;
				continue;
			}
			if (word[1] == 'S' || word[1] == 's') {
				speedmul = 0;
				while (word[i] != 0)
					speedmul = 10 * speedmul + word[i++] - '0';
				g_GameFrameTime = speedmul * 2000L;
				gs = TRUE;
			}
			if (word[1] == 'I' || word[1] == 'i')
				sscanf(word + i, "%u", &g_StartingLevel);
			if (word[1] == 'U' || word[1] == 'u')
				unlimlives = TRUE;

			if (word[1] == 'Q' || word[1] == 'q')
				soundflag = FALSE;
			if (word[1] == 'M' || word[1] == 'm')
				musicflag = FALSE;
			if (word[1] == '2')
				g_DiggersCount = 2;

			if (word[1] == 'G' || word[1] == 'g') {
				g_OptionGauntletTimeInSeconds = 0;
				while (word[i] != 0)
					g_OptionGauntletTimeInSeconds = 10 * g_OptionGauntletTimeInSeconds + word[i++] - '0';
				if (g_OptionGauntletTimeInSeconds > 3599)
					g_OptionGauntletTimeInSeconds = 3599;
				if (g_OptionGauntletTimeInSeconds == 0)
					g_OptionGauntletTimeInSeconds = 120;
				g_IsGauntletMode = TRUE;
			}
		}
		else {
			i = strlen(word);
			if (i < 1)
				continue;
			sf = TRUE;
			if (!gs)
				for (j = 0; j < i; j++)
					if (word[j]<'0' || word[j]>'9') {
						sf = FALSE;
						break;
					}
			if (sf) {
				speedmul = 0;
				j = 0;
				while (word[j] != 0)
					speedmul = 10 * speedmul + word[j++] - '0';
				gs = TRUE;
				g_GameFrameTime = speedmul * 2000L;
			}
			else {
				j = 0;
				while (word[j] != 0) {
					g_LevelFileName[j] = word[j];
					j++;
				}
				g_LevelFileName[j] = word[j];
				g_LevelFileFlag = TRUE;
			}
		}
	}

	if (g_LevelFileFlag) {
		levf = fopen(g_LevelFileName, "rb");
		if (levf == NULL) {
			strcat(g_LevelFileName, ".DLF");
			levf = fopen(g_LevelFileName, "rb");
		}
		if (levf == NULL)
			g_LevelFileFlag = FALSE;
		else {
			fread(&g_BonusScore, 2, 1, levf);
			fread(g_LevelsData, 1200, 1, levf);
			fclose(levf);
		}
	}
}

int g_RandomValue;

short randno(short n)
{
	g_RandomValue = g_RandomValue * 0x15a4e35l + 1;
	return (short)((g_RandomValue & 0x7fffffffl) % n);
}

char *keynames[] = { "Right","Up","Left","Down","Fire",
					"Right","Up","Left","Down","Fire",
					"Cheat","Accel","Brake","Music","Sound","Exit","Pause"
					,"SaveDRF","NumPlayers" };


void inir()
{
	char kbuf[80], vbuf[80];
	int i,/*j, unreferenced*/p;
	bool cgaflag;
#ifdef _WINDOWS
	int vidmode_w, vidmode_h, vidmode_bpp;
#endif

	for (i = 0; i < 19; i++) {
		sprintf(kbuf, "%s%c", keynames[i], (i >= 5 && i < 10) ? '2' : 0);
		sprintf(vbuf, "%i/%i/%i/%i/%i", g_KeyboardKeysSettings[i][0], g_KeyboardKeysSettings[i][1],
			g_KeyboardKeysSettings[i][2], g_KeyboardKeysSettings[i][3], g_KeyboardKeysSettings[i][4]);
		GetPrivateProfileString(INI_KEY_SETTINGS, kbuf, vbuf, vbuf, 80, g_DiggerIniPath);
		p = 0;
		while (vbuf[p]) {
			add_mapped_key(i, atoi(vbuf + p));
			while (vbuf[p] != '/' && vbuf[p] != 0)
				p++;
			if (vbuf[p] == 0)
				break;
			p++;
		}
	}

	g_OptionGauntletTimeInSeconds = (int)GetINIInt(INI_GAME_SETTINGS, "GauntletTime", 120, g_DiggerIniPath);
	g_GameFrameTime = GetINIInt(INI_GAME_SETTINGS, "Speed", 80000L, g_DiggerIniPath);
	g_IsGauntletMode = GetINIBool(INI_GAME_SETTINGS, "GauntletMode", FALSE, g_DiggerIniPath);
	GetPrivateProfileString(INI_GAME_SETTINGS, "Players", "1", vbuf, 80, g_DiggerIniPath);
	_strupr(vbuf);
	if (vbuf[0] == '2' && vbuf[1] == 'S') {
		g_DiggersCount = 2;
		g_PlayersCount = 1;
	}
	else {
		g_DiggersCount = 1;
		g_PlayersCount = atoi(vbuf);
		if (g_PlayersCount < 1 || g_PlayersCount>2)
			g_PlayersCount = 1;
	}

	sound_output_device = GetINIInt(INI_SOUND_SETTINGS, "WinSoundDevice", 1, g_DiggerIniPath);
	g_IsUsingDirectDraw = GetINIBool(INI_GRAPHICS_SETTINGS, "UseDirectDraw", TRUE, g_DiggerIniPath);
	vidmode_w = GetINIInt(INI_GRAPHICS_SETTINGS, "FullscreenVideoModeWidth", 640, g_DiggerIniPath);
	vidmode_h = GetINIInt(INI_GRAPHICS_SETTINGS, "FullscreenVideoModeHeight", 400, g_DiggerIniPath);
	vidmode_bpp = GetINIInt(INI_GRAPHICS_SETTINGS, "FullscreenVideoModeBPP", 8, g_DiggerIniPath);
	set_preferred_vidmode(vidmode_w, vidmode_h, vidmode_bpp);

	soundflag = GetINIBool(INI_SOUND_SETTINGS, "SoundOn", TRUE, g_DiggerIniPath);
	musicflag = GetINIBool(INI_SOUND_SETTINGS, "MusicOn", TRUE, g_DiggerIniPath);
	sound_device = (int)GetINIInt(INI_SOUND_SETTINGS, "Device", DEF_SND_DEV, g_DiggerIniPath);
	sound_port = (int)GetINIInt(INI_SOUND_SETTINGS, "Port", 544, g_DiggerIniPath);
	sound_irq = (int)GetINIInt(INI_SOUND_SETTINGS, "Irq", 5, g_DiggerIniPath);
	sound_dma = (int)GetINIInt(INI_SOUND_SETTINGS, "DMA", 1, g_DiggerIniPath);
	g_SoundRate = (int)GetINIInt(INI_SOUND_SETTINGS, "Rate", 22050, g_DiggerIniPath);
	sound_length = (int)GetINIInt(INI_SOUND_SETTINGS, "BufferSize", DEFAULT_BUFFER,
		g_DiggerIniPath);
	if (sound_device > 0)
	{
		g_Volume = 1;
		setupsound = s1setupsound;
		killsound = s1killsound;
		g_FillSoundBuffer = s1fillbuffer;
		initint8 = s1initint8;
		restoreint8 = s1restoreint8;
		if (sound_device == 1) {
			soundoff = s1soundoff;
			setspkrt2 = s1setspkrt2;
			settimer0 = s1settimer0;
			timer0 = s1timer0;
			settimer2 = s1settimer2;
			timer2 = s1timer2;
			getsample = getsample1;
		}
		else {
			soundoff = s2soundoff;
			setspkrt2 = s2setspkrt2;
			settimer0 = s2settimer0;
			timer0 = s2timer0;
			settimer2 = s2settimer2;
			timer2 = s2timer2;
			getsample = getsample2;
		}
		soundinitglob(sound_port, sound_irq, sound_dma, sound_length, g_SoundRate);
	}

	// initialize with default sound volume
	dx_sound_volume = (int)GetINIInt(INI_SOUND_SETTINGS, "SoundVolume", 85, g_DiggerIniPath);
	set_sound_volume(dx_sound_volume);

	g_bWindowed = !GetINIBool(INI_GRAPHICS_SETTINGS, "FullScreen", FALSE, g_DiggerIniPath);
	use_640x480_fullscreen = GetINIBool(INI_GRAPHICS_SETTINGS, "640x480", FALSE,
		g_DiggerIniPath);
	if (!g_bWindowed)
		ChangeCoopLevel();

	use_async_screen_updates = GetINIBool(INI_GRAPHICS_SETTINGS, "Async", TRUE,
		g_DiggerIniPath);


	cgaflag = GetINIBool(INI_GRAPHICS_SETTINGS, "CGA", FALSE, g_DiggerIniPath);
	biosflag = GetINIBool(INI_GRAPHICS_SETTINGS, "BIOSPalette", FALSE, g_DiggerIniPath);
	if (cgaflag || biosflag) {
		ginit = cgainit;
		gpal = cgapal;
		ginten = cgainten;
		ggetpix = cgagetpix;
		gputim = cgaputim;
		g_DrawSymbolFunc = CgaWrite;
		gtitle = cgatitle;
		ginit();
		gpal(0);
	}
	unlimlives = GetINIBool(INI_GAME_SETTINGS, "UnlimitedLives", FALSE, g_DiggerIniPath);
	g_StartingLevel = (int)GetINIInt(INI_GAME_SETTINGS, "StartLevel", 1, g_DiggerIniPath);
#ifdef _WINDOWS
	GetPrivateProfileString(INI_GAME_SETTINGS, "LevelFile", "", g_LevelFileName, STRBUF_512, g_DiggerIniPath);
	g_LevelFileFlag = (g_LevelFileName[0] != '\0'); /* TRUE; */
#endif
}

// redefine keyboard keys
void redefkeyb(bool allf)
{
	int i, j, k, l, z, y = 0;
	bool f;
	char kbuf[80], vbuf[80];
	if (g_DiggersCount == 2) {
		DrawGameText("PLAYER 1:", 0, y, 3);
		y += 12;
	}

	DrawGameText("PRESS NEW KEY FOR", 0, y, 3);
	y += 12;

	/* Step one: redefine keys that are always redefined. */

	for (i = 0; i < 5; i++) {
		DrawGameText(keynames[i], 0, y, 2); /* Red first */
		findkey(i);
		DrawGameText(keynames[i], 0, y, 1); /* Green once got */
		y += 12;
		for (j = 0; j < i; j++) { /* Note: only check keys just pressed (I hate it when
							   this is done wrong, and it often is.) */
			if (g_KeyboardKeysSettings[i][0] == g_KeyboardKeysSettings[j][0] && g_KeyboardKeysSettings[i][0] != 0) {
				i--;
				y -= 12;
				break;
			}
			for (k = 2; k < 5; k++)
				for (l = 2; l < 5; l++)
					if (g_KeyboardKeysSettings[i][k] == g_KeyboardKeysSettings[j][l] && g_KeyboardKeysSettings[i][k] != -2) {
						j = i;
						k = 5;
						i--;
						y -= 12;
						break; /* Try again if this key already used */
					}
		}
	}

	if (g_DiggersCount == 2) {
		DrawGameText("PLAYER 2:", 0, y, 3);
		y += 12;
		for (i = 5; i < 10; i++) {
			DrawGameText(keynames[i], 0, y, 2); /* Red first */
			findkey(i);
			DrawGameText(keynames[i], 0, y, 1); /* Green once got */
			y += 12;
			for (j = 0; j < i; j++) { /* Note: only check keys just pressed (I hate it when
								   this is done wrong, and it often is.) */
				if (g_KeyboardKeysSettings[i][0] == g_KeyboardKeysSettings[j][0] && g_KeyboardKeysSettings[i][0] != 0) {
					i--;
					y -= 12;
					break;
				}
				for (k = 2; k < 5; k++)
					for (l = 2; l < 5; l++)
						if (g_KeyboardKeysSettings[i][k] == g_KeyboardKeysSettings[j][l] && g_KeyboardKeysSettings[i][k] != -2) {
							j = i;
							k = 5;
							i--;
							y -= 12;
							break; /* Try again if this key already used */
						}
			}
		}
	}

	/* Step two: redefine other keys which step one has caused to conflict */

	z = 0;
	y -= 12;
	for (i = 10; i < 17; i++) {
		f = FALSE;
		for (j = 0; j < 10; j++)
			for (k = 0; k < 5; k++)
				for (l = 2; l < 5; l++)
					if (g_KeyboardKeysSettings[i][k] == g_KeyboardKeysSettings[j][l] && g_KeyboardKeysSettings[i][k] != -2)
						f = TRUE;
		for (j = 10; j < i; j++)
			for (k = 0; k < 5; k++)
				for (l = 0; l < 5; l++)
					if (g_KeyboardKeysSettings[i][k] == g_KeyboardKeysSettings[j][l] && g_KeyboardKeysSettings[i][k] != -2)
						f = TRUE;
		if (f || (allf && i != z)) {
			if (i != z)
				y += 12;
			DrawGameText(keynames[i], 0, y, 2); /* Red first */
			findkey(i);
			DrawGameText(keynames[i], 0, y, 1); /* Green once got */
			z = i;
			i--;
		}
	}

	/* Step three: save the INI file */

	for (i = 0; i < MAX_KEYBOARD_ACTIONS; i++)
	{
		if (g_IsKeyboardActionRedefined[i])
		{
			sprintf(kbuf, "%s%c", keynames[i], (i >= 5 && i < 10) ? '2' : 0);

			sprintf(vbuf, "%i/%i/%i/%i/%i", g_KeyboardKeysSettings[i][0], g_KeyboardKeysSettings[i][1], g_KeyboardKeysSettings[i][2], g_KeyboardKeysSettings[i][3], g_KeyboardKeysSettings[i][4]);

			WritePrivateProfileString(INI_KEY_SETTINGS, kbuf, vbuf, g_DiggerIniPath);
		}
	}
}
