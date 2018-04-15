/*
-----------------------------------------------------------------------
<copyright file="" company="Petr Abdulin">
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

#define DIRECTDRAW_VERSION 0x0300
#include <ddraw.h>

HRESULT restore_surface();
void InitializeDirectDraw();
HRESULT ChangeCoopLevel();
HRESULT release_directdraw_objects();
void toggle_screen_mode();
void attach_clipper();
void release_clipper();
extern LPDIRECTDRAWSURFACE g_pDDSPrimary;
extern int g_IsUsingDirectDraw;

void blit_rect_to_window(int x, int y, int w, int h);
HRESULT blit_to_window();
HRESULT InitializeDirectDrawSurfaces();
HRESULT TerminateOnFatalError(HRESULT hRet, LPCTSTR szError);
bool CreateBackBuffer(HDC window_dc);
void destroy_back_buffer();
void blit_to_window_dc(HWND wnd);
void InitializeGraphics();
void display_title_bitmap(int i);
void LoadTitleBitmaps();

extern bool g_bActive;
extern bool g_bReady;
extern bool g_bWindowed;
extern RECT g_rcWindow;
extern RECT g_rcViewport;
extern RECT g_rcScreen;
HRESULT InitializeDirectDrawSurfaces();
extern HBITMAP g_TitleBitmaps[2];
extern bool use_async_screen_updates;
extern int cur_intensity;
extern int cur_palette;
extern bool palettized_desktop;
extern HDC back_dc;
extern HBITMAP back_bitmap;
extern byte *g_BackBitmapBits;
extern HGDIOBJ old_bitmap;
extern bool use_640x480_fullscreen;

extern char *g_MapIndexTable[];
extern byte *g_VgaSymbolMasks[];
extern byte *g_CgaSymbolMasks[];
extern byte *vgatable[];
extern byte *cgatable[];
extern void DrawGameText(char *p, short x, short y, short c);

enum video_mode_enum
{
	VIDEO_MODE_CGA = 0,
	VIDEO_MODE_VGA_16 = 1,
	VIDEO_MODE_VGA_256 = 2
} video_mode;

extern HINSTANCE hDirectDrawInstance;
extern HRESULT(WINAPI *lpDirectDrawCreate)(GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter);

void change_graphics_routines(byte newmode);

struct FULLSCREEN_VID_MODE_INFO
{
	int width;
	int height;
	int bpp;
	struct FULLSCREEN_VID_MODE_INFO* next_mode;
};

HRESULT WINAPI EnumModesCallback(LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext);
extern struct FULLSCREEN_VID_MODE_INFO* supported_video_modes;
void set_preferred_vidmode(int w, int h, int bpp);
extern struct FULLSCREEN_VID_MODE_INFO preferred_video_mode;


/// <summary>
/// The vga graphics maximum x coordinate.
/// </summary>
#define VGA_640 640

/// <summary>
/// The vga graphics maximum y coordinate.
/// </summary>
#define VGA_400 400

#define CGA_SYMBOL_ALPHA_WIDTH 6
#define CGA_SYMBOL_ALPHA_HEIGHT 12
