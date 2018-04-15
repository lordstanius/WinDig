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
#include "device.h"
#include "sound.h"
#include "newsnd.h"

extern int rate;
extern ushort t0v, t2v, t0rate, t2rate;
extern bool t2sw;

extern short spkrmode, pulsewidth;

int t0 = 0, t2 = 0;

samp getsample2();

/* This is the sound generation engine specific code */

void s2settimer2(ushort t2)
{
	if (t2 == 40)
		t2rate = 0;
	else
		if (t2 == 0)
			t2rate = rate;
		else
			t2rate = (ushort)((((uint)rate) << 16) / t2);
}

void s2timer2(ushort t2)
{
	s2settimer2(t2);
}

void s2soundoff()
{
	t2sw = FALSE;
}

void s2setspkrt2()
{
	t2sw = TRUE;
}

void s2settimer0(ushort t0)
{
	if (t0 == 0)
		t0rate = rate;
	else
		t0rate = (ushort)((((uint)rate) << 16) / t0);
}

void s2timer0(ushort t0)
{
	s2settimer0(t0);
}

samp getsample2()
{
	if (addcarry(&t0v, t0rate)) {
		if (addcarry(&timercount, timerrate)) {
			soundint();
			timercount -= 0x4000;
		}
	}
	t2v += t2rate;
	if (spkrmode != 0)
		if (t0v > pulsewidth * 655)
			t0 = -32767;
		else
			t0 = 32767;
	if (t2rate != 0 && t2sw) {
		if (t2v > 32767)
			t2 = -32767;
		else
			t2 = 32767;
	}
	return (samp)(((t0 + 2 * t2 + 98301L)*(MAX_SAMP - MIN_SAMP)) / 196605L) + MIN_SAMP;
}
