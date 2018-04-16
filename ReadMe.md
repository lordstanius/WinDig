# Digger (for Windows)
Classic game from Windmill Software (1983) remastered by Andrew Jenner and improved by Petr Abdulin and others, updated to build with VS2017 and run on Windows.

#### Notes on implementation (by Pert Abdulin)
1. Symbol is a 24x24 (HxW) pixel square (in 640x400 mode pixels).
For 320x200 mode it is actually a 12x12 square. 
It uses rather complicated storage for it's mask.
The byte array is 12x6 size, but each of 6 bytes in horizontal row actually holds 
two 4-bit mask data, thus resulting in 12x12 symbol.
This significally complicates drawing process.

2. Since back buffer is always 640x400 resolution, all coordinates passed to
graphics functions are for this resolution.
3. Coodinates go from top left to bottom right. Passed to functions 
coordinates also specify top left corner from which object will be drawn.

#### Acronyms
DRF - Digger recording file
