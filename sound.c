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
#include "sound.h"
#include "hardware.h"
#include "Main.h"
#include "digger.h"
#include "input.h"
#include "win_dig.h"
#include "win_snd.h"

short wavetype = 0, musvol = 0;
short spkrmode = 0, timerrate = 0x7d0;
ushort timercount = 0, t2val = 0, t0val = 0;
short pulsewidth = 1;

short g_Volume = 0;

sbyte timerclock = 0;

bool soundflag = TRUE, musicflag = TRUE;

void soundint();
void soundlevdoneoff();
void soundlevdoneupdate();
void soundfallupdate();
void soundbreakoff();
void soundbreakupdate();
void soundwobbleupdate();
void soundfireupdate();
void soundexplodeoff(int n);
void soundexplodeupdate();
void soundbonusupdate();
void soundemoff();
void soundemupdate();
void soundemeraldoff();
void soundemeraldupdate();
void soundgoldoff();
void soundgoldupdate();
void soundeatmoff();
void soundeatmupdate();
void soundddieoff();
void soundddieupdate();
void sound1upoff();
void sound1upupdate();
void musicupdate();
void sett0();
void setsoundmode();
void s0setupsound();
void s0killsound();
void s0fillbuffer();

void(*setupsound)() = s0setupsound;
void(*killsound)() = s0killsound;

void(*g_FillSoundBuffer)() = s0fillbuffer;

void(*initint8)() = s0initint8;
void(*restoreint8)() = s0restoreint8;
void(*soundoff)() = s0soundoff;
void(*setspkrt2)() = s0setspkrt2;
void(*settimer0)(ushort t0v) = s0settimer0;
void(*timer0)(ushort t0v) = s0timer0;
void(*settimer2)(ushort t2v) = s0settimer2;
void(*timer2)(ushort t2v) = s0timer2;
void(*soundkillglob)() = s0soundkillglob;

bool sndflag = FALSE, soundpausedflag = FALSE;

int randvs;

short randnos(short n)
{
	randvs = randvs * 0x15a4e35l + 1;
	return (short)((randvs & 0x7fffffffl) % n);
}

void sett2val(short t2v)
{
	if (sndflag)
		timer2(t2v);
}

void soundint()
{
	timerclock++;
	if (soundflag && !sndflag)
		sndflag = musicflag = TRUE;
	if (!soundflag && sndflag) {
		sndflag = FALSE;
		timer2(40);
		setsoundt2();
		soundoff();
	}
	if (sndflag && !soundpausedflag) {
		t0val = 0x7d00;
		t2val = 40;
		if (musicflag)
			musicupdate();
#ifdef ARM
		else
			soundoff();
#endif
		soundemeraldupdate();
		soundwobbleupdate();
		soundddieupdate();
		soundbreakupdate();
		soundgoldupdate();
		soundemupdate();
		soundexplodeupdate();
		soundfireupdate();
		soundeatmupdate();
		soundfallupdate();
		sound1upupdate();
		soundbonusupdate();
		if (t0val == 0x7d00 || t2val != 40)
			setsoundt2();
		else {
			setsoundmode();
			sett0();
		}
		sett2val(t2val);
	}
}

void soundstop()
{
	int i;
	soundfalloff();
	soundwobbleoff();
	for (i = 0; i < FIREBALLS; i++)
		soundfireoff(i);
	musicoff();
	soundbonusoff();
	for (i = 0; i < FIREBALLS; i++)
		soundexplodeoff(i);
	soundbreakoff();
	soundemoff();
	soundemeraldoff();
	soundgoldoff();
	soundeatmoff();
	soundddieoff();
	sound1upoff();
}


bool soundlevdoneflag = FALSE;
short nljpointer = 0, nljnoteduration = 0;

void soundlevdone()
{
	short timer = 0;
	soundstop();
	nljpointer = 0;
	nljnoteduration = 20;
	soundlevdoneflag = soundpausedflag = TRUE;
	while (soundlevdoneflag && !g_IsGameCycleEnd) {
		g_FillSoundBuffer();
#ifdef _WINDOWS
		ProcessWindowsEvents();
		if (!g_IsWaveDeviceAvailable)
			soundlevdoneflag = FALSE;
#endif


		if (timerclock == timer)
			continue;


		soundlevdoneupdate();
		CheckKeyboard();
		timer = timerclock;
	}
	soundlevdoneoff();
}

void soundlevdoneoff()
{
	soundlevdoneflag = soundpausedflag = FALSE;
}

short newlevjingle[11] = { 0x8e8,0x712,0x5f2,0x7f0,0x6ac,0x54c,
						0x712,0x5f2,0x4b8,0x474,0x474 };

void soundlevdoneupdate()
{
	if (sndflag) {
		if (nljpointer < 11)
			t2val = newlevjingle[nljpointer];
		t0val = t2val + 35;
		musvol = 50;
		setsoundmode();
		sett0();
		sett2val(t2val);
		if (nljnoteduration > 0)
			nljnoteduration--;
		else {
			nljnoteduration = 20;
			nljpointer++;
			if (nljpointer > 10)
				soundlevdoneoff();
		}
	}
	else
		soundlevdoneflag = FALSE;
}


bool soundfallflag = FALSE, soundfallf = FALSE;
short soundfallvalue, soundfalln = 0;

void soundfall()
{
	soundfallvalue = 1000;
	soundfallflag = TRUE;
}

void soundfalloff()
{
	soundfallflag = FALSE;
	soundfalln = 0;
}

void soundfallupdate()
{
	if (soundfallflag)
		if (soundfalln < 1) {
			soundfalln++;
			if (soundfallf)
				t2val = soundfallvalue;
		}
		else {
			soundfalln = 0;
			if (soundfallf) {
				soundfallvalue += 50;
				soundfallf = FALSE;
			}
			else
				soundfallf = TRUE;
		}
}


bool soundbreakflag = FALSE;
short soundbreakduration = 0, soundbreakvalue = 0;

void soundbreak()
{
	soundbreakduration = 3;
	if (soundbreakvalue < 15000)
		soundbreakvalue = 15000;
	soundbreakflag = TRUE;
}

void soundbreakoff()
{
	soundbreakflag = FALSE;
}

void soundbreakupdate()
{
	if (soundbreakflag)
		if (soundbreakduration != 0) {
			soundbreakduration--;
			t2val = soundbreakvalue;
		}
		else
			soundbreakflag = FALSE;
}


bool soundwobbleflag = FALSE;
short soundwobblen = 0;

void soundwobble()
{
	soundwobbleflag = TRUE;
}

void soundwobbleoff()
{
	soundwobbleflag = FALSE;
	soundwobblen = 0;
}

void soundwobbleupdate()
{
	if (soundwobbleflag) {
		soundwobblen++;
		if (soundwobblen > 63)
			soundwobblen = 0;
		switch (soundwobblen) {
		case 0:
			t2val = 0x7d0;
			break;
		case 16:
		case 48:
			t2val = 0x9c4;
			break;
		case 32:
			t2val = 0xbb8;
			break;
		}
	}
}


bool soundfireflag[FIREBALLS] = { FALSE,FALSE }, sff[FIREBALLS];
short soundfirevalue[FIREBALLS], soundfiren[FIREBALLS] = { 0,0 };
int soundfirew = 0;

void soundfire(int n)
{
	soundfirevalue[n] = 500;
	soundfireflag[n] = TRUE;
}

void soundfireoff(int n)
{
	soundfireflag[n] = FALSE;
	soundfiren[n] = 0;
}

void soundfireupdate()
{
	int n;
	bool f = FALSE;
	for (n = 0; n < FIREBALLS; n++) {
		sff[n] = FALSE;
		if (soundfireflag[n])
			if (soundfiren[n] == 1) {
				soundfiren[n] = 0;
				soundfirevalue[n] += soundfirevalue[n] / 55;
				sff[n] = TRUE;
				f = TRUE;
				if (soundfirevalue[n] > 30000)
					soundfireoff(n);
			}
			else
				soundfiren[n]++;
	}
	if (f) {
		do {
			n = soundfirew++;
			if (soundfirew == FIREBALLS)
				soundfirew = 0;
		} while (!sff[n]);
		t2val = soundfirevalue[n] + randnos(soundfirevalue[n] >> 3);
	}
}


bool soundexplodeflag[FIREBALLS] = { FALSE,FALSE }, sef[FIREBALLS];
short soundexplodevalue[FIREBALLS], soundexplodeduration[FIREBALLS];
int soundexplodew = 0;

void soundexplode(int n)
{
	soundexplodevalue[n] = 1500;
	soundexplodeduration[n] = 10;
	soundexplodeflag[n] = TRUE;
	soundfireoff(n);
}

void soundexplodeoff(int n)
{
	soundexplodeflag[n] = FALSE;
}

void soundexplodeupdate()
{
	int n;
	bool f = FALSE;
	for (n = 0; n < FIREBALLS; n++) {
		sef[n] = FALSE;
		if (soundexplodeflag[n])
			if (soundexplodeduration[n] != 0) {
				soundexplodevalue[n] = soundexplodevalue[n] - (soundexplodevalue[n] >> 3);
				soundexplodeduration[n]--;
				sef[n] = TRUE;
				f = TRUE;
			}
			else
				soundexplodeflag[n] = FALSE;
	}
	if (f) {
		do {
			n = soundexplodew++;
			if (soundexplodew == FIREBALLS)
				soundexplodew = 0;
		} while (!sef[n]);
		t2val = soundexplodevalue[n];
	}
}


bool soundbonusflag = FALSE;
short soundbonusn = 0;

void soundbonus()
{
	soundbonusflag = TRUE;
}

void soundbonusoff()
{
	soundbonusflag = FALSE;
	soundbonusn = 0;
}

void soundbonusupdate()
{
	if (soundbonusflag) {
		soundbonusn++;
		if (soundbonusn > 15)
			soundbonusn = 0;
		if (soundbonusn >= 0 && soundbonusn < 6)
			t2val = 0x4ce;
		if (soundbonusn >= 8 && soundbonusn < 14)
			t2val = 0x5e9;
	}
}


bool soundemflag = FALSE;

void soundem()
{
	soundemflag = TRUE;
}

void soundemoff()
{
	soundemflag = FALSE;
}

void soundemupdate()
{
	if (soundemflag) {
		t2val = 1000;
		soundemoff();
	}
}


bool soundemeraldflag = FALSE;
short soundemeraldduration, emerfreq, soundemeraldn;

short emfreqs[8] = { 0x8e8,0x7f0,0x712,0x6ac,0x5f2,0x54c,0x4b8,0x474 };

void soundemerald(int n)
{
	emerfreq = emfreqs[n];
	soundemeraldduration = 7;
	soundemeraldn = 0;
	soundemeraldflag = TRUE;
}

void soundemeraldoff()
{
	soundemeraldflag = FALSE;
}

void soundemeraldupdate()
{
	if (soundemeraldflag)
		if (soundemeraldduration != 0) {
			if (soundemeraldn == 0 || soundemeraldn == 1)
				t2val = emerfreq;
			soundemeraldn++;
			if (soundemeraldn > 7) {
				soundemeraldn = 0;
				soundemeraldduration--;
			}
		}
		else
			soundemeraldoff();
}


bool soundgoldflag = FALSE, soundgoldf = FALSE;
short soundgoldvalue1, soundgoldvalue2, soundgoldduration;

void soundgold()
{
	soundgoldvalue1 = 500;
	soundgoldvalue2 = 4000;
	soundgoldduration = 30;
	soundgoldf = FALSE;
	soundgoldflag = TRUE;
}

void soundgoldoff()
{
	soundgoldflag = FALSE;
}

void soundgoldupdate()
{
	if (soundgoldflag) {
		if (soundgoldduration != 0)
			soundgoldduration--;
		else
			soundgoldflag = FALSE;
		if (soundgoldf) {
			soundgoldf = FALSE;
			t2val = soundgoldvalue1;
		}
		else {
			soundgoldf = TRUE;
			t2val = soundgoldvalue2;
		}
		soundgoldvalue1 += (soundgoldvalue1 >> 4);
		soundgoldvalue2 -= (soundgoldvalue2 >> 4);
	}
}



bool soundeatmflag = FALSE;
short soundeatmvalue, soundeatmduration, soundeatmn;

void soundeatm()
{
	soundeatmduration = 20;
	soundeatmn = 3;
	soundeatmvalue = 2000;
	soundeatmflag = TRUE;
}

void soundeatmoff()
{
	soundeatmflag = FALSE;
}

void soundeatmupdate()
{
	if (soundeatmflag)
		if (soundeatmn != 0) {
			if (soundeatmduration != 0) {
				if ((soundeatmduration % 4) == 1)
					t2val = soundeatmvalue;
				if ((soundeatmduration % 4) == 3)
					t2val = soundeatmvalue - (soundeatmvalue >> 4);
				soundeatmduration--;
				soundeatmvalue -= (soundeatmvalue >> 4);
			}
			else {
				soundeatmduration = 20;
				soundeatmn--;
				soundeatmvalue = 2000;
			}
		}
		else
			soundeatmflag = FALSE;
}


bool soundddieflag = FALSE;
short soundddien, soundddievalue;

void soundddie()
{
	soundddien = 0;
	soundddievalue = 20000;
	soundddieflag = TRUE;
}

void soundddieoff()
{
	soundddieflag = FALSE;
}

void soundddieupdate()
{
	if (soundddieflag) {
		soundddien++;
		if (soundddien == 1)
			musicoff();
		if (soundddien >= 1 && soundddien <= 10)
			soundddievalue = 20000 - soundddien * 1000;
		if (soundddien > 10)
			soundddievalue += 500;
		if (soundddievalue > 30000)
			soundddieoff();
		t2val = soundddievalue;
	}
}


bool sound1upflag = FALSE;
short sound1upduration = 0;

void sound1up()
{
	sound1upduration = 96;
	sound1upflag = TRUE;
}

void sound1upoff()
{
	sound1upflag = FALSE;
}

void sound1upupdate()
{
	if (sound1upflag) {
		if ((sound1upduration / 3) % 2 != 0)
			t2val = (sound1upduration << 2) + 600;
		sound1upduration--;
		if (sound1upduration < 1)
			sound1upflag = FALSE;
	}
}


bool musicplaying = FALSE;
short musicp = 0, tuneno = 0, noteduration = 0, notevalue = 0, musicmaxvol = 0,
musicattackrate = 0, musicsustainlevel = 0, musicdecayrate = 0, musicnotewidth = 0,
musicreleaserate = 0, musicstage = 0, musicn = 0;

void music(short tune)
{
	tuneno = tune;

	musicp = 0;

	noteduration = 0;

	switch (tune)
	{
	case BONUS_TUNE:
		musicmaxvol = 50;
		musicattackrate = 20;
		musicsustainlevel = 20;
		musicdecayrate = 10;
		musicreleaserate = 4;
		break;
	case NORMAL_TUNE:
		musicmaxvol = 50;
		musicattackrate = 50;
		musicsustainlevel = 8;
		musicdecayrate = 15;
		musicreleaserate = 1;
		break;
	case DEATH_TUNE:
		musicmaxvol = 50;
		musicattackrate = 50;
		musicsustainlevel = 25;
		musicdecayrate = 5;
		musicreleaserate = 1;
		break;
	}

	musicplaying = TRUE;

	if (tune == 2)
	{
		soundddieoff();
	}
}

void musicoff()
{
	musicplaying = FALSE;
	musicp = 0;
}

short bonusjingle[321] = {
  0x11d1,2,0x11d1,2,0x11d1,4,0x11d1,2,0x11d1,2,0x11d1,4,0x11d1,2,0x11d1,2,
   0xd59,4, 0xbe4,4, 0xa98,4,0x11d1,2,0x11d1,2,0x11d1,4,0x11d1,2,0x11d1,2,
  0x11d1,4, 0xd59,2, 0xa98,2, 0xbe4,4, 0xe24,4,0x11d1,4,0x11d1,2,0x11d1,2,
  0x11d1,4,0x11d1,2,0x11d1,2,0x11d1,4,0x11d1,2,0x11d1,2, 0xd59,4, 0xbe4,4,
   0xa98,4, 0xd59,2, 0xa98,2, 0x8e8,10,0xa00,2, 0xa98,2, 0xbe4,2, 0xd59,4,
   0xa98,4, 0xd59,4,0x11d1,2,0x11d1,2,0x11d1,4,0x11d1,2,0x11d1,2,0x11d1,4,
  0x11d1,2,0x11d1,2, 0xd59,4, 0xbe4,4, 0xa98,4,0x11d1,2,0x11d1,2,0x11d1,4,
  0x11d1,2,0x11d1,2,0x11d1,4, 0xd59,2, 0xa98,2, 0xbe4,4, 0xe24,4,0x11d1,4,
  0x11d1,2,0x11d1,2,0x11d1,4,0x11d1,2,0x11d1,2,0x11d1,4,0x11d1,2,0x11d1,2,
   0xd59,4, 0xbe4,4, 0xa98,4, 0xd59,2, 0xa98,2, 0x8e8,10,0xa00,2, 0xa98,2,
   0xbe4,2, 0xd59,4, 0xa98,4, 0xd59,4, 0xa98,2, 0xa98,2, 0xa98,4, 0xa98,2,
   0xa98,2, 0xa98,4, 0xa98,2, 0xa98,2, 0xa98,4, 0x7f0,4, 0xa98,4, 0x7f0,4,
   0xa98,4, 0x7f0,4, 0xa98,4, 0xbe4,4, 0xd59,4, 0xe24,4, 0xfdf,4, 0xa98,2,
   0xa98,2, 0xa98,4, 0xa98,2, 0xa98,2, 0xa98,4, 0xa98,2, 0xa98,2, 0xa98,4,
   0x7f0,4, 0xa98,4, 0x7f0,4, 0xa98,4, 0x7f0,4, 0x8e8,4, 0x970,4, 0x8e8,4,
   0x970,4, 0x8e8,4, 0xa98,2, 0xa98,2, 0xa98,4, 0xa98,2, 0xa98,2, 0xa98,4,
   0xa98,2, 0xa98,2, 0xa98,4, 0x7f0,4, 0xa98,4, 0x7f0,4, 0xa98,4, 0x7f0,4,
   0xa98,4, 0xbe4,4, 0xd59,4, 0xe24,4, 0xfdf,4, 0xa98,2, 0xa98,2, 0xa98,4,
   0xa98,2, 0xa98,2, 0xa98,4, 0xa98,2, 0xa98,2, 0xa98,4, 0x7f0,4, 0xa98,4,
   0x7f0,4, 0xa98,4, 0x7f0,4, 0x8e8,4, 0x970,4, 0x8e8,4, 0x970,4, 0x8e8,4,
  0x7d64 };

short backgjingle[291] = {
   0xfdf,2,0x11d1,2, 0xfdf,2,0x1530,2,0x1ab2,2,0x1530,2,0x1fbf,4, 0xfdf,2,
  0x11d1,2, 0xfdf,2,0x1530,2,0x1ab2,2,0x1530,2,0x1fbf,4, 0xfdf,2, 0xe24,2,
   0xd59,2, 0xe24,2, 0xd59,2, 0xfdf,2, 0xe24,2, 0xfdf,2, 0xe24,2,0x11d1,2,
   0xfdf,2,0x11d1,2, 0xfdf,2,0x1400,2, 0xfdf,4, 0xfdf,2,0x11d1,2, 0xfdf,2,
  0x1530,2,0x1ab2,2,0x1530,2,0x1fbf,4, 0xfdf,2,0x11d1,2, 0xfdf,2,0x1530,2,
  0x1ab2,2,0x1530,2,0x1fbf,4, 0xfdf,2, 0xe24,2, 0xd59,2, 0xe24,2, 0xd59,2,
   0xfdf,2, 0xe24,2, 0xfdf,2, 0xe24,2,0x11d1,2, 0xfdf,2,0x11d1,2, 0xfdf,2,
   0xe24,2, 0xd59,4, 0xa98,2, 0xbe4,2, 0xa98,2, 0xd59,2,0x11d1,2, 0xd59,2,
  0x1530,4, 0xa98,2, 0xbe4,2, 0xa98,2, 0xd59,2,0x11d1,2, 0xd59,2,0x1530,4,
   0xa98,2, 0x970,2, 0x8e8,2, 0x970,2, 0x8e8,2, 0xa98,2, 0x970,2, 0xa98,2,
   0x970,2, 0xbe4,2, 0xa98,2, 0xbe4,2, 0xa98,2, 0xd59,2, 0xa98,4, 0xa98,2,
   0xbe4,2, 0xa98,2, 0xd59,2,0x11d1,2, 0xd59,2,0x1530,4, 0xa98,2, 0xbe4,2,
   0xa98,2, 0xd59,2,0x11d1,2, 0xd59,2,0x1530,4, 0xa98,2, 0x970,2, 0x8e8,2,
   0x970,2, 0x8e8,2, 0xa98,2, 0x970,2, 0xa98,2, 0x970,2, 0xbe4,2, 0xa98,2,
   0xbe4,2, 0xa98,2, 0xd59,2, 0xa98,4, 0x7f0,2, 0x8e8,2, 0xa98,2, 0xd59,2,
  0x11d1,2, 0xd59,2,0x1530,4, 0xa98,2, 0xbe4,2, 0xa98,2, 0xd59,2,0x11d1,2,
   0xd59,2,0x1530,4, 0xa98,2, 0x970,2, 0x8e8,2, 0x970,2, 0x8e8,2, 0xa98,2,
   0x970,2, 0xa98,2, 0x970,2, 0xbe4,2, 0xa98,2, 0xbe4,2, 0xd59,2, 0xbe4,2,
   0xa98,4,0x7d64 };

short dirge[] = {
  0x7d00, 2,0x11d1, 6,0x11d1, 4,0x11d1, 2,0x11d1, 6, 0xefb, 4, 0xfdf, 2,
   0xfdf, 4,0x11d1, 2,0x11d1, 4,0x12e0, 2,0x11d1,12,0x7d00,16,0x7d00,16,
  0x7d00,16,0x7d00,16,0x7d00,16,0x7d00,16,0x7d00,16,0x7d00,16,0x7d00,16,
  0x7d00,16,0x7d00,16,0x7d00,16,0x7d64 };

void musicupdate()
{
	if (!musicplaying)
		return;
	if (noteduration != 0)
		noteduration--;
	else {
		musicstage = musicn = 0;
		switch (tuneno) {
		case 0:
			noteduration = bonusjingle[musicp + 1] * 3;
			musicnotewidth = noteduration - 3;
			notevalue = bonusjingle[musicp];
			musicp += 2;
			if (bonusjingle[musicp] == 0x7d64)
				musicp = 0;
			break;
		case 1:
			noteduration = backgjingle[musicp + 1] * 6;
			musicnotewidth = 12;
			notevalue = backgjingle[musicp];
			musicp += 2;
			if (backgjingle[musicp] == 0x7d64)
				musicp = 0;
			break;
		case 2:
			noteduration = dirge[musicp + 1] * 10;
			musicnotewidth = noteduration - 10;
			notevalue = dirge[musicp];
			musicp += 2;
			if (dirge[musicp] == 0x7d64)
				musicp = 0;
			break;
		}
	}
	musicn++;
	wavetype = 1;
	t0val = notevalue;
	if (musicn >= musicnotewidth)
		musicstage = 2;
	switch (musicstage) {
	case 0:
		if (musvol + musicattackrate >= musicmaxvol) {
			musicstage = 1;
			musvol = musicmaxvol;
			break;
		}
		musvol += musicattackrate;
		break;
	case 1:
		if (musvol - musicdecayrate <= musicsustainlevel) {
			musvol = musicsustainlevel;
			break;
		}
		musvol -= musicdecayrate;
		break;
	case 2:
		if (musvol - musicreleaserate <= 1) {
			musvol = 1;
			break;
		}
		musvol -= musicreleaserate;
	}
	if (musvol == 1)
		t0val = 0x7d00;
}


void soundpause()
{
	soundpausedflag = TRUE;
#ifdef _WINDOWS
	pause_windows_sound_playback();
#endif
}

void soundpauseoff()
{
	soundpausedflag = FALSE;
#ifdef _WINDOWS
	resume_windows_sound_playback();
#endif
}

void sett0()
{
	if (sndflag) {
		timer2(t2val);
		if (t0val < 1000 && (wavetype == 1 || wavetype == 2))
			t0val = 1000;
		timer0(t0val);
		timerrate = t0val;
		if (musvol < 1)
			musvol = 1;
		if (musvol > 50)
			musvol = 50;
		pulsewidth = musvol * g_Volume;
		setsoundmode();
	}
}

bool soundt0flag = FALSE;

void setsoundt2()
{
	if (soundt0flag) {
		spkrmode = 0;
		soundt0flag = FALSE;
		setspkrt2();
	}
}

void setsoundmode()
{
	spkrmode = wavetype;
	if (!soundt0flag && sndflag) {
		soundt0flag = TRUE;
		setspkrt2();
	}
}

bool int8flag = FALSE;

void startint8()
{
	if (!int8flag) {
		initint8();
		timerrate = 0x4000;
		settimer0(0x4000);
		int8flag = TRUE;
	}
}

void stopint8()
{
	settimer0(0);
	if (int8flag) {
		restoreint8();
		int8flag = FALSE;
	}
	sett2val(40);
	setspkrt2();
}

void initsound()
{
	settimer2(40);
	setspkrt2();
	settimer0(0);
	wavetype = 2;
	t0val = 12000;
	musvol = 8;
	t2val = 40;
	soundt0flag = TRUE;
	sndflag = TRUE;
	spkrmode = 0;
	int8flag = FALSE;
	setsoundt2();
	soundstop();
	setupsound();
	timerrate = 0x4000;
	settimer0(0x4000);
	randvs = GetHighResolutionTime();
}

void s0killsound()
{
	setsoundt2();
	timer2(40);
	stopint8();
}

void s0setupsound()
{
	inittimer();
	g_CurrentGameTime = 0;
	startint8();
}

void s0fillbuffer()
{
}
