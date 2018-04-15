/*
-----------------------------------------------------------------------
<copyright file="Graphics.h" company="Petr Abdulin">
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

#include "Types.h"

/// <summary>
/// The graphics map size. 
/// </summary>
#define GRAPHICS_MAP_SIZE 240

/// <summary>
/// Free graphics map.
/// </summary>
/// <param name="asciiMap">[in,out] The graphics map array to deallocate.</param>
void FreeGraphicsMap(byte *graphicsMap[]);

/// <summary>
/// Reads a graphics data file.
/// </summary>
/// <param name="path">Full pathname of the file.</param>
/// <param name="indexMap">The graphics map index map.</param>
/// <param name="graphicsMap">[in,out] The graphics map array to fill. Must be previously allocated.</param>
void ReadGraphicsFile(const char *path, const char *indexMap[], byte *graphicsMap[]);
