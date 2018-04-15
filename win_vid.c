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

#include "def.h"
#include "Types.h"
#include "win_dig.h"
#include "hardware.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "win_vid.h"
#include "resource.h"
#include "Sprite.h"

struct FULLSCREEN_VID_MODE_INFO* supported_video_modes = NULL;  // list of supported video modes (modes that are VGA_640+ x VGA_400+)
struct FULLSCREEN_VID_MODE_INFO preferred_video_mode;         // the vid mode to use when running fullscreen

/// <summary> 
/// Flag indication whether direct draw is used for rendering.
/// </summary>
int g_IsUsingDirectDraw = 0;

/// <summary>
/// Title bitmaps (VGA and CGA).
/// </summary>
HBITMAP g_TitleBitmaps[2] = { (HBITMAP)NULL, (HBITMAP)NULL };

RGBQUAD vga16_pal1_rgbq[] = { {0,0,0,0},        /* palette1, normal intensity */
					  {128,0,0,0},
					  {0,128,0,0},
					  {128,128,0,0},
					  {0,0,128,0},
					  {128,0,128,0},
					  {0,64,128,0},
					  {128,128,128,0},
					  {64,64,64,0},
					  {255,0,0,0},
					  {0,255,0,0},
					  {255,255,0,0},
					  {0,0,255,0},
					  {255,0,255,0},
					  {0,255,255,0},
					  {255,255,255,0} };

RGBQUAD vga16_pal1i_rgbq[] = { {0,0,0,0},       /* palette1, high intensity */
					  {255,0,0,0},
					  {0,255,0,0},
					  {255,255,0,0},
					  {0,0,255,0},
					  {255,0,255,0},
					  {0,128,255,0},
					  {192,192,192,0},
					  {128,128,128,0},
					  {255,128,128,0},
					  {128,255,128,0},
					  {255,255,128,0},
					  {128,128,255,0},
					  {255,128,255,0},
					  {128,255,255,0},
					  {255,255,255,0} };

RGBQUAD vga16_pal2_rgbq[] = { {0,0,0,0},        /* palette2, normal intensity */
					  {0,128,0,0},
					  {0,0,128,0},
					  {0,64,128,0},
					  {128,0,0,0},
					  {128,128,0,0},
					  {128,0,128,0},
					  {128,128,128,0},
					  {64,64,64,0},
					  {0,255,0,0},
					  {0,0,255,0},
					  {0,255,255,0},
					  {255,0,0,0},
					  {255,255,0,0},
					  {255,0,255,0},
					  {255,255,255,0} };

RGBQUAD vga16_pal2i_rgbq[] = { {0,0,0,0},       /* palette2, high intensity */
					  {0,255,0,0},
					  {0,0,255,0},
					  {0,128,255,0},
					  {255,0,0,0},
					  {255,255,0,0},
					  {255,0,255,0},
					  {192,192,192,0},
					  {128,128,128,0},
					  {128,255,128,0},
					  {128,128,255,0},
					  {128,255,255,0},
					  {255,128,128,0},
					  {255,255,128,0},
					  {255,128,255,0},
					  {255,255,255,0} };

RGBQUAD cga16_pal1_rgbq[] = { {0,0,0,0},        /* palette1, normal intensity */
					  {0,168,0,0},
					  {0,0,168,0},
					  {0,84,168,0},
					  {0,0,128,0},
					  {128,0,128,0},
					  {0,64,128,0},
					  {128,128,128,0},
					  {64,64,64,0},
					  {255,0,0,0},
					  {0,255,0,0},
					  {255,255,0,0},
					  {0,0,255,0},
					  {255,0,255,0},
					  {0,255,255,0},
					  {255,255,255,0} };

RGBQUAD cga16_pal1i_rgbq[] = { {0,0,0,0},       /* palette1, high intensity */
					  {85,255,85,0},
					  {85,85,255,0},
					  {85,255,255,0},
					  {0,0,255,0},
					  {255,0,255,0},
					  {0,128,255,0},
					  {192,192,192,0},
					  {128,128,128,0},
					  {255,128,128,0},
					  {128,255,128,0},
					  {255,255,128,0},
					  {128,128,255,0},
					  {255,128,255,0},
					  {128,255,255,0},
					  {255,255,255,0} };

RGBQUAD cga16_pal2_rgbq[] = { {0,0,0,0},        /* palette2, normal intensity */
					  {0,128,0,0},
					  {128,0,128,0},
					  {160,160,160,0},
					  {160,160,160,0},
					  {128,128,0,0},
					  {128,0,128,0},
					  {128,128,128,0},
					  {64,64,64,0},
					  {0,255,0,0},
					  {0,0,255,0},
					  {0,255,255,0},
					  {255,0,0,0},
					  {255,255,0,0},
					  {255,0,255,0},
					  {255,255,255,0} };

RGBQUAD cga16_pal2i_rgbq[] = { {0,0,0,0},       /* palette2, high intensity */
					  {0,255,0,0},
					  {0,0,255,0},
					  {160,160,160,0},
					  {255,0,0,0},
					  {255,255,0,0},
					  {255,0,255,0},
					  {192,192,192,0},
					  {128,128,128,0},
					  {128,255,128,0},
					  {128,128,255,0},
					  {128,255,255,0},
					  {255,128,128,0},
					  {255,255,128,0},
					  {255,128,255,0},
					  {255,255,255,0} };


RGBQUAD *windowed_palette[4];  /* Used in Windowed mode. These palettes are applied to the 'back_bitmap' */

HPALETTE desktop_palette = (HPALETTE)NULL;                                     /* Used in Windowed mode, but is only if Windows is set to a color resolution which supports palettes.  {ie. 256 colors} ) */
bool palettized_desktop = FALSE;
LPDIRECTDRAWPALETTE    fullscreen_palette[4] = { NULL, NULL, NULL, NULL };      /* Used in Full Screen mode.  These palettes are applied to the DirectDraw primary surface. */
LPDIRECTDRAW            g_pDD = NULL;   /* DirectDraw object */
LPDIRECTDRAWSURFACE     g_pDDSPrimary = NULL;   /* DirectDraw primary surface */
//LPDIRECTDRAWSURFACE     g_pDDSBack    = NULL;   /* DirectDraw back surface */
RECT                    g_rcWindow;             /* Saves the window size & pos.*/
RECT                    g_rcViewport;           /* Pos. & size to blt from */
RECT                    g_rcScreen;             /* Screen pos. for blt */
bool                    g_bActive = FALSE;
bool                    g_bReady = FALSE;  /* App is ready for updates */
bool                    g_bWindowed = TRUE;   /* App is in windowed mode */
RECT                    rc_draw_area;
RECT                    rc_640x400;

bool use_async_screen_updates;
enum video_mode_enum video_mode = VIDEO_MODE_VGA_16;

int cur_intensity;
int cur_palette;

HDC back_dc;
HBITMAP back_bitmap;
byte *g_BackBitmapBits;
HGDIOBJ old_bitmap;
bool use_640x480_fullscreen;

byte *sprite_array[200];


HRESULT blit_to_window();
void GraphicsClear();
void destroy_palettes();
void init_palettes();

void InitializeGraphics()
{
	HDC window_dc = GetDC(g_hWnd);

	InitializeDirectDraw();

	CreateBackBuffer(window_dc);

	ReleaseDC(g_hWnd, window_dc);

	InitializeDirectDrawSurfaces();
}


/********************************************************/
/* Functions for displaying the VGA data                */
/********************************************************/
void vgainit()
{
	video_mode = VIDEO_MODE_VGA_16;
	windowed_palette[0] = vga16_pal1_rgbq;
	windowed_palette[1] = vga16_pal1i_rgbq;
	windowed_palette[2] = vga16_pal2_rgbq;
	windowed_palette[3] = vga16_pal2i_rgbq;
	destroy_palettes();
	init_palettes();
}

/// <summary>
/// Cleares up back bitmat.
/// </summary>
void GraphicsClear()
{
	memset(g_BackBitmapBits, 0, VGA_640 * VGA_400);

	blit_to_window();
}

void vgapal(short pal)
{
	cur_palette = pal;
	if (g_bWindowed || preferred_video_mode.bpp > 8)
	{
		SetDIBColorTable(back_dc, 0, 16, windowed_palette[cur_palette * 2 + cur_intensity]);
		if (use_async_screen_updates)
			InvalidateRect(g_hWnd, NULL, FALSE);
		else
			blit_to_window();
	}
	else
	{
		IDirectDrawSurface_SetPalette(g_pDDSPrimary, fullscreen_palette[cur_palette * 2 + cur_intensity]);
		SetDIBColorTable(back_dc, 0, 16, windowed_palette[cur_palette * 2 + cur_intensity]);
	}
};

void vgainten(short inten)
{
	cur_intensity = inten;
	if (g_bWindowed || preferred_video_mode.bpp > 8)
	{
		SetDIBColorTable(back_dc, 0, 16, windowed_palette[cur_palette * 2 + cur_intensity]);
		if (use_async_screen_updates)
			InvalidateRect(g_hWnd, NULL, FALSE);
		else
			blit_to_window();
	}
	else
	{
		IDirectDrawSurface_SetPalette(g_pDDSPrimary, fullscreen_palette[cur_palette * 2 + cur_intensity]);
		SetDIBColorTable(back_dc, 0, 16, windowed_palette[cur_palette * 2 + cur_intensity]);
	}
};

short vgagetpix(short x, short y)
{
	ushort xi, yi;
	short rval;

	rval = 0;
	if (x > 319 || y > 199)
	{
		return 0xff;
	}
	for (yi = 0; yi < 2; yi++)
		for (xi = 0; xi < 8; xi++)
			if (g_BackBitmapBits[(uint)((y * 2L + yi)*VGA_640 + x * 2L + xi)])
				rval |= 0x80 >> xi;

	rval &= 0xee;
	return rval;
};

void VgaWrite(short x, short y, short ch, short c)
{
	uint yi;
	uint xi;
	int color;

	ch -= 32;
	if (ch < 0x5f)
	{
		if (g_VgaSymbolMasks[ch])
		{
			for (yi = 0; yi < 24; yi++)
			{
				for (xi = 0; xi < 24; xi++)
				{
					if (xi & 0x1)
						color = (g_VgaSymbolMasks[ch][yi * 12 + (xi >> 1)] & 0x0F);
					else
						color = (g_VgaSymbolMasks[ch][yi * 12 + (xi >> 1)] >> 4);
					if (color == 10)
						if (c == 2)
							color = 12;
						else
						{
							if (c == 3)
								color = 14;
						}
					else
						if (color == 12)
							if (c == 1)
								color = 2;
							else
								if (c == 2)
									color = 4;
								else
									if (c == 3)
										color = 6;

					g_BackBitmapBits[(uint)((y * 2 + yi)*VGA_640 + x * 2L + xi)] = color;
				}
			}
		}
		else    /* draw a space (needed when reloading and displaying high scores when user switches to/from normal/g_IsGauntletMode mode, etc ) */
			for (yi = 0; yi < 24; yi++)
				memset(&g_BackBitmapBits[(y * 2 + yi)*VGA_640 + x * 2], 0, 24);
	}
	blit_rect_to_window(x * 2, y * 2, 24, 24);
}

/// <summary>
/// Cga write symbol.
/// </summary>
/// <param name="x">The x coordinate.</param>
/// <param name="y">The y coordinate.</param>
/// <param name="symbol">The symbol.</param>
/// <param name="color">The color.</param>
void CgaWrite(short x, short y, short symbol, short color)
{
	uint yi;
	uint xi;
	int alphaMask;
	int alphaColor;

	// adjust symbol, ' ' = 32, so ' ' becomes 0
	symbol -= 32;

	// if less than 127 (in ASCII), 0x5f = 95
	if (symbol < 0x5f)
	{
		// if present in map
		if (g_CgaSymbolMasks[symbol])
		{
			// lines
			for (yi = 0; yi < CGA_SYMBOL_ALPHA_HEIGHT; yi++)
			{
				// pixel in line
				for (xi = 0; xi < CGA_SYMBOL_ALPHA_WIDTH * 2; xi++)
				{
					// vertical "deinterlacing", will handle 0x0f and 0xf0 alpha values, and doesn't affect 0x00 and 0xff.
					// each row will be entered twice here
					if (xi % 2)
					{
						// right part of alpha
						// erase first 4 bits, get significant part from 0x0f values
						// will get 0 from 0xf0 
						alphaMask = (g_CgaSymbolMasks[symbol][yi * CGA_SYMBOL_ALPHA_WIDTH + (xi >> 1)] & 0x0F);
					}
					else
					{
						// left part of alpha
						// erase last 4 bits, get significant part from 0xf0 values
						// will get 0 from 0x0f
						alphaMask = (g_CgaSymbolMasks[symbol][yi * CGA_SYMBOL_ALPHA_WIDTH + (xi >> 1)] >> 4);
					}

					// apply alpha
					alphaColor = alphaMask & color;

					// draw 2x2 square
					memset(g_BackBitmapBits + ((y * 2 + yi * 2)*VGA_640 + x * 2 + xi * 2), alphaColor, 2);
					memset(g_BackBitmapBits + ((y * 2 + yi * 2 + 1)*VGA_640 + x * 2 + xi * 2), alphaColor, 2);
				}
			}
		}
		else
		{
			/* draw a space (needed when reloading and displaying high scores when user switches to/from normal/g_IsGauntletMode mode, etc ) */
			for (yi = 0; yi < 24; yi++)
			{
				memset(&g_BackBitmapBits[(y * 2 + yi)*VGA_640 + x * 2], 0, 24);
			}
		}
	}

	blit_rect_to_window(x * 2, y * 2, 24, 24);
}


void vgatitle()
{
	display_title_bitmap(0);
}

void cgatitle()
{
	display_title_bitmap(1);
}

void display_title_bitmap(int idx)
{
	HDC temp_dc;
	HGDIOBJ old_bitmap;
	GdiFlush();
	temp_dc = CreateCompatibleDC(back_dc);
	if (palettized_desktop)
	{
		SelectPalette(temp_dc, desktop_palette, FALSE);
		RealizePalette(temp_dc);
	}
	old_bitmap = SelectObject(temp_dc, g_TitleBitmaps[idx]);
	BitBlt(back_dc, 0, 0, VGA_640, VGA_400, temp_dc, 0, 0, SRCCOPY);
	GdiFlush();
	SelectObject(temp_dc, old_bitmap);
	DeleteDC(temp_dc);
	blit_to_window();

}

/* blit entire backbuffer to the window */
HRESULT blit_to_window()
{
	HRESULT hresult;
	DDSURFACEDESC ddsd;
	int x, y;
	RECT nodraw;
	POINT pt;

	ddsd.dwSize = sizeof(ddsd);
	hresult = DD_OK;

	if (!g_bWindowed && !g_bActive)
		return hresult;

	if (g_bWindowed || preferred_video_mode.bpp > 8)
	{
		InvalidateRect(g_hWnd, NULL, FALSE);
		UpdateWindow(g_hWnd);
	}
	else
	{
		//    if (g_pDDSPrimary && !suspend_game)
		//      return IDirectDrawSurface_Blt(g_pDDSPrimary,  &g_rcScreen, g_pDDSBack, &rc_draw_area, DDBLT_WAIT, NULL);
		if (cur_dialog_box)
			GetWindowRect(cur_dialog_box, &nodraw);
		IDirectDrawSurface_Lock(g_pDDSPrimary, NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
		for (x = 0; x < VGA_640; x++)
			for (y = 0; y < VGA_400; y++)
			{
				if (cur_dialog_box)
				{
					pt.x = x;
					pt.y = y;
					if (PtInRect(&nodraw, pt))
						continue;
				}
				((char*)ddsd.lpSurface)[y*ddsd.lPitch + x] = g_BackBitmapBits[y*VGA_640 + x];
			}
		IDirectDrawSurface_Unlock(g_pDDSPrimary, ddsd.lpSurface);
	}
	return DD_OK;
}

/* This is ONLY called when the window receives a WM_PAINT message. */
/* Repaint all or a portion of the window.                          */
/* (used in both Windowed and Fullscreen mode)                      */
void blit_to_window_dc(HWND wnd)
{
	HDC hDC;
	PAINTSTRUCT paintStruct;

	hDC = BeginPaint(g_hWnd, &paintStruct);
	if (palettized_desktop)
	{
		SelectPalette(hDC, desktop_palette, TRUE);
		RealizePalette(hDC);
	}

	if (g_bWindowed)
	{
		BitBlt(hDC, 0, 0, VGA_640, VGA_400, back_dc, 0, 0, SRCCOPY);
	}
	else
		BitBlt(hDC, g_rcScreen.left, g_rcScreen.top, VGA_640, VGA_400, back_dc, 0, 0, SRCCOPY);
	EndPaint(g_hWnd, &paintStruct);
}

HRESULT restore_surface()
{
	HRESULT hRet;
	hRet = IDirectDrawSurface_Restore(g_pDDSPrimary);
	if (hRet != DD_OK)
	{
		TerminateOnFatalError(hRet, "DirectDraw: restore surfaces (primary) failed");
	}
	return TRUE;
}

/* Blits the given rectangle to the Window (or marks the given rectangle */
/* as dirty if Async option is on).                                      */
/* In DirectX/Fullscreen mode, there must NOT be a Clipper attached to   */
/* the surface (otherwise BltFast will not work).                        */
void blit_rect_to_window(int x, int y, int w, int h)
{
	RECT rect;
	DDSURFACEDESC ddsd;
	int xi, yi;

	ddsd.dwSize = sizeof(ddsd);

	//  if (!g_bWindowed && !g_bActive)
	//    return;

	if (g_bWindowed || preferred_video_mode.bpp > 8)
	{
		rect.left = x;
		rect.right = x + w;
		rect.top = y;
		rect.bottom = y + h;
		InvalidateRect(g_hWnd, &rect, FALSE);
		if (!use_async_screen_updates)
			UpdateWindow(g_hWnd);
	}
	else
	{
		if (g_pDDSPrimary)
		{
			rect.left = x;
			rect.right = x + w;
			rect.top = y;
			rect.bottom = y + h;
			//      IDirectDrawSurface_BltFast(g_pDDSPrimary, g_rcScreen.left+x, g_rcScreen.top+y, g_pDDSBack, &rect, DDBLTFAST_NOCOLORKEY /* | DDBLTFAST_WAIT */);
			IDirectDrawSurface_Lock(g_pDDSPrimary, &rect, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
			for (xi = 0; xi < w; xi++)
				for (yi = 0; yi < h; yi++)
					((char*)ddsd.lpSurface)[yi*ddsd.lPitch + xi] = g_BackBitmapBits[(yi + y)*VGA_640 + xi + x];
			IDirectDrawSurface_Unlock(g_pDDSPrimary, ddsd.lpSurface);
		}
	}
}

/// <summary>
/// This function creates a back buffer for drawing on.
/// On Win32 this is a DIBSection.
/// </summary>
/// <param name="window_dc">The window device-context.</param>
/// <returns>
/// True if it succeeds, False if it fails.
/// </returns>
/// <see cref="http://msdn.microsoft.com/en-us/library/dd183376(v=vs.85).aspx"/>
bool CreateBackBuffer(HDC window_dc)
{
	HANDLE hloc;
	BITMAPINFO *pbmi;

	back_dc = CreateCompatibleDC(window_dc);
	back_bitmap = 0;

	hloc = LocalAlloc(LMEM_ZEROINIT | LMEM_MOVEABLE, sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD) * 16));
	pbmi = (BITMAPINFO*)LocalLock(hloc);
	pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pbmi->bmiHeader.biWidth = VGA_640;
	pbmi->bmiHeader.biHeight = -VGA_400;
	pbmi->bmiHeader.biPlanes = 1;
	pbmi->bmiHeader.biBitCount = 8;
	pbmi->bmiHeader.biClrUsed = 16;
	pbmi->bmiHeader.biClrImportant = 16;
	pbmi->bmiHeader.biCompression = BI_RGB;
	memset(pbmi->bmiColors, 0, sizeof(RGBQUAD) * 16);

	// create main bitmap for drawing on
	back_bitmap = CreateDIBSection(window_dc, (BITMAPINFO*)pbmi, DIB_RGB_COLORS, (VOID **)&g_BackBitmapBits, (HANDLE)NULL, 0);

	LocalFree(hloc);
	if (!back_bitmap)
		return FALSE;
	old_bitmap = SelectObject(back_dc, back_bitmap);

	return TRUE;
}

void destroy_back_buffer()
{
	release_directdraw_objects();
	SelectObject(back_dc, old_bitmap);
	DeleteObject(back_bitmap);
	DeleteDC(back_dc);
	DeleteObject(desktop_palette);
}

HRESULT release_directdraw_objects()
{
	if (g_IsUsingDirectDraw)
	{
		if (g_pDD != NULL)
		{
			IDirectDraw_RestoreDisplayMode(g_pDD);
			IDirectDraw_SetCooperativeLevel(g_pDD, g_hWnd, DDSCL_NORMAL);
			//      if (g_pDDSBack != NULL)
			//      {
			//          IDirectDraw_Release(g_pDDSBack);
			//          g_pDDSBack = NULL;
			//      }
			if (g_pDDSPrimary != NULL)
			{
				IDirectDraw_Release(g_pDDSPrimary);
				g_pDDSPrimary = NULL;
			}
		}
	}
	return DD_OK;
}

HRESULT TerminateOnFatalError(HRESULT hRet, LPCTSTR szError)
{
	windows_finish();

	MessageBox(g_hWnd, szError, "Digger", MB_OK);

	DestroyWindow(g_hWnd);

	exit(1);

	return hRet;
}


HRESULT InitializeDirectDrawSurfaces()
{
	HRESULT          hRet;
	DDSURFACEDESC   ddsd;
	DDBLTFX ddbltfx;
	int i, palnum;
	HDC hDC;
	LOGPALETTE*      logpalette;
	RGBQUAD *wp[] = { vga16_pal1_rgbq,vga16_pal1i_rgbq,vga16_pal2_rgbq,vga16_pal2i_rgbq };

	if (!g_IsUsingDirectDraw)
		g_bWindowed = TRUE;

	if (g_bWindowed)
	{
		/* Are we running in 256 colors (or less)?  */

		hDC = GetDC(g_hWnd);
		palettized_desktop = GetDeviceCaps(hDC, RASTERCAPS)&RC_PALETTE;
		if (palettized_desktop)
		{
			logpalette = (LOGPALETTE*)malloc(sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * 64);
			if (!logpalette)
				TerminateOnFatalError(0, "InitializeDirectDrawSurfaces: could not allocate memory for 'logpalette'");
			logpalette->palNumEntries = 64;
			logpalette->palVersion = 0x300;

			for (palnum = 0; palnum < 4; palnum++)
				for (i = 0; i < 16; i++)
				{
					logpalette->palPalEntry[palnum * 16 + i].peRed = wp[palnum][i].rgbRed;
					logpalette->palPalEntry[palnum * 16 + i].peBlue = wp[palnum][i].rgbBlue;
					logpalette->palPalEntry[palnum * 16 + i].peGreen = wp[palnum][i].rgbGreen;
					logpalette->palPalEntry[palnum * 16 + i].peFlags = (BYTE)NULL; //PC_NOCOLLAPSE;
				}

			desktop_palette = CreatePalette(logpalette);
			free(logpalette);
			SelectPalette(hDC, desktop_palette, TRUE);
			i = RealizePalette(hDC);
		}
		ReleaseDC(g_hWnd, hDC);
		if (g_IsUsingDirectDraw)
		{
			IDirectDraw_RestoreDisplayMode(g_pDD);
			hRet = IDirectDraw_SetCooperativeLevel(g_pDD, g_hWnd, DDSCL_NORMAL);
			if (hRet != DD_OK)
				return TerminateOnFatalError(hRet, "DirectDraw: SetCooperativeLevel FAILED");
		}
		GetClientRect(g_hWnd, &g_rcViewport);
		GetClientRect(g_hWnd, &g_rcScreen);
		ClientToScreen(g_hWnd, (POINT*)&g_rcScreen.left);
		ClientToScreen(g_hWnd, (POINT*)&g_rcScreen.right);
	}
	else
	{
		SetWindowLong(g_hWnd, GWL_STYLE, DIGGER_WS_FULLSCREEN);
		if (preferred_video_mode.height <= VGA_400)
			SetMenu(g_hWnd, NULL);
		hRet = IDirectDraw_SetCooperativeLevel(g_pDD, g_hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
		if (hRet != DD_OK)
			return TerminateOnFatalError(hRet, "DirectDraw: SetCooperativeLevel (fullscreen) FAILED");

		SetRect(&g_rcViewport, 0, 0, VGA_640, VGA_400);
		memcpy(&g_rcScreen, &g_rcViewport, sizeof(RECT));

		if (!use_640x480_fullscreen)
			hRet = IDirectDraw_SetDisplayMode(g_pDD, preferred_video_mode.width, preferred_video_mode.height, preferred_video_mode.bpp);
		if (hRet != DD_OK || use_640x480_fullscreen)
		{
			hRet = IDirectDraw_SetDisplayMode(g_pDD, VGA_640, 480, 8);
			if (hRet != DD_OK)
			{
				return TerminateOnFatalError(hRet, "DirectDraw: SetDisplayMode FAILED");
			}
			SetRect(&g_rcViewport, 0, 40, VGA_640, 440);
			memcpy(&g_rcScreen, &g_rcViewport, sizeof(RECT));
		}

		/* Create the primary surface */
		ZeroMemory(&ddsd, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
		hRet = IDirectDraw_CreateSurface(g_pDD, &ddsd, &g_pDDSPrimary, NULL);
		if (hRet != DD_OK)
			return TerminateOnFatalError(hRet, "DirectDraw: CreateSurface (primary, fullscreen) FAILED");

		vgainten(cur_intensity);

		/* clear the screen */
		ZeroMemory(&ddbltfx, sizeof(DDBLTFX));
		ddbltfx.dwFillColor = 0;
		ddbltfx.dwSize = sizeof(DDBLTFX);
		IDirectDrawSurface_Blt(g_pDDSPrimary, NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);

	}
	rc_640x400.left = 0;
	rc_640x400.top = 0;
	rc_640x400.right = VGA_640;
	rc_640x400.bottom = VGA_400;
	rc_draw_area.left = 0;
	rc_draw_area.top = 0;
	rc_draw_area.right = VGA_640;
	rc_draw_area.bottom = VGA_400;
	return DD_OK;
}

void destroy_palettes()
{
	fullscreen_palette[0] = NULL;
}

void init_palettes()
{
	// HRESULT          hRet; unreferenced
	PALETTEENTRY     palentry[256];
	int i, palnum;
	HDC hDC;
	LOGPALETTE*      logpalette;

	if (!g_IsUsingDirectDraw)
		g_bWindowed = TRUE;

	if (g_bWindowed)
	{
		/* Are we running in 256 colors (or less)?  */

		hDC = GetDC(g_hWnd);
		palettized_desktop = GetDeviceCaps(hDC, RASTERCAPS)&RC_PALETTE;
		if (palettized_desktop)
		{
			logpalette = (LOGPALETTE*)malloc(sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * 64);
			if (!logpalette)
				TerminateOnFatalError(0, "InitializeDirectDrawSurfaces: could not allocate memory for 'logpalette'");
			logpalette->palNumEntries = 64;
			logpalette->palVersion = 0x300;

			for (palnum = 0; palnum < 4; palnum++)
				for (i = 0; i < 16; i++)
				{
					logpalette->palPalEntry[palnum * 16 + i].peRed = windowed_palette[palnum][i].rgbRed;
					logpalette->palPalEntry[palnum * 16 + i].peBlue = windowed_palette[palnum][i].rgbBlue;
					logpalette->palPalEntry[palnum * 16 + i].peGreen = windowed_palette[palnum][i].rgbGreen;
					logpalette->palPalEntry[palnum * 16 + i].peFlags = (BYTE)NULL; //PC_NOCOLLAPSE;
				}

			desktop_palette = CreatePalette(logpalette);
			free(logpalette);
			SelectPalette(hDC, desktop_palette, TRUE);
			i = RealizePalette(hDC);
		}
		ReleaseDC(g_hWnd, hDC);

		/*
			if (g_IsUsingDirectDraw)
			{
			  hRet = IDirectDraw_SetCooperativeLevel(g_pDD, g_hWnd, DDSCL_NORMAL);
			  if (hRet != DD_OK)
				TerminateOnFatalError(hRet, "DirectDraw: SetCooperativeLevel FAILED");
			}
		*/
		GetClientRect(g_hWnd, &g_rcViewport);
		GetClientRect(g_hWnd, &g_rcScreen);
		ClientToScreen(g_hWnd, (POINT*)&g_rcScreen.left);
		ClientToScreen(g_hWnd, (POINT*)&g_rcScreen.right);
	}
	else
	{
		if (fullscreen_palette[0] == NULL)
		{
			for (palnum = 0; palnum < 4; palnum++)
			{
				for (i = 0; i < 256; i++)
				{
					if (i < 16)
					{
						palentry[i].peRed = windowed_palette[palnum][i].rgbRed;
						palentry[i].peBlue = windowed_palette[palnum][i].rgbBlue;
						palentry[i].peGreen = windowed_palette[palnum][i].rgbGreen;
						palentry[i].peFlags = (BYTE)NULL;
					}
					else
					{
						palentry[i].peRed = 0;
						palentry[i].peBlue = 0;
						palentry[i].peGreen = 0;
						palentry[i].peFlags = (BYTE)NULL;
					}
				}
				IDirectDraw_CreatePalette(g_pDD, DDPCAPS_8BIT, palentry, &fullscreen_palette[palnum], NULL);
			}
		}

		vgainten(cur_intensity);
	}
}

/* toggle between windowed and fullscreen mode */

HRESULT ChangeCoopLevel()
{
	HRESULT hRet;

	if (FAILED(hRet = release_directdraw_objects()))
		return TerminateOnFatalError(hRet, "DirectDraw: release_directdraw_objects FAILED");

	if (g_bWindowed)
	{
		SetWindowLong(g_hWnd, GWL_STYLE, DIGGER_WS_WINDOWED);
		SetWindowPos(g_hWnd, HWND_NOTOPMOST, g_rcWindow.left, g_rcWindow.top,
			(g_rcWindow.right - g_rcWindow.left),
			(g_rcWindow.bottom - g_rcWindow.top), SWP_SHOWWINDOW);
		InvalidateRect(NULL, NULL, TRUE);   // all windows should be repainted
	}
	hRet = InitializeDirectDrawSurfaces();
	init_palettes();
	return hRet;
}

void InitializeDirectDraw()
{
	HRESULT hRet;
	DDSURFACEDESC ddsd;

	GetWindowRect(g_hWnd, &g_rcWindow); // what is this doing here???

	if (!lpDirectDrawCreate)
	{
		g_IsUsingDirectDraw = 0;
		return;
	}

	hRet = DirectDrawCreate(NULL, &g_pDD, NULL);
	if (hRet == DD_OK)
	{
		g_IsUsingDirectDraw = 1;
	}
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwWidth = VGA_640;
	ddsd.dwFlags = DDSD_WIDTH;
	hRet = IDirectDraw_EnumDisplayModes(g_pDD, (DWORD)NULL, /*NULL*/ &ddsd, NULL, &EnumModesCallback);
}

void attach_clipper()
{
	LPDIRECTDRAWCLIPPER pClipper;
	HRESULT hRet;

	/* add a clipper to the primary surface */
	hRet = IDirectDraw_CreateClipper(g_pDD, 0, &pClipper, NULL);
	if (hRet != DD_OK)
		TerminateOnFatalError(hRet, "attach_clipper(): CreateClipper FAILED");
	hRet = IDirectDrawClipper_SetHWnd(pClipper, 0, g_hWnd);
	if (hRet != DD_OK)
		TerminateOnFatalError(hRet, "attach_clipper(): SetHWnd FAILED");
	hRet = IDirectDrawSurface_SetClipper(g_pDDSPrimary, pClipper);
	if (hRet != DD_OK)
		TerminateOnFatalError(hRet, "attach_clipper(): SetClipper FAILED");
	IDirectDrawClipper_Release(pClipper);
	pClipper = NULL;
}

void release_clipper()
{
	HRESULT hRet;
	hRet = IDirectDrawSurface_SetClipper(g_pDDSPrimary, (LPDIRECTDRAWCLIPPER)NULL);
	if (hRet != DD_OK)
		TerminateOnFatalError(hRet, "release_clipper(): SetClipper FAILED");
}

void vgaputim(short x, short y, short ch, short w, short h)
{
	/* TO DO: convert the vgagraphics to a more appropriate format,
			  rewrite this routine(load the sprites onto the DirectDraw
			  Surface beforehand). */


	uint scrn_width;
	uint scrn_height;
	int y_loop_end;
	int y_loop_count;
	int x_loop_end;
	int x_loop_count;
	unsigned char* cur_src_mask_ptr;
	unsigned char* cur_src_ptr;
	unsigned char* cur_dest_ptr;
	unsigned char* scrn_max_ptr;
	uint i;
	uint plane;
	int color;
	uint dest_next_row_offset;
	uint src_plane_offset;
	// RECT rect; unreferenced
	// DDSURFACEDESC ddsd; unreferenced
	// DDBLTFX ddbltfx; unreferenced
	// HRESULT hRet; unreferenced

	scrn_width = VGA_640;
	scrn_height = VGA_400;
	scrn_max_ptr = g_BackBitmapBits + (uint)scrn_width*scrn_height;

	cur_src_mask_ptr = vgatable[ch * 2 + 1];
	cur_src_ptr = vgatable[ch * 2];
	cur_dest_ptr = &(g_BackBitmapBits[(uint)(y * 2L * scrn_width + x * 2L)]);
	dest_next_row_offset = scrn_width - w * 8L;

	src_plane_offset = w * h * 2;

	y_loop_end = h * 2;
	x_loop_end = w;

	for (y_loop_count = 0; y_loop_count < y_loop_end; y_loop_count++)
	{
		for (x_loop_count = 0; x_loop_count < x_loop_end; x_loop_count++)
		{
			for (i = 0; i < 8; i++)
			{
				if (!((*cur_src_mask_ptr)&(0x80 >> i)))
				{
					color = 0;
					for (plane = 0; plane < 4; plane++)
					{
						color |= ((((*(cur_src_ptr + (uint)(plane*src_plane_offset))) << i) & 0x80) >> (4 + plane));
					}
					if (cur_dest_ptr < scrn_max_ptr)
						*cur_dest_ptr = color;

				}
				cur_dest_ptr++;
			}
			cur_src_ptr++;
			cur_src_mask_ptr++;
		}
		cur_dest_ptr += dest_next_row_offset;
	}

	if (y * 2 + h * 2 < VGA_400)
		blit_rect_to_window(x * 2, y * 2, w * 8, h * 2);
	else
		blit_rect_to_window(x * 2, y * 2, w * 8, h * 2 - (y * 2 + h * 2) + VGA_400);
}

void vgaputi(short x, short y, byte *p, short w, short h)
{
	//Unsignedint i;
	int i;

	for (i = 0; i < h * 2; i++)
	{
		if (i + y * 2 < VGA_400)
		{
			memcpy(g_BackBitmapBits + (uint)((y * 2 + i)*VGA_640 + x * 2), (char *)(p + (uint)(i*w * 8L)), w * 8L);
		}
	}

	if (y * 2 + h * 2 < VGA_400)
	{
		blit_rect_to_window(x * 2, y * 2, w * 8, h * 2);
	}
	else
	{
		blit_rect_to_window(x * 2, y * 2, w * 8, h * 2 - (y * 2 + h * 2) + VGA_400);
	}
}

/// <summary>
/// Grab an area of screen.
/// </summary>
/// <param name="x">The x coordinate.</param>
/// <param name="y">The y coordinate.</param>
/// <param name="p">The place to grab to.</param>
/// <param name="w">The width.</param>
/// <param name="h">The height.</param>
void vgageti(short x, short y, byte *p, short w, short h)
{
	//Unsignedint i;
	int i;

	for (i = 0; i < h * 2; i++)
		if (i + y * 2 < VGA_400)
			memcpy((char *)(p + (uint)i*w * 8L), g_BackBitmapBits + (uint)((y * 2L + i)*VGA_640 + x * 2L), w * 8);
}

void cgaputim(short x, short y, short ch, short w, short h)
{
	/* TO DO: convert the cgagraphics to a more appropriate format,
			  rewrite this routine(load the sprites onto the DirectDraw
			  Surface beforehand). */

	uint scrn_width;
	uint scrn_height;
	int y_loop_end;
	int y_loop_count;
	int x_loop_end;
	int x_loop_count;
	unsigned char* cur_src_mask_ptr;
	unsigned char* cur_src_ptr;
	unsigned char* cur_dest_ptr;
	unsigned char* scrn_max_ptr;
	uint i;
	int color;
	uint dest_next_row_offset;
	// RECT rect; unreferenced
	// DDSURFACEDESC ddsd; unreferenced
	// DDBLTFX ddbltfx; unreferenced
	// HRESULT hRet; unreferenced

	scrn_width = VGA_640;
	scrn_height = VGA_400;
	scrn_max_ptr = g_BackBitmapBits + (uint)scrn_width*scrn_height;

	cur_src_mask_ptr = cgatable[ch * 2 + 1];
	cur_src_ptr = cgatable[ch * 2];
	cur_dest_ptr = &(g_BackBitmapBits[(uint)(y * 2 * scrn_width + x * 2)]);
	dest_next_row_offset = scrn_width; // - w*4L;

	y_loop_end = h;
	x_loop_end = w;

	for (y_loop_count = 0; y_loop_count < y_loop_end; y_loop_count++)
	{
		for (x_loop_count = 0; x_loop_count < x_loop_end; x_loop_count++)
		{
			for (i = 0; i < 4; i++)
			{
				if (!((*cur_src_mask_ptr)&(0xC0 >> (i * 2))))
				{
					color = ((*cur_src_ptr) >> (6 - (i * 2))) & 0x03;
					if (cur_dest_ptr < scrn_max_ptr)
						*cur_dest_ptr = *(cur_dest_ptr + 1) = *(cur_dest_ptr + dest_next_row_offset) = *(cur_dest_ptr + dest_next_row_offset + 1) = color;
				}
				cur_dest_ptr += 2;
			}
			cur_src_ptr++;
			cur_src_mask_ptr++;
		}
		cur_dest_ptr += (dest_next_row_offset * 2 - w * 8);
	}
	if (y * 2 + h * 2 < VGA_400)
		blit_rect_to_window(x * 2, y * 2, w * 8, h * 2);
	else
		blit_rect_to_window(x * 2, y * 2, w * 8, h * 2 - (y * 2 + h * 2) + VGA_400);
}

short cgagetpix(short x, short y)
{
	ushort xi, yi;
	short rval;

	rval = 0;
	if (x > 319 || y > 199)
	{
		return 0xff;
	}
	for (yi = 0; yi < 2; yi++)
		for (xi = 0; xi < 8; xi++)
			if (g_BackBitmapBits[(uint)((y * 2L + yi)*VGA_640 + x * 2L + xi)])
				rval |= 0x80 >> xi;

	rval &= 0xee;
	return rval;
}

/*******************************************************/
/* Functions for displaying the CGA data               */
/*******************************************************/
void cgainit()
{
	video_mode = VIDEO_MODE_CGA;
	windowed_palette[0] = cga16_pal1_rgbq;
	windowed_palette[1] = cga16_pal1i_rgbq;
	windowed_palette[2] = cga16_pal2_rgbq;
	windowed_palette[3] = cga16_pal2i_rgbq;
	destroy_palettes();
	init_palettes();
}

void cgapal(short pal)
{
	vgapal(pal);
}

void cgainten(short inten)
{
	vgainten(inten);
}

void LoadTitleBitmaps()
{
	g_TitleBitmaps[0] = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_TITLEBITMAP));
	g_TitleBitmaps[1] = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_CGATITLEBITMAP));
}

void change_graphics_routines(byte newmode)
{
	g_bReady = FALSE;
	ginit = cgainit;
	switch (newmode)
	{
	case VIDEO_MODE_CGA:
		gpal = cgapal;
		ginten = cgainten;
		ggetpix = cgagetpix;
		gputim = cgaputim;
		g_DrawSymbolFunc = CgaWrite;
		gtitle = cgatitle;
		break;
	case VIDEO_MODE_VGA_16:
		ginit = vgainit;
		gpal = vgapal;
		ginten = vgainten;
		ggetpix = vgagetpix;
		gputim = vgaputim;
		g_DrawSymbolFunc = VgaWrite;
		gtitle = vgatitle;
	}

	video_mode = (enum video_mode_enum)newmode;
	g_bReady = TRUE;

	GraphicsClear();

	ginit();
	gpal(0);
	reset_main_menu_screen = TRUE;
}

HRESULT WINAPI EnumModesCallback(LPDDSURFACEDESC lpddsd, LPVOID lpContext)
{
	struct FULLSCREEN_VID_MODE_INFO* vmode;

	if (lpddsd->dwHeight >= VGA_400)
	{
		if (supported_video_modes == NULL)
		{
			vmode = supported_video_modes = (struct FULLSCREEN_VID_MODE_INFO *) malloc(sizeof(struct FULLSCREEN_VID_MODE_INFO));
		}
		else
		{
			vmode = supported_video_modes;
			while (vmode->next_mode != NULL)
			{
				vmode = vmode->next_mode;
			}
			vmode->next_mode = (struct FULLSCREEN_VID_MODE_INFO *) malloc(sizeof(struct FULLSCREEN_VID_MODE_INFO));
			vmode = vmode->next_mode;
		}
		vmode->width = lpddsd->dwWidth;
		vmode->height = lpddsd->dwHeight;
		vmode->bpp = lpddsd->ddpfPixelFormat.dwRGBBitCount;
		vmode->next_mode = NULL;
	}
	return DDENUMRET_OK;
}

void set_preferred_vidmode(int w, int h, int bpp)
{
	preferred_video_mode.width = w;
	preferred_video_mode.height = h;
	preferred_video_mode.bpp = bpp;
	preferred_video_mode.next_mode = NULL;

	// TODO: if we are in fullscreen mode, then switch to new mode
}
