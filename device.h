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

/* Generic sound device header */

typedef unsigned char samp;      /* 8 bit unsigned samples */
#define MIN_SAMP 0x0
#define MAX_SAMP 0xff

extern samp *buffer;
extern ushort firsts, last, size;

bool setsounddevice(int base, int irq, int dma, ushort samprate, ushort bufsize);
bool initsounddevice();
void killsounddevice();
