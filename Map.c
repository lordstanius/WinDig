/*
-----------------------------------------------------------------------
<copyright file="Map.c" company="Petr Abdulin">
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
#include "Map.h"

/// <summary>
/// Check if 'mapSymbol' is a gold bag.
/// </summary>
/// <param name="mapSymbol">The map symbol.</param>
/// <returns>
/// TRUE (1) if bag, false (0) if not.
/// </returns>
bool IsBag(int mapSymbol)
{
	if (mapSymbol == MAP_SYMBOL_BAG)
	{
		return 1;
	}

	return 0;
}
