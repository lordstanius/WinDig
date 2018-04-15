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

#include "def.h"
#include "Types.h"
#include <stdlib.h>
#include "newsnd.h"
#include "win_dig.h"
#include "win_snd.h"
#include "sound.h"

#define WAVE_OUT_DEVICE 2
#define DIRECT_SOUND_DEVICE 1

LPDIRECTSOUND lpds;                   /* the DirectSound object */
LPDIRECTSOUNDBUFFER lpdsb;            /* the DirectSoundBuffer that all sound is written to */
int sound_output_device = 0;            /* 0=sound disabled;1=directsound;2=standard windows waveOut*/

samp* sound_buffer = (samp*)NULL;

extern bool sndflag;
int sound_card_capture_flag = 0;
bool sound_buffer_playing = FALSE;      /* DirectSound only: has playback been started */
LONG dx_sound_volume;                 /* current volume (percentage from 0 to 100)   */
bool g_IsWaveDeviceAvailable;           /* is there a DirectSound device or any other wave device that can be used? */
HWAVEOUT wave_output_device = (HWAVEOUT)NULL;   /* handle to wave output device (when not using DirectSound) */

#define NUM_SOUND_BUFFERS 2

samp *  waveout_buffer[NUM_SOUND_BUFFERS] = { NULL, NULL }; /* Non DirectSound only */
WAVEHDR waveheader[NUM_SOUND_BUFFERS]; /* Non DirectSound only */
bool fill_sound_buffer = FALSE;
ushort waveout_sample_rate;             /* Non DirectSound only: just stores the waveout_sample_rate that was in the INI file */

/* DIRECTSOUND                                                                                */
/* If using DirectSound, a circular buffer is used similar to the DOS/Soundblaster version.   */
/* s1fillbuffer() operates more or less the same as the DOS version.                          */



/* NON-DIRECTSOUND                                                                            */
/* If using the standard Windows waveOut functions, things are a little strange...            */
/* There are two WAVEHDR structures, each with an asoociated sound buffer.  When the playback */
/* is first started, both of these buffers are 'prepared' (locked) and written out.  When the */
/* first buffer is finished g_IsRecordingPlayback, the second starts g_IsRecordingPlayback automatically, and the         */
/* waveOutProc function is called to notify the program that the first buffer is finished     */
/* g_IsRecordingPlayback.  In the waveOutProc function, the first header/buffer is 'unprepared', then       */
/* filled with new sound data (this is obtained from 'buffer' which is filled in              */
/* 's1fillbuffer'), 'prepared', and finally written out with 'waveOutWrite'. Then, when the   */
/* second buffer is finished being played, waveOutProc is again called, and the second buffer */
/* is unprepared, filled, prepared, and written out also.  Playback continues to alternate    */
/* between these two headers/buffers for the entire game.                                     */

/* Currently, s1fillbuffer() fills the 'buffer' variable, similar to the Soundblaster version,*/
/* except that it doesn't automatically wrap around to the beginning of 'buffer'.             */
/* Also, this function must call 'waveOutGetPosition' to try to control the rate at which     */
/* 'buffer' is filled (otherwise the sound is really messed up, especially the music at the   */
/* end of a level).                                                                           */

/* waveOut_fillbuffer() copies the sound data from 'buffer' to one of the waveheader/buffers  */
/* and resets the 'last' variable to indicate that s1fillbuffer should start filling 'buffer' */
/* up again with new sound data.                                                              */


void destroy_sound_buffers()
{
	// MMRESULT mmresult; unreferenced
	int i;
	for (i = 0; i < NUM_SOUND_BUFFERS; i++)
		if (waveout_buffer[i])
		{
			free(waveout_buffer[i]);
			waveout_buffer[i] = NULL;
		}
	if (sound_buffer)
	{
		free(sound_buffer);
		sound_buffer = NULL;
	}
}


void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	switch (uMsg)
	{
	case MM_WOM_DONE:
		unprepare_sound_data((WAVEHDR*)dwParam1);  /* unlock the sound buffer, so we can fill it with new data */
		if (!shutting_down)
		{
			play_sound_data((WAVEHDR*)dwParam1);
		}
		break;
	}
}

bool setsounddevice(int base, int irq, int dma, ushort samprate, ushort bufsize)
{
	HRESULT hr;
	samp* lpvAudioPtr1;
	DWORD dwAudioBytes1;
	samp* lpvAudioPtr2;
	DWORD dwAudioBytes2;

	bufsize <<= 1;
	size = bufsize;

	if (sound_output_device == DIRECT_SOUND_DEVICE && !lpDirectSoundCreate)
	{
		sound_output_device = WAVE_OUT_DEVICE;
	}

	switch (sound_output_device)
	{
	case DIRECT_SOUND_DEVICE:
		hr = DirectSoundCreate(NULL, &lpds, NULL);
		if (!hr == DS_OK)
		{
			g_IsWaveDeviceAvailable = FALSE;
			return FALSE;
		}
		g_IsWaveDeviceAvailable = TRUE;
		hr = IDirectSound_SetCooperativeLevel(lpds, g_hWnd, DSSCL_PRIORITY);
		if (!hr == DS_OK)
			TerminateOnFatalError(hr, "DirectSound_SetCooperativeLevel: FAILED");

		if (!create_windows_sound_buffer(samprate, bufsize))
			TerminateOnFatalError(0, "create_windows_sound_buffer FAILED");

		hr = IDirectSoundBuffer_Lock(lpdsb, 0, bufsize, (void **)&lpvAudioPtr1,
			&dwAudioBytes1, (void **)&lpvAudioPtr2,
			&dwAudioBytes2, (DWORD)NULL);
		switch (hr)
		{
		case DSERR_BUFFERLOST:
			TerminateOnFatalError(hr, "DirectSoundBuffer: lock FAILED  (BufferLost)");
		case DSERR_INVALIDCALL:
			TerminateOnFatalError(hr, "DirectSoundBuffer: lock FAILED  (InvalidCall)");
		case DSERR_INVALIDPARAM:
			TerminateOnFatalError(hr, "DirectSoundBuffer: lock FAILED  (InvalidParam)");
		case DSERR_PRIOLEVELNEEDED:
			TerminateOnFatalError(hr, "DirectSoundBuffer: lock FAILED  (PrioLevelNeeded)");
		}

		if (hr == DS_OK) {
			memset(lpvAudioPtr1, (MIN_SAMP + MAX_SAMP) >> 1, dwAudioBytes1);
			IDirectSoundBuffer_Unlock(lpdsb, lpvAudioPtr1, dwAudioBytes1, lpvAudioPtr2,
				dwAudioBytes2);
		}
		s1fillbuffer();
		sound_buffer_playing = TRUE;
		hr = IDirectSoundBuffer_Play(lpdsb, 0, 0, DSBPLAY_LOOPING);
		if (hr != DS_OK)
			TerminateOnFatalError(hr, "DirectSoundBuffer_Play FAILED");
		return TRUE;

	case WAVE_OUT_DEVICE:
		g_IsWaveDeviceAvailable = waveOutGetNumDevs();
		if (g_IsWaveDeviceAvailable)
			create_windows_sound_buffer(samprate, bufsize);
		waveout_sample_rate = samprate;
		return TRUE;
	case 0:  // disabled
		g_IsWaveDeviceAvailable = FALSE;
		return TRUE;
	}
	return FALSE;
}

bool initsounddevice()
{
	return TRUE;
}

void killsounddevice()
{
}

bool create_windows_sound_buffer(ushort samprate, ushort bufsize)
{
	PCMWAVEFORMAT pcmwf;
	int i;
	HRESULT hr;
	DSBUFFERDESC dsbdesc;
	MMRESULT mmresult;

	if (!g_IsWaveDeviceAvailable)
		return FALSE;
	memset(&pcmwf, 0, sizeof(PCMWAVEFORMAT));
	pcmwf.wf.wFormatTag = WAVE_FORMAT_PCM;
	pcmwf.wf.nChannels = 1;
	pcmwf.wf.nSamplesPerSec = samprate;
	pcmwf.wf.nBlockAlign = 1;
	pcmwf.wf.nAvgBytesPerSec = pcmwf.wf.nSamplesPerSec * pcmwf.wf.nBlockAlign;
	pcmwf.wBitsPerSample = 8;
	/* Using Directsound */
	if (sound_output_device == DIRECT_SOUND_DEVICE)
	{
		memset(&dsbdesc, 0, sizeof(DSBUFFERDESC)); // Zero it out.
		dsbdesc.dwSize = sizeof(DSBUFFERDESC);
		dsbdesc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY;
		dsbdesc.dwBufferBytes = bufsize;
		dsbdesc.lpwfxFormat = (LPWAVEFORMATEX)&pcmwf;
		hr = IDirectSound_CreateSoundBuffer(lpds, &dsbdesc, &lpdsb, NULL);
		if SUCCEEDED(hr)
		{
			return TRUE;
		}
		else
		{
			lpdsb = NULL;
			return FALSE;
		}
	}
	if (sound_output_device == WAVE_OUT_DEVICE)
	{
		/* Not using DirectSound */
		mmresult = waveOutOpen(&wave_output_device, WAVE_MAPPER, (LPWAVEFORMATEX)&pcmwf, (DWORD)waveOutProc, 0, CALLBACK_FUNCTION);
		if (mmresult != MMSYSERR_NOERROR)
		{
			g_IsWaveDeviceAvailable = FALSE;
			return FALSE;
		}
		else
		{
			sound_buffer = (samp*)malloc(bufsize * sizeof(samp));
			if (!sound_buffer)
				TerminateOnFatalError(0, "create_windows_sound_buffer: FAILED to allocate memory for 'sound_buffer'");
			for (i = 0; i < NUM_SOUND_BUFFERS; i++)
			{
				/* create sound buffers and WAVEHEADERs */
				waveout_buffer[i] = (samp*)malloc(bufsize * sizeof(samp));
				if (waveout_buffer[i] == NULL)
				{
					g_IsWaveDeviceAvailable = FALSE;
					return FALSE;
				}
				memset(waveout_buffer[i], (MIN_SAMP + MAX_SAMP) >> 1, bufsize * sizeof(samp));
				waveheader[i].lpData = waveout_buffer[i];
				waveheader[i].dwBufferLength = bufsize * sizeof(samp);
				waveheader[i].dwBytesRecorded = 0;
				waveheader[i].dwUser = 0;
				waveheader[i].dwFlags = 0;
				waveheader[i].dwLoops = 0;
				waveheader[i].lpNext = 0;
				waveheader[i].reserved = 0;
			}
			for (i = 0; i < NUM_SOUND_BUFFERS; i++)
			{
				/* start g_IsRecordingPlayback the wavedata */
				play_sound_data(&waveheader[i]);
			}
			return TRUE;
		}
	}
	return FALSE;
}

/* Not used with DirectSound */
void play_sound_data(WAVEHDR* pwhdr)
{
	MMRESULT mmresult;
	waveOut_fillbuffer(pwhdr);
	mmresult = waveOutPrepareHeader(wave_output_device, pwhdr, sizeof(WAVEHDR));
	if (mmresult != MMSYSERR_NOERROR)
		mmresult = 0;
	mmresult = waveOutWrite(wave_output_device, pwhdr, sizeof(WAVEHDR));
	if (mmresult != MMSYSERR_NOERROR)
		mmresult = 0;
}

/* Not used with DirectSound */
void unprepare_sound_data(WAVEHDR* pwhdr)
{
	MMRESULT mmresult;
	mmresult = waveOutUnprepareHeader(wave_output_device, pwhdr, sizeof(WAVEHDR));
	if (mmresult != MMSYSERR_NOERROR)
		mmresult = 0;
}


LONG get_sound_volume()
{
	return dx_sound_volume;
}

void set_sound_volume(LONG new_volume)
{
	LONG adjustedVolume;

	if (new_volume <= 100 && new_volume >= 0)
		dx_sound_volume = new_volume;
	else
		dx_sound_volume = 100;
	if (!g_IsWaveDeviceAvailable)
		return;
	if (sound_output_device == DIRECT_SOUND_DEVICE && lpdsb)
	{
		adjustedVolume = DSBVOLUME_MIN + (new_volume) * (DSBVOLUME_MAX - DSBVOLUME_MIN) / 100;
		IDirectSoundBuffer_SetVolume(lpdsb, adjustedVolume);
	}
}

void pause_windows_sound_playback()
{
	if (!g_IsWaveDeviceAvailable)
		return;
	if (sound_output_device == DIRECT_SOUND_DEVICE && lpdsb)
		IDirectSoundBuffer_Stop(lpdsb);
	if (sound_output_device == WAVE_OUT_DEVICE)
		waveOutPause(wave_output_device);

};

void resume_windows_sound_playback()
{
	if (!g_IsWaveDeviceAvailable)
		return;
	if (sound_output_device == DIRECT_SOUND_DEVICE && lpdsb)
		IDirectSoundBuffer_Play(lpdsb, 0, 0, DSBPLAY_LOOPING);
	if (sound_output_device == WAVE_OUT_DEVICE)
		waveOutRestart(wave_output_device);
};

DWORD get_sound_freq()
{
	DWORD sound_freq;

	if (!g_IsWaveDeviceAvailable)
		return waveout_sample_rate;
	if (sound_output_device == DIRECT_SOUND_DEVICE)
	{
		if (IDirectSoundBuffer_GetFrequency(lpdsb, &sound_freq) == DS_OK)
			return sound_freq;
		else
			return 0;
	}
	else
	{
		return waveout_sample_rate;
	}
}

void s0initint8() {}
void s0restoreint8() {}
void s0soundoff() {}
void s0setspkrt2() {}
void s0settimer0(ushort t0v) {}
void s0timer0(ushort t0v) {}
void s0settimer2(ushort t2v) {}
void s0timer2(ushort t2v) {}
void s0soundinitglob() {}
void s0soundkillglob() {}
void s1initint8() {}
void s1restoreint8() {}

void waveOut_fillbuffer(WAVEHDR* pwhdr)
{
	if (sound_output_device != WAVE_OUT_DEVICE || !sound_buffer)
		return;

	/* make sure that 'buffer' has been completely filled */
	fill_sound_buffer = TRUE;
	s1fillbuffer();

	/* copy the data in 'buffer' to the buffer associated with the waveheader */
	memcpy(pwhdr->lpData, sound_buffer, sizeof(samp)*size);

	fill_sound_buffer = FALSE;
	last = 0;
	firsts = 0;
}

void s1fillbuffer()
{
	MMTIME mmtime;
	MMRESULT mmresult;

	samp *lpvAudioPtr1;
	DWORD dwAudioBytes1;
	samp *lpvAudioPtr2;
	DWORD dwAudioBytes2;
	HRESULT hRet;
	DWORD i;
	DWORD play_position;
	DWORD write_position;
	int bytes_to_fill;
	/*
	  if (sound_card_capture_flag==1)
	  {
		sound_card_capture_flag=0;
		capture_sound_card();
	  }
	  else
	  {
		if (sound_card_capture_flag==2)
		{
		  sound_card_capture_flag=0;
		  release_sound_card();
		}
	  }
	*/
	if (!g_IsWaveDeviceAvailable)
	{
		/* Since there is no sound device available, there is no point in     */
		/* calling getsample() and filling a sound buffer, however, we still  */
		/* have to detect when sound is turned on or off...                   */
		/* This code is copied from soundint().                               */
	/*
		if (soundflag && !sndflag)
		{
		  sndflag=musicflag=TRUE;
		  sound_card_capture_flag=1;
		}
		if (!soundflag && sndflag) {
		  sndflag=FALSE;
		  timer2(40);
		  setsoundt2();
		  soundoff();
		  sound_card_capture_flag=2;
		}
	*/
		return;
	}

	if (sound_output_device == DIRECT_SOUND_DEVICE)
		/* Using DirectSound */
	{
		if (sound_buffer_playing)
		{
			hRet = IDirectSoundBuffer_GetCurrentPosition(lpdsb, &play_position, &write_position /*NULL */);
			if (hRet != DS_OK)
				TerminateOnFatalError(hRet, "IDirectSoundBuffer_GetCurrentPosition FAILED");
			firsts = (ushort)play_position;
		}
		else
			firsts = 0;
		if (last > firsts)
			bytes_to_fill = size - last + firsts;
		else
			bytes_to_fill = firsts - last;

		if (bytes_to_fill > 0) {
			hRet = IDirectSoundBuffer_Lock(lpdsb, last, bytes_to_fill,
				(void **)&lpvAudioPtr1, &dwAudioBytes1,
				(void **)&lpvAudioPtr2, &dwAudioBytes2,
				(DWORD)NULL);
			if (hRet == DS_OK) {
				for (i = 0; i < dwAudioBytes1; i++)
					lpvAudioPtr1[i] = getsample();
				for (i = 0; i < dwAudioBytes2; i++)
					lpvAudioPtr2[i] = getsample();
				IDirectSoundBuffer_Unlock(lpdsb, lpvAudioPtr1, dwAudioBytes1, lpvAudioPtr2,
					dwAudioBytes2);
			}
			else
				TerminateOnFatalError(hRet, "IDirectSoundBuffer_Lock FAILED");
			last = firsts;
		}
	}
	else
	{
		if (fill_sound_buffer)  /* Are we ready to write out another block of sound?       */
								/* If so, make sure the entire buffer is filled.           */
		{
			firsts = size - 1;
		}
		else                    /* Try to keep the calls to get sample() at a steady pace. */
		{
			mmtime.wType = TIME_BYTES;
			mmresult = waveOutGetPosition(wave_output_device, &mmtime, sizeof(MMTIME));
			if (mmresult != MMSYSERR_NOERROR)
				return;
			firsts = (ushort)mmtime.u.cb % size;
		}
		while (firsts >= last)
		{
			/* when not using DirectSound, the volume is applied when getsample() is called */
			  // TODO very strange calculations then not using DirectSound, possible otherflows ??
			sound_buffer[last] = (samp)(dx_sound_volume ? ((getsample() - 127) * (dx_sound_volume / 100)) + 127 : 127);
			last = last + 1;
		}
	}
}

extern bool soundlevdoneflag, soundpausedflag;
void capture_sound_card()
{
	soundinitglob(0, 0, 0, sound_length, g_SoundRate);
	set_sound_volume(dx_sound_volume);
	soundlevdoneflag = soundpausedflag = FALSE;
	//soundstop();
	//soundlevdone();
}

void release_sound_card()
{
	pause_windows_sound_playback();
	if (lpdsb)
	{
		IDirectSoundBuffer_Release(lpdsb);
		lpdsb = (LPDIRECTSOUNDBUFFER)NULL;
	}
	if (lpds)
	{
		IDirectSound_Release(lpds);
		lpds = (LPDIRECTSOUND)NULL;
	}
	if (wave_output_device)
	{
		waveOutReset(wave_output_device);
		waveOutClose(wave_output_device);
		wave_output_device = 0;
	}

	g_IsWaveDeviceAvailable = FALSE;
	destroy_sound_buffers();
}
