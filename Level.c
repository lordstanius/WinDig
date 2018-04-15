/*
-----------------------------------------------------------------------
<copyright file="Level.c" company="Petr Abdulin">
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
#include "Level.h"
#include "Map.h"

/// <summary>
/// Gets the current level map (number).
/// </summary>
/// <param name="level">The level for which to get map.</param>
/// <returns>
/// The current level map (number).
/// </returns>
/// <remarks>
/// Maps goes like: 12345678, 678, (5678) 247 times, 5 forever.
/// </remarks>
short GetLevelMap(short level)
{
	// default, maps goes like: 12345678
	short map = level;

	if (level > MAX_MAP_COUNT)
	{
		// magic algorithm
		// maps goes like: 678, (5678) 247 times, 5 forever
		map = (level & 3) + 5;
	}

	return map;
}

/// <summary>
/// Gets the level difficulty modifier.
/// </summary>
/// <param name="level">The level for which to get diffifulty.</param>
/// <returns>
/// If current level is > 10, then return 10, otherwise return current level number.
/// </returns>
short GetLevelDifficultyModifier(short level)
{
	if (level > MAX_LEVEL_DIFFICULTY)
	{
		return MAX_LEVEL_DIFFICULTY;
	}

	return level;
}
