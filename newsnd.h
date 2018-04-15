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

#include "device.h"

void soundinitglob(int port, int irq, int dma, ushort bufsize, ushort samprate);
void s1setupsound();
void s1killsound();
void s1fillbuffer();
void s1settimer2(ushort t2);
void s1soundoff();
void s1setspkrt2();
void s1settimer0(ushort t0);
void s1timer0(ushort t0);
void s1timer2(ushort t2);

extern samp(*getsample)();
samp getsample1();

void s2settimer2(ushort t2);
void s2soundoff();
void s2setspkrt2();
void s2settimer0(ushort t0);
void s2timer0(ushort t0);
void s2timer2(ushort t2);

samp getsample2();

void inits2();
void finishs2();

bool addcarry(ushort *dest, ushort add);
