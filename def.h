/*
-----------------------------------------------------------------------
<copyright file="def.h" company="Petr Abdulin">
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

#ifndef DEF_H
#define DEF_H

/// <summary>
/// String buffer predefined sizes.
/// </summary>
#define STRBUF_32 32
#define STRBUF_512 512
#define STRBUF_4096 4096

#define DIR_NONE -1
#define DIR_RIGHT 0
#define DIR_UP 2
#define DIR_LEFT 4
#define DIR_DOWN 6

/// <summary>
/// The total number of sprite types.
/// </summary>
#define SPRITE_TYPES 5

/// <summary>
/// The sprite type index in collections.
/// </summary>
#define SPRITE_TYPE_BONUS 0
#define SPRITE_TYPE_BAG 1
#define SPRITE_TYPE_MONSTER 2
#define SPRITE_TYPE_FIREBALL 3
#define SPRITE_TYPE_DIGGER 4

#define BONUSES 1
#define BAGS 50


#define DIGGERS 2

/// <summary>
/// Number of fireballs are equal to number of diggers.
/// </summary>
#define FIREBALLS DIGGERS

/// <summary>
/// Total number of the sprites (total is: 1+50+6+2+2 = 61).
/// </summary>
#define SPRITES (BONUSES + BAGS + MONSTERS + FIREBALLS + DIGGERS)

// Sprite order is figured out here. By LAST I mean last + 1.
#define FIRSTBONUS 0
#define LASTBONUS (FIRSTBONUS + BONUSES)

#define FIRSTBAG LASTBONUS
#define LASTBAG (FIRSTBAG + BAGS)

#define FIRSTMONSTER LASTBAG
#define LASTMONSTER (FIRSTMONSTER + MONSTERS)

#define FIRSTFIREBALL LASTMONSTER
#define LASTFIREBALL (FIRSTFIREBALL + FIREBALLS)

#define FIRSTDIGGER LASTFIREBALL
#define LASTDIGGER (FIRSTDIGGER + DIGGERS)

// Game filed sizes
#define MWIDTH 15
#define MHEIGHT 10
#define MSIZE (MWIDTH*MHEIGHT)

// levels number
#define MAX_LEVELS 8

/// <summary>
/// Gets the maximum record buffer.
/// </summary>
/// <remarks>
/// I reckon this is enough for about 36 hours of continuous play.
/// </remarks>
#define MAX_REC_BUFFER 262144L

#define INI_GAME_SETTINGS "Game"
#define INI_GRAPHICS_SETTINGS "Graphics"
#define INI_SOUND_SETTINGS "Sound"


#define INI_KEY_SETTINGS "Win Keys"


#define DEFAULT_BUFFER 2048
#define DEF_SND_DEV 1

#if defined _WINDOWS
#define DIGGER_VERSION "TD WIN 20020310"
#elif defined ARM
#define DIGGER_VERSION "JB ARM 19990320"
#else
#define DIGGER_VERSION "AJ DOS 20010825"
#endif

/* Version string:
  First word: your initials if you have changed anything.
  Second word: platform.
  Third word: compilation date in yyyymmdd format. */

#endif