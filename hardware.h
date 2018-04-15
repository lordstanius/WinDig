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

void olddelay(short t);
void inittimer();

/// <summary>
/// Gets the "high" resolution time.
/// </summary>
/// <returns>
/// The "high" resolution time.
/// </returns>
/// <remarks>
/// This function have very deep root into game random number generation,
/// so treat it carefully.
/// </remarks>
uint GetHighResolutionTime();

void s0initint8();
void s0restoreint8();
void s0soundoff();
void s0setspkrt2();
void s0settimer0(ushort t0v);
void s0timer0(ushort t0v);
void s0settimer2(ushort t2v);
void s0timer2(ushort t2v);
void s0soundinitglob();
void s0soundkillglob();
void s1initint8();
void s1restoreint8();

void initkeyb();
void restorekeyb();
short getkey();
bool kbhit();

void cgainit();
void cgapal(short pal);
void cgainten(short inten);
void cgaputim(short x, short y, short ch, short w, short h);
short cgagetpix(short x, short y);

/// <summary>
/// Cga write symbol.
/// </summary>
/// <param name="x">The x coordinate.</param>
/// <param name="y">The y coordinate.</param>
/// <param name="symbol">The symbol.</param>
/// <param name="color">The color.</param>
void CgaWrite(short x, short y, short symbol, short color);

void cgatitle();

void vgainit();
void GraphicsClear();
void vgapal(short pal);
void vgainten(short inten);
void vgaputi(short x, short y, byte *p, short w, short h);
void vgageti(short x, short y, byte *p, short w, short h);
void vgaputim(short x, short y, short ch, short w, short h);
short vgagetpix(short x, short y);
void VgaWrite(short x, short y, short ch, short c);
void vgatitle();
