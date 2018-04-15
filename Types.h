#pragma once
/*
-----------------------------------------------------------------------
<copyright file="Types.h" company="Petr Abdulin">
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

typedef int bool;

/// <summary>
/// The value for TRUE. 
/// This one is for old code compatibility purposes only, 
/// all new bool functions should use raw 1 for TRUE.
/// </summary>
#define TRUE 1

/// <summary>
/// The value for FALSE. 
/// This one is for old code compatibility purposes only, 
/// all new bool functions should use raw 0 for false.
/// </summary>
#define FALSE 0

/// <summary>
/// The TRUE value string.
/// </summary>
#define TRUE_STR "True"

/// <summary>
/// The false value string.
/// </summary>
#define FALSE_STR "False"

/// <summary>
/// Defines an alias representing the signed/unsigned byte size type (1 byte).
/// </summary>
typedef signed char sbyte;
typedef unsigned char byte;

/// <summary>
/// Defines an alias representing the signed/unsigned word size type (2 bytes).
/// </summary>
typedef unsigned short ushort;

/// <summary>
/// Defines an alias representing the signed/unsigned double word size type (4 bytes).
/// </summary>
typedef unsigned int uint;
