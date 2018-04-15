/*
-----------------------------------------------------------------------
<copyright file="Input.c" company="Petr Abdulin">
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
#include "Main.h"
#include "sound.h"
#include "hardware.h"
#include "record.h"
#include "digger.h"
#include "win_dig.h"
#include "Input.h"

bool oupressed[MAX_PLAYERS] = { 0 };
bool odpressed[MAX_PLAYERS] = { 0 };
bool olpressed[MAX_PLAYERS] = { 0 };
bool orpressed[MAX_PLAYERS] = { 0 };

bool start = FALSE;
bool g_IsGameCycleEnd = FALSE;

bool firepflag[MAX_PLAYERS] = { 0 };
bool aleftpressed[MAX_PLAYERS] = { 0 };
bool arightpressed[MAX_PLAYERS] = { 0 };
bool auppressed[MAX_PLAYERS] = { 0 };
bool adownpressed[MAX_PLAYERS] = { 0 };
bool af1pressed[MAX_PLAYERS] = { 0 };

short akeypressed;

short g_DynamicDirection[MAX_PLAYERS] = { DIR_NONE };
short g_StaticDirection[MAX_PLAYERS] = { DIR_NONE };

short joyx = 0, joyy = 0;

bool joybut1 = FALSE, joybut2 = FALSE;

short keydir[MAX_PLAYERS] = { 0 };

short jleftthresh = 0, jupthresh = 0, jrightthresh = 0,
jdownthresh = 0, joyanax = 0, joyanay = 0;

bool g_UseJoystick = FALSE;
bool g_IsPaused = FALSE;

bool g_IsKeyboardActionRedefined[MAX_KEYBOARD_ACTIONS] = { 0 };

int g_KeyboardKeysSettings[MAX_KEYBOARD_ACTIONS][MAX_ACTION_KEYS] =
{
	{VK_RIGHT, VK_NUMPAD6, 0, 0, 0}, /* 1 Right */
	{VK_UP, VK_NUMPAD8, 0, 0, 0}, /* 1 Up */
	{VK_LEFT, VK_NUMPAD4, 0, 0, 0}, /* 1 Left */
	{VK_DOWN, VK_NUMPAD2, 0, 0, }, /* 1 Down */
	{VK_F1, VK_RETURN, 0, 0, 0}, /* 1 Fire */
	{'D', 0, 0, 0, 0}, /* 2 Right */
	{'W', 0, 0, 0, 0}, /* 2 Up */
	{'A', 0, 0, 0, 0}, /* 2 Left */
	{'S', 0, 0, 0, 0}, /* 2 Down */
	{VK_TAB, 0, 0, 0, 0}, /* 2 Fire */
	{'T', 0, 0, 0, 0}, /* Cheat */
	{VK_ADD, 0, 0, 0, 0}, /* Accelerate */
	{VK_SUBTRACT, 0, 0, 0, 0}, /* Brake */
	{VK_F7, 0, 0, 0, 0}, /* Music */
	{VK_F9, 0, 0, 0, 0}, /* Sound */
	{VK_F10, 0, 0, 0, 0}, /* Exit */
	{VK_SPACE, 0, 0, 0, 0}, /* Pause */
	{VK_F8, 0, 0, 0, 0}, // Save recording
	{VK_ESCAPE, 'N', 0, 0, 0} // Number of players
};


int pki;

/* This function exclusively used in keyboard redefinition */
void findkey(int action)
{
	int i;
	int k = 0;
	//UnsignedWord scancode = 0;

	while (k == 0)// && (scancode == 0 || scancode & 0x80));
	{
		if (kbhit())
		{
			k = getkey();
		}

		ProcessWindowsEvents();
	}

	//if (kbhit())
	//{
	//    k = getkey();
	//}

	if (k == 0)
	{
		k = -2;
	}

	for (i = 0; i < 5; i++)
	{
		g_KeyboardKeysSettings[action][i] = k;
	}

	g_IsKeyboardActionRedefined[action] = 1;
}

/// <summary>
/// Check keyboard.
/// </summary>
/// <remarks>
/// The standard ASCII keyboard is also checked so that very short keypresses
/// are not overlooked. The functions kbhit() (returns bool denoting whether or
/// not there is a key in the buffer) and getkey() (wait until a key is in the
/// buffer, then return it) are used. These functions are emulated on platforms
/// which only provide an inkey() function (return the key in the buffer, unless
/// there is none, in which case return -1. It is done this way around for
/// historical reasons, there is no fundamental reason why it shouldn't be the
/// other way around.
/// </remarks>
void CheckKeyboard()
{
	int i, /*j, unreferenced*/ k = 0;

	for (i = 0; i < MAX_PLAYERS; i++)
	{
		if (leftpressed(i))
		{
			aleftpressed[i] = 1;
		}

		if (rightpressed(i))
		{
			arightpressed[i] = 1;
		}

		if (uppressed(i))
		{
			auppressed[i] = 1;
		}

		if (downpressed(i))
		{
			adownpressed[i] = 1;
		}

		if (f1pressed(i))
		{
			af1pressed[i] = 1;
		}
	}

	akeypressed = 0;

	if (kbhit())
	{
		akeypressed = getkey();

		command_buffer = translate_key_code_to_function(akeypressed);

		if (command_buffer != 19)
		{
			start = TRUE;
		}
	}

	if (command_buffer)
	{
		k = command_buffer - 1;
		if (command_buffer != 19)   // the check for this is currently done in GameRun(), so don't clear command_buffer if command_buffer==19
		{
			command_buffer = 0;
		}

		switch (k)
		{
		case 10: /* Cheat! */
			if (g_IsRecordingPlayback) {      /* Why was this "!g_IsGauntletMode"? */
				g_IsRecordingPlayback = FALSE;
				g_IsDrfValid = FALSE;
			}
			break;
		case 11: /* Increase speed */
			if (g_GameFrameTime > 10000L)
				g_GameFrameTime -= 10000L;
			break;
		case 12: /* Decrease speed */
			g_GameFrameTime += 10000L;
			break;
		case 13: /* Toggle music */
			musicflag = !musicflag;
			break;
		case 14: /* Toggle sound */
			soundflag = !soundflag;
			break;
		case 15: /* Exit */
			g_IsGameCycleEnd = TRUE;
			break;
		case 16: /* Pause */
			g_IsPaused = TRUE;
			break;
		case 17:
			g_IsSaveGameRecordKeyPressed = TRUE;
			break;
		}
	}
}

/* Joystick not yet implemented. It will be, though, using GetHighResolutionTime on platform
   DOSPC. */
void readjoy()
{

	JOYINFO ji;
	if (joyGetPos(0, &ji) == JOYERR_NOERROR)
	{
		joyx = (short)(ji.wXpos / 655);
		joyy = (short)(ji.wYpos / 655);
		joybut1 = ji.wButtons & JOY_BUTTON1;
		joybut2 = ji.wButtons & JOY_BUTTON2;
	}

}

void detectjoy()
{
	init_joystick();

	ClearDirectionStatus();
}

/// <summary>
/// Clears the direction (static and dynamic) status.
/// </summary>
void ClearDirectionStatus()
{
	int i;

	for (i = 0; i < MAX_PLAYERS; i++)
	{
		g_StaticDirection[i] = DIR_NONE;
		g_DynamicDirection[i] = DIR_NONE;
	}
}

/// <summary>
/// Clears the keyboard status.
/// </summary>
void ClearKeyboardStatus()
{
	int i;

	for (i = 0; i < MAX_PLAYERS; i++)
	{
		aleftpressed[i] = 0;
		arightpressed[i] = 0;
		auppressed[i] = 0;
		adownpressed[i] = 0;
		af1pressed[i] = 0;
	}
}

/// <summary>
/// Flushes the keyboard buffer.
/// </summary>
/// <remarks>
/// Contrary to some beliefs, you don't need a separate OS call to flush the keyboard buffer.
/// </remarks>
void FlushKeyboardBuffer()
{
	// swallow all
	while (kbhit())
	{
		getkey();
	}

	// clear values
	ClearKeyboardStatus();
}

void clearfire(int n)
{
	af1pressed[n] = FALSE;
}

void ReadDiggerDirection(int n)
{
	// short j; unreferenced
	bool u[MAX_PLAYERS] = { FALSE };
	bool d[MAX_PLAYERS] = { FALSE };
	bool l[MAX_PLAYERS] = { FALSE };
	bool r[MAX_PLAYERS] = { FALSE };

	if (auppressed[n] || uppressed(n)) { u[n] = TRUE; auppressed[n] = FALSE; }
	if (adownpressed[n] || downpressed(n)) { d[n] = TRUE; adownpressed[n] = FALSE; }
	if (aleftpressed[n] || leftpressed(n)) { l[n] = TRUE; aleftpressed[n] = FALSE; }
	if (arightpressed[n] || rightpressed(n)) { r[n] = TRUE; arightpressed[n] = FALSE; }
	if (af1pressed[n] || f1pressed(n)) { firepflag[n] = TRUE; af1pressed[n] = FALSE; }
	else
	{
		firepflag[n] = FALSE;
	}

	if (u[n] && !oupressed[n])
		g_StaticDirection[n] = g_DynamicDirection[n] = DIR_UP;
	if (d[n] && !odpressed[n])
		g_StaticDirection[n] = g_DynamicDirection[n] = DIR_DOWN;
	if (l[n] && !olpressed[n])
		g_StaticDirection[n] = g_DynamicDirection[n] = DIR_LEFT;
	if (r[n] && !orpressed[n])
		g_StaticDirection[n] = g_DynamicDirection[n] = DIR_RIGHT;
	if ((oupressed[n] && !u[n] && g_DynamicDirection[n] == DIR_UP) ||
		(odpressed[n] && !d[n] && g_DynamicDirection[n] == DIR_DOWN) ||
		(olpressed[n] && !l[n] && g_DynamicDirection[n] == DIR_LEFT) ||
		(orpressed[n] && !r[n] && g_DynamicDirection[n] == DIR_RIGHT))
	{
		g_DynamicDirection[n] = DIR_NONE;
		if (u[n]) g_DynamicDirection[n] = g_StaticDirection[n] = DIR_UP;
		if (d[n]) g_DynamicDirection[n] = g_StaticDirection[n] = DIR_DOWN;
		if (l[n]) g_DynamicDirection[n] = g_StaticDirection[n] = DIR_LEFT;
		if (r[n]) g_DynamicDirection[n] = g_StaticDirection[n] = DIR_RIGHT;
	}

	oupressed[n] = u[n];
	odpressed[n] = d[n];
	olpressed[n] = l[n];
	orpressed[n] = r[n];

	keydir[n] = g_StaticDirection[n];
	if (g_DynamicDirection[n] != DIR_NONE)
	{
		keydir[n] = g_DynamicDirection[n];
	}

	g_StaticDirection[n] = DIR_NONE;

	readjoy();
	if (joybut1)
		firepflag[0] = TRUE;

}

/// <summary>
/// Calibrate joystick while waiting at title screen. 
/// </summary>
/// <remarks>
/// This works more effectively if the user waggles the joystick in the title screen.
/// </remarks>
bool CalibrateJoystick()
{
	short j;
	bool startf = FALSE;

	if (g_UseJoystick)
	{
		readjoy();

		if (joybut1)
		{
			startf = TRUE;
		}
	}

	if (start)
	{
		start = FALSE;
		startf = TRUE;
	}

	if (!startf)
	{
		return FALSE;
	}

	if (g_UseJoystick)
	{
		joyanay = 0;
		joyanax = 0;

		for (j = 0; j < 50; j++)
		{
			readjoy();
			joyanax += joyx;
			joyanay += joyy;
		}

		joyx = joyanax / 50;
		joyy = joyanay / 50;
		jleftthresh = joyx - 35;

		if (jleftthresh < 0)
		{
			jleftthresh = 0;
		}

		jleftthresh += 10;

		jupthresh = joyy - 35;

		if (jupthresh < 0)
		{
			jupthresh = 0;
		}

		jupthresh += 10;
		jrightthresh = joyx + 35;

		if (jrightthresh > 255)
		{
			jrightthresh = 255;
		}

		jrightthresh -= 10;
		jdownthresh = joyy + 35;

		if (jdownthresh > 255)
		{
			jdownthresh = 255;
		}

		jdownthresh -= 10;
		joyanax = joyx;
		joyanay = joyy;
	}

	return TRUE;
}

/// <summary>
/// Gets a digger direction.
/// </summary>
/// <param name="n">The.</param>
/// <returns>
/// The digger direction.
/// </returns>
/// <remarks>
/// Why the joystick reading is split between ReadDiggerDirection and 
/// GetDiggerDirection like this is a mystery to me.
/// </remarks>
short GetDiggerDirection(int n)
{
	// int i; unreferenced

	short dir = keydir[n];

	if (g_UseJoystick)
	{
		if (abs(joyx - 50) > abs(joyy - 50))
		{
			if (joyx < jleftthresh)
			{
				dir = DIR_LEFT;
			}

			if (joyx > jrightthresh)
			{
				dir = DIR_RIGHT;
			}
		}
		else
		{
			if (joyy < jupthresh)
			{
				dir = DIR_UP;
			}
			if (joyy > jdownthresh)
			{
				dir = DIR_DOWN;
			}
		}
	}

	// if recordgin is g_IsRecordingPlayback
	if (g_IsRecordingPlayback)
	{
		playgetdir(&dir, &firepflag[n]);
	}

	// put direction into recording
	recputdir(dir, firepflag[n]);

	return dir;
}
