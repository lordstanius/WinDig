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

/// <summary>
/// The game music tunes.
/// </summary>
#define BONUS_TUNE 0
#define NORMAL_TUNE 1
#define DEATH_TUNE 2

void initsound();
void soundstop();
void music(short tune);
void musicoff();
void soundlevdone();
void sound1up();
void soundpause();
void soundpauseoff();
void setsoundt2();
void sett2val(short t2v);
void startint8();
void stopint8();
void soundbonus();
void soundbonusoff();
void soundfire(int n);
void soundexplode(int n);
void soundfireoff(int n);
void soundem();
void soundemerald(int emn);
void soundeatm();
void soundddie();
void soundwobble();
void soundwobbleoff();
void soundfall();
void soundfalloff();
void soundbreak();
void soundgold();

void soundint();

/*
void soundoff();
void timer2(UnsignedWord t2v);
*/

extern bool soundflag, musicflag;
extern short g_Volume, timerrate;
extern ushort timercount;

extern void(*setupsound)();
extern void(*killsound)();
extern void(*g_FillSoundBuffer)();
extern void(*initint8)();
extern void(*restoreint8)();
extern void(*soundoff)();
extern void(*setspkrt2)();
extern void(*settimer0)(ushort t0v);
extern void(*timer0)(ushort t0v);
extern void(*settimer2)(ushort t2v);
extern void(*timer2)(ushort t2v);
extern void(*soundkillglob)();

