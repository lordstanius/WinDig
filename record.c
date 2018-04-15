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

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "def.h"
#include "Types.h"
#include "record.h"
#include "hardware.h"
#include "sound.h"
#include "input.h"
#include "Main.h"
#include "Scores.h"
#include "Sprite.h"
#include "win_dig.h"


char  *g_RecordingBuffer, *plb, *plp;

/// <summary> 
/// True if game is in recording playback mode.
/// </summary>
bool g_IsRecordingPlayback = FALSE;

/// <summary>TRUE if is save game record key (F8) was pressed.</summary>
bool g_IsSaveGameRecordKeyPressed = FALSE;

/// <summary>TRUE if is name file DRF file was supplied via command line argument.</summary>
bool g_IsCmdLineSaveFileNameAvailable = FALSE;

/// <summary>TRUE if game record available, so we can save it.</summary>
bool g_IsGameRecordAvailable = FALSE;

bool g_IsDrfValid = TRUE;
bool kludge = FALSE;

char rname[128];

int reccc = 0;
int recrl = 0;
int rlleft = 0;

uint g_RecordingPointer = 0;
char recd, rld;

void mprintf(char *f, ...);
void makedir(short *dir, bool *fire, char d);
char maked(short dir, bool fire);

// digger record file
#define DEFAULTSN "DIGGER.DRF"

void openplay(char *name)
{
	FILE *playf = fopen(name, "rt");
	int l, i;
	char c, buf[80];
	int x, y, n, origgtime = g_OptionGauntletTimeInSeconds;
	bool origg = g_IsGauntletMode;
	short origstartlev = g_StartingLevel, orignplayers = g_PlayersCount, origdiggers = g_DiggersCount;

	if (playf == NULL) {
		g_IsGameCycleEnd = TRUE;
		return;
	}
	rlleft = 0;
	g_IsGauntletMode = FALSE;
	g_StartingLevel = 1;
	g_PlayersCount = 1;
	g_DiggersCount = 1;
	/* The file is in two distint parts. In the first, line breaks are used as
	   separators. In the second, they are ignored. This is the first. */

	fgets(buf, 80, playf); /* Get id string */
	if (buf[0] != 'D' || buf[1] != 'R' || buf[2] != 'F') {
		fclose(playf);
		g_IsGameCycleEnd = TRUE;
		return;
	}
	fgets(buf, 80, playf); /* Get version for kludge switches */
	if (atol(buf + 7) <= 19981125L)
		kludge = TRUE;
	fgets(buf, 80, playf); /* Get mode */
	if (*buf == '1') {
		g_PlayersCount = 1;
		x = 1;
	}
	else
		if (*buf == '2') {
			g_PlayersCount = 2;
			x = 1;
		}
		else {
			if (*buf == 'M') {
				g_DiggersCount = buf[1] - '0';
				x = 2;
			}
			else
				x = 0;
			if (buf[x] == 'G') {
				g_IsGauntletMode = TRUE;
				x++;
				g_OptionGauntletTimeInSeconds = atoi(buf + x);
				while (buf[x] >= '0' && buf[x] <= '9')
					x++;
			}
		}
		if (buf[x] == 'U') /* Unlimited lives are ignored on playback. */
			x++;
		if (buf[x] == 'I')
			g_StartingLevel = atoi(buf + x + 1);
		fgets(buf, 80, playf); /* Get bonus score */
		g_BonusScore = atoi(buf);
		for (n = 0; n < 8; n++)
			for (y = 0; y < 10; y++) {
				for (x = 0; x < 15; x++)
					buf[x] = ' ';
				fgets(buf, 80, playf); /* Get a line of map */
				for (x = 0; x < 15; x++)
					g_LevelsData[n][y][x] = buf[x];
			}

		/* This is the second. The line breaks here really are only so that the file
		   can be emailed. */

		i = ftell(playf);
		fseek(playf, 0, SEEK_END);
		l = ftell(playf) - i;
		fseek(playf, i, SEEK_SET);
		plb = plp = (char  *)malloc(l);
		if (plb == (char  *)NULL) {
			fclose(playf);
			g_IsGameCycleEnd = TRUE;
			return;
		}

		for (i = 0; i < l; i++) {
			c = fgetc(playf); /* Get everything that isn't line break into 1 string */
			if (c >= ' ')
				*(plp++) = c;
		}
		fclose(playf);
		plp = plb;

		g_IsRecordingPlayback = TRUE;
		RecordingStart();
		GameCycle();
		g_IsGameRecordAvailable = TRUE;
		g_IsRecordingPlayback = FALSE;
		free(plb);
		g_IsGauntletMode = origg;
		g_OptionGauntletTimeInSeconds = origgtime;
		kludge = FALSE;
		g_StartingLevel = origstartlev;
		g_DiggersCount = origdiggers;
		g_PlayersCount = orignplayers;
}

void InitializeRecordingBuffer()
{
	uint s = MAX_REC_BUFFER;

	// try to allocate as much as possible
	//do 
	//{
	g_RecordingBuffer = (char  *)malloc(s);

	//    if (g_RecordingBuffer==NULL)
	//    {
	//        s>>=1;
	//    }

	//} while (g_RecordingBuffer==(char  *)NULL && s>1024);

	// if unable to allocate
	if (g_RecordingBuffer == NULL)
	{
		ReleaseResources();

		MessageBox(g_hWnd, "Cannot allocate memory for recording buffer.", "Error", MB_OK);

		exit(1);
	}

	g_RecordingPointer = 0;
}

void mprintf(char *f, ...)
{
	va_list ap;
	char buf[80];
	int i, l;
	va_start(ap, f);
	vsprintf(buf, f, ap);
	l = strlen(buf);
	for (i = 0; i < l; i++)
		g_RecordingBuffer[g_RecordingPointer + i] = buf[i];
	g_RecordingPointer += l;
	if (g_RecordingPointer > MAX_REC_BUFFER - 80)
		g_RecordingPointer = 0;          /* Give up, file is too long */
}

void makedir(short *dir, bool *fire, char d)
{
	if (d >= 'A' && d <= 'Z') {
		*fire = TRUE;
		d -= 'A' - 'a';
	}
	else
		*fire = FALSE;
	switch (d) {
	case 's': *dir = DIR_NONE; break;
	case 'r': *dir = DIR_RIGHT; break;
	case 'u': *dir = DIR_UP; break;
	case 'l': *dir = DIR_LEFT; break;
	case 'd': *dir = DIR_DOWN; break;
	}
}

void playgetdir(short *dir, bool *fire)
{
	if (rlleft > 0) {
		makedir(dir, fire, rld);
		rlleft--;
	}
	else {
		if (*plp == 'E' || *plp == 'e') {
			g_IsGameCycleEnd = TRUE;
			return;
		}
		rld = *(plp++);
		while (*plp >= '0' && *plp <= '9')
			rlleft = rlleft * 10 + ((*(plp++)) - '0');
		makedir(dir, fire, rld);
		if (rlleft > 0)
			rlleft--;
	}
}

char maked(short dir, bool fire)
{
	char d;
	if (dir == DIR_NONE)
		d = 's';
	else
		d = "ruld"[dir >> 1];
	if (fire)
		d += 'A' - 'a';
	return d;
}

void putrun()
{
	if (recrl > 1)
		mprintf("%c%i", recd, recrl);
	else
		mprintf("%c", recd);
	reccc++;
	if (recrl > 1) {
		reccc++;
		if (recrl >= 10) {
			reccc++;
			if (recrl >= 100)
				reccc++;
		}
	}
	if (reccc >= 60) {
		mprintf("\n");
		reccc = 0;
	}
}

void recputdir(short dir, bool fire)
{
	char d = maked(dir, fire);
	if (recrl == 0)
		recd = d;
	if (recd != d) {
		putrun();
		recd = d;
		recrl = 1;
	}
	else {
		if (recrl == 999) {
			putrun(); /* This probably won't ever happen. */
			recrl = 0;
		}
		recrl++;
	}
}

/// <summary>
/// Starts recording of the game.
/// </summary>
void RecordingStart()
{
	int x, y, l;

	g_RecordingPointer = 0;

	g_IsDrfValid = TRUE;

	mprintf("DRF\n"); /* Required at start of DRF */
	if (kludge)
		mprintf("AJ DOS 19981125\n");
	else
		mprintf(DIGGER_VERSION"\n");
	if (g_DiggersCount > 1) {
		mprintf("M%i", g_DiggersCount);
		if (g_IsGauntletMode)
			mprintf("G%i", g_OptionGauntletTimeInSeconds);
	}
	else
		if (g_IsGauntletMode)
			mprintf("G%i", g_OptionGauntletTimeInSeconds);
		else
			mprintf("%i", g_PlayersCount);
	/*  if (unlimlives)
	mprintf("U"); */
	if (g_StartingLevel > 1)
		mprintf("I%i", g_StartingLevel);
	mprintf("\n%i\n", g_BonusScore);
	for (l = 0; l < 8; l++) {
		for (y = 0; y < MHEIGHT; y++) {
			for (x = 0; x < MWIDTH; x++)
				mprintf("%c", g_LevelsData[l][y][x]);
			mprintf("\n");
		}
	}
	reccc = recrl = 0;
}

// save random number to record
void RecordPutRandom(uint randomValue)
{
	mprintf("%08lX\n", randomValue);

	reccc = recrl = 0;
}

/* This is a mess. Sorry. */

void RecordSaveFileDialog()
{
	FILE *recf;
	uint i;
	int j;
	bool gotfile = TRUE;
	char nambuf[STRBUF_512], init[4];
	if (!g_IsDrfValid)
		return;
	if (g_IsCmdLineSaveFileNameAvailable)
		strcpy(nambuf, rname);

	if (!g_IsCmdLineSaveFileNameAvailable) {
		if (g_PlayersCount == 2)
			strcpy(nambuf, DEFAULTSN);

		else {
			for (j = 0; j < 3; j++) {
				init[j] = g_ScoreInitials[0][j];
				if (!((init[j] >= 'A' && init[j] <= 'Z') ||
					(init[j] >= 'a' && init[j] <= 'z')))
					init[j] = '_';
			}
			init[3] = 0;
			if (scoret < 100000L)
				sprintf(nambuf, "%s%li", init, scoret);
			else
				if (init[2] == '_')
					sprintf(nambuf, "%c%c%li", init[0], init[1], scoret);
				else
					if (init[0] == '_')
						sprintf(nambuf, "%c%c%li", init[1], init[2], scoret);
					else
						sprintf(nambuf, "%c%c%li", init[0], init[2], scoret);
			strcat(nambuf, ".drf");
		}
		if (!get_open_save_filename(SAVE, "Save Recorded Game", "Digger Record Files (*.drf)\0*.DRF\0All Files (*.*)\0*.*\0", "DRF", nambuf))
			return;
		recf = fopen(nambuf, "wt");

		if (recf == NULL)
			gotfile = FALSE;
		else
			gotfile = TRUE;
	}
	if (!gotfile)
		return;
	for (i = 0; i < g_RecordingPointer; i++)
		fputc(g_RecordingBuffer[i], recf);
	fclose(recf);
	g_IsGameRecordAvailable = FALSE;
	strcpy(drf_filename, nambuf);

}

void playskipeol()
{
	plp += 3;
}

uint playgetrand()
{
	int i;
	uint r = 0;
	char p;
	if ((*plp) == '*')
		plp += 4;
	for (i = 0; i < 8; i++) {
		p = *(plp++);
		if (p >= '0' && p <= '9')
			r |= (uint)(p - '0') << ((7 - i) << 2);
		if (p >= 'A' && p <= 'F')
			r |= (uint)(p - 'A' + 10) << ((7 - i) << 2);
		if (p >= 'a' && p <= 'f')
			r |= (uint)(p - 'a' + 10) << ((7 - i) << 2);
	}
	return r;
}

void recputinit(char *init)
{
	mprintf("*%c%c%c\n", init[0], init[1], init[2]);
}

void recputeol()
{
	if (recrl > 0)
		putrun();
	if (reccc > 0)
		mprintf("\n");
	mprintf("EOL\n");
}

void recputeog()
{
	mprintf("EOG\n");
}

void recname(char *name)
{
	g_IsCmdLineSaveFileNameAvailable = TRUE;
	strcpy(rname, name);
}
