/*
-----------------------------------------------------------------------
<copyright file="Ini.c" company="Petr Abdulin">
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

#include <Windows.h>
#include <stdio.h>
#include "def.h"
#include "Types.h"

int GetINIInt(const char *section, const char *key, int def, const char *filename)
{
	char buf[STRBUF_32];
	char buf_out[STRBUF_32];

	// default value
	sprintf(buf, "%i", def);

	GetPrivateProfileString(section, key, buf, buf_out, STRBUF_32, filename);

	return atoi(buf_out);
}

void WriteINIInt(const char *section, const char *key, int value, const char *filename)
{
	char buf[STRBUF_32];

	// prepare value
	sprintf(buf, "%i", value);

	WritePrivateProfileString(section, key, buf, filename);
}

bool GetINIBool(const char *section, const char *key, bool def, const char *filename)
{
	char buf_out[STRBUF_32];

	GetPrivateProfileString(section, key, NULL, buf_out, STRBUF_32, filename);

	// not found, return default
	if (buf_out[0] == 0)
	{
		return def;
	}

	// found, check and return
	if (_stricmp(buf_out, TRUE_STR) == 0)
	{
		return TRUE;
	}

	// any other than TRUE is false
	return FALSE;
}

void WriteINIBool(const char *section, const char *key, bool value, const char *filename)
{
	WritePrivateProfileString(section, key, value ? TRUE_STR : FALSE_STR, filename);
}
