/*
-----------------------------------------------------------------------
<copyright file="Ini.h" company="Petr Abdulin">
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

int GetINIInt(const char *section, const char *key, int def, const char *filename);

void WriteINIInt(const char *section, const char *key, int value, const char *filename);

bool GetINIBool(const char *section, const char *key, bool def, const char *filename);

void WriteINIBool(const char *section, const char *key, bool value, const char *filename);
