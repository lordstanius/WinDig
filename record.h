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

void openplay(char *name);
void InitializeRecordingBuffer();
void recname(char *name);
void playgetdir(short *dir, bool *fire);

/// <summary>
/// Starts recording of the game.
/// </summary>
void RecordingStart();

void RecordPutRandom(uint g_RandomValue);
uint playgetrand();
void recputinit(char *init);
void recputeol();
void recputeog();
void playskipeol();
void recputdir(short dir, bool fire);
void RecordSaveFileDialog();

extern bool g_IsRecordingPlayback, g_IsSaveGameRecordKeyPressed, g_IsCmdLineSaveFileNameAvailable, g_IsGameRecordAvailable, g_IsDrfValid, kludge;
