/*
-----------------------------------------------------------------------
<copyright file="Input.h" company="Petr Abdulin">
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

#define MAX_KEYBOARD_ACTIONS 19
#define MAX_ACTION_KEYS 5

#define ASCIIF8 VK_F8

#define rightpressed(x)  (key_pressed(0 + (5*(x)),TRUE))
#define uppressed(x)     (key_pressed(1 + (5*(x)),TRUE))
#define leftpressed(x)   (key_pressed(2 + (5*(x)),TRUE))
#define downpressed(x)   (key_pressed(3 + (5*(x)),TRUE))
#define f1pressed(x)     (key_pressed(4 + (5*(x)),FALSE))

/// <summary>
/// Clears the direction (static and dynamic) status.
/// </summary>
void ClearDirectionStatus();

/// <summary>
/// Clears the keyboard status.
/// </summary>
void ClearKeyboardStatus();

void readjoy();
void detectjoy();
bool CalibrateJoystick();
void ReadDiggerDirection(int n);
short GetDiggerDirection(int n);
void CheckKeyboard();

/// <summary>
/// Flushes the keyboard buffer.
/// </summary>
void FlushKeyboardBuffer();

void findkey(int kn);
void clearfire(int n);

extern bool firepflag[];
extern bool g_IsGameCycleEnd;
extern sbyte keypressed;
extern short akeypressed;

extern int g_KeyboardKeysSettings[][MAX_ACTION_KEYS];
extern bool g_IsKeyboardActionRedefined[];
extern bool g_IsPaused;
