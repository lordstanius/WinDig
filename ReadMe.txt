=== Notes on implementation ===
1. Symbol is a 24x24 (HxW) pixel square (in 640*400 mode pixels).
For 320*200 mode it is actually a 12x12 square. 
It uses rather complicated storage for it's mask.
The byte array is 12x6 size, but each of 6 bytes in horizontal row actually holds 
two 4-bit mask data, thus resulting in 12x12 symbol.
This significally complicates drawing process.

2a. Since back buffer is always 640x400 resolution, all coordinates passed to
graphics functions are for this resolution.
2b. Coodinates go from top left to bottom right. Passed to functions 
coordinates also specify top left corner from wich object will be drawn.

=== Acronyms ===
1. DRF - Digger recording file
