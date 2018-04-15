/*
-----------------------------------------------------------------------
<copyright file="Drawing.h" company="Petr Abdulin">
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

#include "Main.h"

/// <summary>
/// Black (transparent) color.
/// </summary>
#define COLOR_0 0

/// <summary>
/// Green color.
/// </summary>
#define COLOR_1 1 

/// <summary>
/// Red color.
/// </summary>
#define COLOR_2 2

/// <summary>
/// Orange color.
/// </summary>
#define COLOR_3 3

/// <summary>
/// Symbol (character) width.
/// </summary>
#define SYMBOL_WIDTH 12

/// <summary>
/// Symbol (character) height.
/// </summary>
#define SYMBOL_HEIGHT 18

/// <summary>
/// Graphics block width offset.
/// </summary>
#define BLOCK_WIDTH_OFFSET 12

/// <summary>
/// Graphics block width.
/// </summary>
#define BLOCK_WIDTH 20

/// <summary>
/// Graphics block height offset.
/// </summary>
#define BLOCK_HEIGHT_OFFSET 18

/// <summary>
/// Graphics block height.
/// </summary>
#define BLOCK_HEIGHT 18

/// <summary>
/// Draw game text.
/// </summary>
/// <param name="text">Text to draw.</param>
/// <param name="x">The x coordinate.</param>
/// <param name="y">The y coordinate.</param>
/// <param name="color">The color of the text.</param>
void DrawGameText(char *text, short x, short y, short color);

void CreateMonsterAndBagSprites();
void InitializeMonsterAndBagSprites();
void drawmon(short n, bool nobf, short dir, short x, short y);
void drawdigger(int n, short t, short x, short y, bool f);
void drawgold(short n, short t, short x, short y);
void drawemerald(short x, short y);
void eraseemerald(short x, short y);
void drawbonus(short x, short y);
void DrawLivesAndGauntletTime();
void savefield();
void makefield();
void drawstatics();
void drawfire(int n, short x, short y, short t);
void eatfield(short x, short y, short dir);
void drawrightblob(short x, short y);
void drawleftblob(short x, short y);
void drawtopblob(short x, short y);
void drawbottomblob(short x, short y);
void drawmondie(short n, bool nobf, short dir, short x, short y);
void drawfurryblob(short x, short y);
void drawsquareblob(short x, short y);

extern short g_Field[];
extern short fireheight;

/// <summary> Information describing the game </summary>
extern _game g_GameData[MAX_PLAYERS];
