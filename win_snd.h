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

#include <mmsystem.h>
#include <dsound.h>
extern LPDIRECTSOUND lpds;           /* address of direct sound object */
extern LPDIRECTSOUNDBUFFER lpdsb;    /* address of secondary direct sound buffer */

extern LONG dx_sound_volume;
extern bool g_IsWaveDeviceAvailable;
//extern int sound_card_capture_flag;

bool create_windows_sound_buffer(ushort samprate, ushort bufsize);
LONG get_sound_volume();
void set_sound_volume(LONG new_volume);
void change_sound_volume(bool louder);
void pause_windows_sound_playback();
void resume_windows_sound_playback();
DWORD get_sound_freq();
void unprepare_sound_data(WAVEHDR*);
void play_sound_data(WAVEHDR* pwhdr);
void destroy_sound_buffers();
void waveOut_fillbuffer(WAVEHDR* pwhdr);
void capture_sound_card();
void release_sound_card();

extern void change_sound_volume(bool louder);
extern LONG get_sound_volume();
extern void set_sound_volume(LONG);
extern DWORD get_sound_freq();
extern ushort g_SoundRate, sound_length;

extern HINSTANCE hDirectSoundInstance;
extern HRESULT(WINAPI *lpDirectSoundCreate)(LPCGUID, LPDIRECTSOUND *, LPUNKNOWN);
extern int sound_output_device;
