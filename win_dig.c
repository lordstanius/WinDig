/*
-----------------------------------------------------------------------
<copyright file="win_dig.c" company="Petr Abdulin">
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

#define INITGUID
#include "def.h"
#include "Types.h"
#include <stdio.h>
#include <stdlib.h>
#include "win_dig.h"
#include <windowsx.h>
#include "resource.h"
#include "Scores.h"
#include <commctrl.h>
#include "win_vid.h"
#include "win_snd.h"
#include "win_cfg.h"
#include "Main.h"
#include "ini.h"
#include "hardware.h"
#include "Sprite.h"
#include "record.h"
#include "Scores.h"
#include "Graphics.h"
#include "Input.h"

extern void ReleaseResources();
extern int g_KeyboardKeysSettings[][MAX_ACTION_KEYS];
extern void redefkeyb(bool allf);
extern void ParseCommandLine(int argc, char *argv[]);

extern bool g_UseJoystick;

/* these are used by the menu/dialogue boxes */
extern short g_PlayersCount, g_DiggersCount, g_CurrentPlayer, g_StartingLevel;
extern bool start;
extern void DrawGameMode();
extern bool g_IsGauntletMode;
extern int g_OptionGauntletTimeInSeconds;
extern bool soundflag, musicflag;
extern bool g_IsPaused;
extern uint g_GameFrameTime;
extern bool g_IsTimeout;
extern ushort size;
extern bool soundlevdoneflag;
extern bool g_IsGameCycleEnd;
extern short g_GameFrame;
extern bool g_IsStarted;

HINSTANCE hDirectDrawInstance = NULL;
HINSTANCE hDirectSoundInstance = NULL;
HRESULT(WINAPI *lpDirectSoundCreate)(LPCGUID, LPDIRECTSOUND *, LPUNKNOWN);
HRESULT(WINAPI *lpDirectDrawCreate)(GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter);


// This will store a copy of the original level data so that the user can return to using the original set of level, after loading one of the DLF files
sbyte g_OriginalLevelsData[MAX_LEVELS][MHEIGHT][MWIDTH];
ushort g_OriginalBonusScore;

// This will store a copy of the 'current' level data so that the current level data is not lost when a DRF is played
sbyte g_BackupLevelsData[MAX_LEVELS][MHEIGHT][MWIDTH];
ushort backup_bonusscore;
char backup_levfname[STRBUF_512];
bool backup_levfflag;

char drf_filename[STRBUF_512] = "";
int use_direct_input = 0;

bool suspend_game = FALSE;

HWND g_hWnd;
SIZE g_WindowSize;
HINSTANCE g_hInstance;
bool reset_main_menu_screen = FALSE;
bool use_performance_counter;
_int64 performance_frequency;
int kb_buffer = 0;
int command_buffer = 0;

HWND cur_dialog_box = (HWND)NULL;
char* params[10];
int param_count;
bool main_function_started = FALSE;   /* has main() been called yet? */
bool shutting_down = FALSE;
HMENU hMenu;
char g_DiggerDirectory[STRBUF_512];

/// <summary>
/// Digger INI file full path.
/// </summary>
char g_DiggerIniPath[STRBUF_512] = { 0 };

LRESULT CALLBACK gauntlet_settings_dialog_proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK sound_settings_dialog_proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK help_about_dialog_proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK levels_dialog_proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

/* time routines */
void olddelay(short t) { };

void inittimer() { };

/// <summary>
/// Gets the "high" resolution time.
/// </summary>
/// <returns>
/// The "high" resolution time.
/// </returns>
uint GetHighResolutionTime()
{
	return GetTickCount() * GAME_TIME_TICK_MULTIPLIER;
};

short getkey()  // returns the Virtual Key Code of the last key pressed
{
	short temp_buffer;
	do
	{
		if (kb_buffer)
		{
			temp_buffer = kb_buffer;
			kb_buffer = 0;
			return temp_buffer;
		}
		else
			ProcessWindowsEvents();
	} while (TRUE);

};

short getcommand()   // similar to getkey() but the value returned has already been translated from the keycode to the function/command that the key is mapped to
{
	short temp_buffer;

	temp_buffer = command_buffer;
	command_buffer = 0;
	return temp_buffer;
};
bool kbhit()
{
	return (kb_buffer != 0);
};
bool cmdhit()
{
	return (command_buffer);
}

void initkeyb() {};
void restorekeyb() {};


void clear_nplayerlines()
{
	DrawGameText("          ", 180, 25, 3);
	DrawGameText("            ", 170, 39, 3);
}

void refresh_menu_items()
{
	/* set checkmarks, etc. */
	HMENU cur_menu;

	cur_menu = GetMenu(g_hWnd);
	CheckMenuItem(cur_menu, ID_GAME_PLAYERS_ONE, MF_BYCOMMAND | (g_DiggersCount > 1 || g_PlayersCount > 1 ? MF_UNCHECKED : MF_CHECKED));
	CheckMenuItem(cur_menu, ID_GAME_PLAYERS_TWO, MF_BYCOMMAND | (g_DiggersCount > 1 || g_PlayersCount != 2 ? MF_UNCHECKED : MF_CHECKED));
	CheckMenuItem(cur_menu, ID_GAME_PLAYERS_TWOSIMULTANEOUS, MF_BYCOMMAND | (g_DiggersCount != 2 ? MF_UNCHECKED : MF_CHECKED));
	CheckMenuItem(cur_menu, ID_GAME_MODE_NORMAL, MF_BYCOMMAND | (g_IsGauntletMode ? MF_UNCHECKED : MF_CHECKED));
	CheckMenuItem(cur_menu, ID_GAME_MODE_GAUNTLET, MF_BYCOMMAND | (!g_IsGauntletMode ? MF_UNCHECKED : MF_CHECKED));
	CheckMenuItem(cur_menu, ID_SOUND_SOUNDS_PLAY, MF_BYCOMMAND | (!soundflag ? MF_UNCHECKED : MF_CHECKED));
	CheckMenuItem(cur_menu, ID_SOUND_MUSIC_PLAY, MF_BYCOMMAND | (!musicflag ? MF_UNCHECKED : MF_CHECKED));
	CheckMenuItem(cur_menu, ID_CONFIGURATION_GRAPHICS_ASYNCHRONOUS, MF_BYCOMMAND | (!use_async_screen_updates ? MF_UNCHECKED : MF_CHECKED));
	CheckMenuItem(cur_menu, ID_CONFIGURATION_SPEED_DEFAULT, MF_BYCOMMAND | (g_GameFrameTime != 80000L ? MF_UNCHECKED : MF_CHECKED));
	CheckMenuItem(cur_menu, ID_VIEW_FULLSCREEN, MF_BYCOMMAND | (g_bWindowed ? MF_UNCHECKED : MF_CHECKED));
	EnableMenuItem(cur_menu, ID_VIEW_FULLSCREEN, MF_BYCOMMAND | (g_IsUsingDirectDraw /*==6*/ ? MF_ENABLED : MF_GRAYED));
	CheckMenuItem(cur_menu, ID_VIEW_VGAGRAPHICS, MF_BYCOMMAND | (video_mode == VIDEO_MODE_VGA_16 ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(cur_menu, ID_VIEW_CGAGRAPHICS, MF_BYCOMMAND | (video_mode == VIDEO_MODE_CGA ? MF_CHECKED : MF_UNCHECKED));
	EnableMenuItem(cur_menu, ID_RECORDING_SAVE, MF_BYCOMMAND | (g_IsGameRecordAvailable && g_IsDrfValid) ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(cur_menu, ID_RECORDING_INSTANTREPLAY, MF_BYCOMMAND | (g_IsGameRecordAvailable && g_IsDrfValid) ? MF_ENABLED : MF_GRAYED);
}

// Display the menu which should be displayed when a game is in progress
void show_game_menu()
{
	// main menu
	HMENU main_menu;
	HMENU cur_menu;

	main_menu = GetMenu(g_hWnd);
	EnableMenuItem(main_menu, ID_GAME_ABORT, MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(main_menu, ID_GAME_PAUSE, MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(main_menu, ID_GAME_START, MF_BYCOMMAND | MF_GRAYED);
	EnableMenuItem(main_menu, ID_VIEW_VGAGRAPHICS, MF_BYCOMMAND | MF_GRAYED);
	EnableMenuItem(main_menu, ID_VIEW_CGAGRAPHICS, MF_BYCOMMAND | MF_GRAYED);
	EnableMenuItem(main_menu, ID_CONFIGURATION_CONTROLS_KEYBOARD_REDEFINEKEYS, MF_BYCOMMAND | MF_GRAYED);
	EnableMenuItem(main_menu, ID_CONFIGURATION_CONTROLS_KEYBOARD_REDEFINEALLKEYS, MF_BYCOMMAND | MF_GRAYED);

	// ? disable some items
	cur_menu = GetSubMenu(main_menu, 0);
	EnableMenuItem(cur_menu, 5, MF_BYPOSITION | MF_GRAYED); // MODE>
	EnableMenuItem(cur_menu, 6, MF_BYPOSITION | MF_GRAYED); // PLAYERS>
	EnableMenuItem(cur_menu, 7, MF_BYPOSITION | MF_GRAYED); // LEVELS...
	EnableMenuItem(cur_menu, 9, MF_BYPOSITION | MF_GRAYED); // Recording>
}

// Display menu for title/high score screen
void show_main_menu()
{
	HMENU main_menu;
	HMENU cur_menu;

	main_menu = GetMenu(g_hWnd);
	EnableMenuItem(main_menu, ID_GAME_ABORT, MF_BYCOMMAND | MF_GRAYED);
	EnableMenuItem(main_menu, ID_GAME_PAUSE, MF_BYCOMMAND | MF_GRAYED);
	EnableMenuItem(main_menu, ID_GAME_START, MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(main_menu, ID_VIEW_VGAGRAPHICS, MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(main_menu, ID_VIEW_CGAGRAPHICS, MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(main_menu, ID_RECORDING_PLAY, MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(main_menu, ID_CONFIGURATION_CONTROLS_KEYBOARD_REDEFINEKEYS, MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(main_menu, ID_CONFIGURATION_CONTROLS_KEYBOARD_REDEFINEALLKEYS, MF_BYCOMMAND | MF_ENABLED);

	cur_menu = GetSubMenu(main_menu, 0);
	EnableMenuItem(cur_menu, 5, MF_BYPOSITION | MF_ENABLED); // MODE>
	EnableMenuItem(cur_menu, 6, MF_BYPOSITION | MF_ENABLED); // PLAYERS>
	EnableMenuItem(cur_menu, 7, MF_BYPOSITION | MF_ENABLED); // LEVEL...
	EnableMenuItem(cur_menu, 9, MF_BYPOSITION | MF_ENABLED); // Recordings>

	cur_menu = GetSubMenu(main_menu, 1);
}

void refresh_screen_info()
{
	/* called whenever the user switches the number of players or the game type */
	LoadGameScores();
	DrawHighScores();
	clear_nplayerlines();
	DrawGameMode();
}

LRESULT CALLBACK WndProc(HWND g_hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HRESULT hRet;
	//BYTE clear_keyboard_state[256]; unreferenced

	switch (message)
	{
	case WM_KEYDOWN:
		//if (wParam!=VK_SHIFT && wParam!=VK_CONTROL)
		//{
		kb_buffer = wParam;
		//}
		if (wParam == VK_MENU && !g_bWindowed)
			SetMenu(g_hWnd, hMenu);
		return 0;

	case WM_SYSKEYDOWN:
		switch (wParam)
		{
			/*
				case VK_RETURN:
				  if (g_IsUsingDirectDraw)
					toggle_screen_mode();
				  return 0;
			*/
		case VK_MENU:
			if (!g_bWindowed)
				SetMenu(g_hWnd, hMenu);
		}
		break;

	case WM_SYSKEYUP:
		switch (wParam)
		{
		case VK_F10:
			kb_buffer = VK_F10;
			return 0;
		}
		break;

		/************************************/
		/* menu commands                    */
		/************************************/
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case VK_F10:
			kb_buffer = VK_F10;
		case ID_GAME_EXIT:
			SendMessage(g_hWnd, WM_CLOSE, 0, 0L);
			break;
		case ID_VIEW_FULLSCREEN:
			toggle_screen_mode();
			break;
		case ID_GAME_PLAYERS_ONE:
			g_PlayersCount = 1;
			g_DiggersCount = 1;
			refresh_screen_info();
			break;
		case ID_GAME_PLAYERS_TWO:
			g_PlayersCount = 2;
			g_DiggersCount = 1;
			g_IsGauntletMode = FALSE;
			refresh_screen_info();
			break;
		case ID_GAME_PLAYERS_TWOSIMULTANEOUS:
			g_PlayersCount = 1;
			g_DiggersCount = 2;
			refresh_screen_info();
			break;
		case ID_GAME_MODE_NORMAL:
			g_IsGauntletMode = FALSE;
			g_IsTimeout = FALSE;  /* must do this in case the previous game was a guantlet game */
			refresh_screen_info();
			break;
		case ID_GAME_MODE_GAUNTLET:
			g_IsGauntletMode = TRUE;
			g_PlayersCount = 1;
			if (!g_OptionGauntletTimeInSeconds)
				g_OptionGauntletTimeInSeconds = 120;
			refresh_screen_info();
			if (do_dialog_box(g_hInstance,
				MAKEINTRESOURCE(IDD_DIALOG_GAUNTLET_SETTINGS),
				g_hWnd, (DLGPROC)gauntlet_settings_dialog_proc))
				WriteINIInt(INI_GAME_SETTINGS, "GauntletTime", g_OptionGauntletTimeInSeconds, g_DiggerIniPath);
			break;
		case ID_GAME_LEVEL:
			if (do_dialog_box(g_hInstance,
				MAKEINTRESOURCE(IDD_DIALOG_LEVELS),
				g_hWnd, (DLGPROC)levels_dialog_proc))
			{
				g_StartingLevel = g_StartingLevel;
			}
			break;
		case ID_GAME_START:
			start = TRUE;
			break;
		case ID_GAME_ABORT:
			kb_buffer = 0;
			g_IsGameCycleEnd = TRUE;
			break;
		case ID_GAME_PAUSE:
			if (!g_IsPaused)
				g_IsPaused = TRUE;
			else
				kb_buffer = 1;  // any key will cause game to continue
			  //kb_buffer=g_KeyboardKeysSettings[16][0];
			break;
		case ID_SOUND_SOUNDS_PLAY:
			soundflag = !soundflag;
			soundlevdoneflag = FALSE;
			break;
		case ID_SOUND_MUSIC_PLAY:
			musicflag = !musicflag;
			break;
		case ID_CONFIGURATION_SPEED_DEFAULT:
			g_GameFrameTime = 80000L;
			break;
		case ID_SPEED_FASTER:
			if (g_GameFrameTime > 10000L)
				g_GameFrameTime -= 10000L;
			break;
		case ID_SPEED_SLOWER:
			g_GameFrameTime += 10000L;
			break;
		case ID_SOUND_SETTINGS:
			if (do_dialog_box(g_hInstance,
				MAKEINTRESOURCE(IDD_DIALOG_SOUND_SETTINGS),
				g_hWnd, (DLGPROC)sound_settings_dialog_proc))
			{
				WriteINIBool(INI_SOUND_SETTINGS, "SoundOn", soundflag, g_DiggerIniPath);
				WriteINIInt(INI_SOUND_SETTINGS, "SoundVolume", get_sound_volume(), g_DiggerIniPath);
			}
			break;
		case ID_HELP_ABOUTDIGGER:
			do_dialog_box(g_hInstance, MAKEINTRESOURCE(IDD_DIALOG_ABOUT), g_hWnd,
				(DLGPROC)help_about_dialog_proc);
			break;
		case ID_CONFIGURATION_GRAPHICS_ASYNCHRONOUS:
			use_async_screen_updates = !use_async_screen_updates;
			break;
		case ID_CONFIGURATION_CONTROLS_KEYBOARD_REDEFINEKEYS:
			GraphicsClear();
			redefkeyb(FALSE);
			reset_main_menu_screen = TRUE;
			break;
		case ID_CONFIGURATION_CONTROLS_KEYBOARD_REDEFINEALLKEYS:
			GraphicsClear();
			redefkeyb(TRUE);
			reset_main_menu_screen = TRUE;
			break;
		case ID_CONFIGURATION_SHOWSETTINGSDIALOG:
			if (g_IsStarted)
				g_IsPaused = TRUE;
			create_config_window();
			break;
		case ID_VIEW_VGAGRAPHICS:
			change_graphics_routines(VIDEO_MODE_VGA_16);
			break;

		case ID_VIEW_CGAGRAPHICS:
			change_graphics_routines(VIDEO_MODE_CGA);
			break;
		case ID_RECORDING_PLAY:
			if (get_open_save_filename(OPEN, "Open Recorded Game", "Digger Record Files (*.drf)\0*.DRF\0All Files (*.*)\0*.*\0", "DRF", drf_filename))
			{
				// make a copy of the current level info, so that it can be restored after the DLF playback is finished
				memcpy(g_BackupLevelsData, g_LevelsData, 8 * MHEIGHT*MWIDTH);
				strcpy(backup_levfname, g_LevelFileName);
				backup_bonusscore = g_BonusScore;
				backup_levfflag = g_LevelFileFlag;

				g_IsStarted = FALSE;
				openplay(drf_filename);
				g_IsStarted = FALSE;

				// restore level data after DLF playback is finished
				memcpy(g_LevelsData, g_BackupLevelsData, 8 * MHEIGHT*MWIDTH);
				strcpy(g_LevelFileName, backup_levfname);
				g_BonusScore = backup_bonusscore;
				g_LevelFileFlag = backup_levfflag;

				reset_main_menu_screen = TRUE;
			}
			break;
		case ID_RECORDING_SAVE:
			if (g_IsGameRecordAvailable)
				RecordSaveFileDialog();
			break;
		case ID_RECORDING_INSTANTREPLAY:
			if (g_IsGameRecordAvailable)
			{
				drf_filename[0] = '\0';
				RecordSaveFileDialog();
				if (drf_filename[0] != '\0')
				{
					g_IsStarted = FALSE;
					openplay(drf_filename);
					g_IsStarted = FALSE;
					reset_main_menu_screen = TRUE;
				}
			}
			break;
		}
		return 0;

	case WM_MENUSELECT:
		refresh_menu_items();   /* it isn't neccessary to do this for every WM_MENUSELECT...  Should only do this when the menu is first activated. */
		pause_windows_sound_playback();
		if (!g_bWindowed)
		{
			if (!(((UINT)HIWORD(wParam)) == 0xFFFF && lParam == (LPARAM)NULL))
			{
				if (!GetCursor())
				{
					if (!g_bWindowed)
						IDirectDrawSurface_SetPalette(g_pDDSPrimary, NULL);  // restore default palette, so that the colors of menus and dialog boxes are correct.
					SetMenu(g_hWnd, GetMenu(g_hWnd));
					show_mouse_cursor();
				}
				break;
			}
			else
			{
				if (preferred_video_mode.height <= 400)
					SetMenu(g_hWnd, NULL);
				hide_mouse_cursor();
				resume_windows_sound_playback();
				if (!g_bWindowed)
					gpal(cur_palette);
				/* repaint the window once the menu is deactivated */
			}
		}
		else
			break;

	case WM_PAINT:
		if (g_bReady)
		{
			if (g_bWindowed || preferred_video_mode.bpp > 8)
			{
				blit_to_window_dc(g_hWnd);
			}
			else
			{
				while (TRUE)
				{
					hRet = blit_to_window();
					if (hRet == DDERR_SURFACELOST)
					{
						hRet = restore_surface();
					}
					else
						if (hRet != DDERR_WASSTILLDRAWING)
						{
							if (hRet == DD_OK)
								if (cur_dialog_box != NULL)
								{
									InvalidateRect(cur_dialog_box, NULL, TRUE);
									UpdateWindow(cur_dialog_box);
								}
							break;
						}
				}
			}
		}
		break;

	case WM_MOVE:
		if (g_bActive && g_bReady && g_bWindowed)
		{
			GetWindowRect(g_hWnd, &g_rcWindow);
			GetClientRect(g_hWnd, &g_rcViewport);
			GetClientRect(g_hWnd, &g_rcScreen);
			ClientToScreen(g_hWnd, (POINT*)&g_rcScreen.left);
			ClientToScreen(g_hWnd, (POINT*)&g_rcScreen.right);
		}
		break;

	case WM_SIZE:
		g_WindowSize.cx = LOWORD(lParam);
		g_WindowSize.cy = HIWORD(lParam);
		InvalidateRect(g_hWnd, NULL, FALSE);

		if (SIZE_MAXHIDE == wParam || SIZE_MINIMIZED == wParam)
			g_bActive = FALSE;
		else
			g_bActive = TRUE;
		break;

	case WM_SETCURSOR:
		/* Display the cursor in the window if windowed */
		if (g_bActive && g_bReady && !g_bWindowed)
		{
			SetCursor((HCURSOR)NULL);
			return TRUE;
		}
		break;

	case WM_CLOSE:
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_POWERBROADCAST:
		switch (wParam)
		{
		case PBT_APMQUERYSTANDBY:
		case PBT_APMQUERYSUSPEND:
			return BROADCAST_QUERY_DENY;
		}

	}
	return DefWindowProc(g_hWnd, message, wParam, lParam);

}



/// <summary>
/// This creates the main window, then calls another function to create 
/// the back buffer that is to be used, and set up other graphics stuff.
/// </summary>
/// <param name="hInstance">The instance.</param>
/// <param name="nShowCmd">The show command.</param>
/// <returns>
/// True if it succeeds, false if it fails.
/// </returns>
bool CreateMainWindow(HINSTANCE hInstance, int nShowCmd)
{
	// class for window
	WNDCLASS wndClass;

	ZeroMemory(&g_rcWindow, sizeof(g_rcWindow));
	g_rcWindow.right = VGA_640;
	g_rcWindow.bottom = VGA_400;
	AdjustWindowRectEx(&g_rcWindow, DIGGER_WS_WINDOWED, TRUE, WS_EX_WINDOWEDGE);

	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInstance;
	wndClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wndClass.hCursor = LoadCursor((HINSTANCE)NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MAINMENU);
	wndClass.lpszClassName = "Digger";
	RegisterClass(&wndClass);

	g_hWnd = CreateWindowEx(
		WS_EX_WINDOWEDGE,
		"Digger",
		"Digger",
		DIGGER_WS_WINDOWED,
		(GetSystemMetrics(SM_CXFULLSCREEN) - VGA_640) / 2,
		(GetSystemMetrics(SM_CYFULLSCREEN) - VGA_400) / 2,
		g_rcWindow.right - g_rcWindow.left,
		g_rcWindow.bottom - g_rcWindow.top,
		(HWND)NULL,
		(HMENU)NULL,
		(HINSTANCE)hInstance,
		(void FAR*)NULL);

	g_WindowSize.cx = 0;
	g_WindowSize.cy = 0;

	ShowWindow(g_hWnd, nShowCmd);
	GetWindowRect(g_hWnd, &g_rcWindow);
	GetClientRect(g_hWnd, &g_rcViewport);
	GetClientRect(g_hWnd, &g_rcScreen);
	ClientToScreen(g_hWnd, (POINT*)&g_rcScreen.left);
	ClientToScreen(g_hWnd, (POINT*)&g_rcScreen.right);
	UpdateWindow(g_hWnd);

	// TODO: add error checking
	return TRUE;
}

void windows_finish()
{
	int i;

	shutting_down = TRUE;

	/* save the current settings */
	WriteINIBool(INI_SOUND_SETTINGS, "SoundOn", soundflag, g_DiggerIniPath);
	WriteINIBool(INI_SOUND_SETTINGS, "MusicOn", musicflag, g_DiggerIniPath);
	WriteINIBool(INI_GRAPHICS_SETTINGS, "FullScreen", !g_bWindowed, g_DiggerIniPath);
	WriteINIBool(INI_GRAPHICS_SETTINGS, "Async", use_async_screen_updates, g_DiggerIniPath);
	if (g_DiggersCount > 1)
		WritePrivateProfileString(INI_GAME_SETTINGS, "Players", "2S", g_DiggerIniPath);
	else
		WriteINIInt(INI_GAME_SETTINGS, "Players", g_PlayersCount, g_DiggerIniPath);
	WriteINIBool(INI_GAME_SETTINGS, "GauntletMode", g_IsGauntletMode, g_DiggerIniPath);
	WriteINIInt(INI_GAME_SETTINGS, "Speed", g_GameFrameTime, g_DiggerIniPath);
	WriteINIBool(INI_GRAPHICS_SETTINGS, "CGA", (video_mode == VIDEO_MODE_CGA), g_DiggerIniPath);
	WriteINIInt(INI_GAME_SETTINGS, "StartLevel", g_StartingLevel, g_DiggerIniPath);
	WritePrivateProfileString(INI_GAME_SETTINGS, "LevelFile", g_LevelFileName, g_DiggerIniPath);

	release_sound_card(); /* see if this solves Brandon's NT problem */

	/* destroy variables */
	destroy_back_buffer();
	destroy_sound_buffers();
	for (i = 0; i < param_count; i++)
		free(params[i]);
	release_direct_x();
	if (g_TitleBitmaps[0])
		DeleteObject(g_TitleBitmaps[0]);
	if (g_TitleBitmaps[1])
		DeleteObject(g_TitleBitmaps[1]);
}

/// <summary>
/// Window main. Program entry point.
/// </summary>
/// <param name="hInstance">The instance.</param>
/// <param name="hPrevInstance">The previous instance.</param>
/// <param name="lpCmdLine">The pointer to a command line.</param>
/// <param name="nShowCmd">The show command.</param>
/// <returns>
/// Exit code.
/// </returns>
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	// initialize here instead of globals
	GlobalInit();

	g_hInstance = hInstance;

	// Make a backup copy of the original level data so
	memcpy(g_OriginalLevelsData, g_LevelsData, MAX_LEVELS * MHEIGHT * MWIDTH);

	// 
	g_OriginalBonusScore = g_BonusScore;

	// initialize directx
	InitializeDirectX();

	// suppose we have no sound
	g_IsWaveDeviceAvailable = FALSE;

	// load menu
	hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MAINMENU));

	// create window class and initialize
	CreateMainWindow(hInstance, nShowCmd);

	// initialize graphics
	InitializeGraphics();

	// show title bitmaps
	LoadTitleBitmaps();

	// 
	main_function_started = TRUE;

	// 
	GameEngineInitialize();

	//
	g_bReady = TRUE;
	g_bActive = TRUE;

	// run the game
	GameRun();

	ReleaseResources();

	// return to OS
	return 0;
}

void toggle_screen_mode()
{
	if (g_bActive && g_bReady)
	{
		g_bReady = FALSE;
		g_bWindowed = !g_bWindowed;

		pause_windows_sound_playback();
		ChangeCoopLevel();
		resume_windows_sound_playback();
		g_bReady = TRUE;
		if (g_bWindowed)
		{
			SetMenu(g_hWnd, hMenu);
			GetWindowRect(g_hWnd, &g_rcWindow);
		}
	}
	InvalidateRect(g_hWnd, NULL, TRUE);
	UpdateWindow(g_hWnd);
}

/* This function is called from various places in Digger to allow Windows */
/* to perform its tasks.                                                  */
void ProcessWindowsEvents()
{
	MSG msg;
	//if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE | PM_NOYIELD ))
	//if (PeekMessage(&msg, g_hWnd, 0, 0, PM_REMOVE | PM_NOYIELD ))
	//do
	//{
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE /*| PM_NOYIELD */))
	{
		if (g_hDlg)
			if (IsDialogMessage(g_hDlg, &msg))
				return;
		if (msg.message == WM_QUIT)
		{
			if (main_function_started)
				ReleaseResources();
			else
				windows_finish();
			exit(0);
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	//}
	//while (suspend_game);
}

int do_dialog_box(HANDLE hInstance, LPCTSTR lpTemplate, HWND hWndParent, DLGPROC lpDialogFunc)
{
	int result;

	pause_windows_sound_playback();
	if (!g_bWindowed)
	{
		attach_clipper();
		IDirectDrawSurface_SetPalette(g_pDDSPrimary, NULL);
	}
	SetMenu(g_hWnd, GetMenu(g_hWnd));
	show_mouse_cursor();
	result = DialogBox(hInstance, lpTemplate, hWndParent, lpDialogFunc);
	cur_dialog_box = (HWND)NULL;
	hide_mouse_cursor();
	if (!g_bWindowed)
	{
		release_clipper();
		gpal(cur_palette);
	}
	resume_windows_sound_playback();
	InvalidateRect(NULL, NULL, FALSE);
	//UpdateWindow(g_hWnd);
	return result;
}

void show_mouse_cursor()
{
	POINT cursor_pos;

	SetCursor(LoadCursor((HINSTANCE)NULL, (LPCSTR)IDC_ARROW));
	GetCursorPos(&cursor_pos);
	SetCursorPos(cursor_pos.x - 1, cursor_pos.y - 1);
	SetCursorPos(cursor_pos.x, cursor_pos.y);
}

void hide_mouse_cursor()
{
	SetCursor((HCURSOR)NULL);
}

LRESULT CALLBACK gauntlet_settings_dialog_proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	bool success;
	UINT rval;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		cur_dialog_box = hDlg;
		SetDlgItemInt(hDlg, IDC_EDIT_TIME, g_OptionGauntletTimeInSeconds, FALSE);
		return TRUE;

	case WM_SYSCOMMAND:
		switch (wParam)
		{
		case SC_CLOSE:
			EndDialog(hDlg, FALSE);
			return TRUE;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			return TRUE;
		case IDOK:
			rval = GetDlgItemInt(hDlg, IDC_EDIT_TIME, &success, FALSE);
			if (success)
				g_OptionGauntletTimeInSeconds = rval;
			if (g_OptionGauntletTimeInSeconds > 3599)
				g_OptionGauntletTimeInSeconds = 3599;
			if (g_OptionGauntletTimeInSeconds == 0)
				g_OptionGauntletTimeInSeconds = 120;
			EndDialog(hDlg, TRUE);
			return TRUE;
		}
		break;
	}
	return FALSE;
}


LRESULT CALLBACK sound_settings_dialog_proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LONG cur_volume;
	LONG new_val;
	bool success;
	HWND control;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		cur_dialog_box = hDlg;
		cur_volume = get_sound_volume();
		control = GetDlgItem(hDlg, IDC_SCROLLBAR_SOUND_VOLUME);
		SetScrollRange(control, SB_CTL, 0, 100, TRUE);
		SetScrollPos(control, SB_CTL, (100 - cur_volume), TRUE);
		EnableScrollBar(control, SB_CTL, ESB_ENABLE_BOTH);
		CheckDlgButton(hDlg, IDC_CHECK_PLAY_SOUNDS, soundflag ? 1 : 0);
		CheckDlgButton(hDlg, IDC_CHECK_PLAY_MUSIC, musicflag ? 1 : 0);
		SetDlgItemInt(hDlg, IDC_EDIT_BUFFER_SIZE, (int)(size / 2), FALSE);
		SetDlgItemInt(hDlg, IDC_EDIT_SAMPLE_RATE, get_sound_freq(), FALSE);
		return TRUE;

	case WM_SYSCOMMAND:
		switch (wParam)
		{
		case SC_CLOSE:
			EndDialog(hDlg, FALSE);
			return TRUE;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			return TRUE;
		case IDOK:
			new_val = GetScrollPos(GetDlgItem(hDlg, IDC_SCROLLBAR_SOUND_VOLUME), SB_CTL);
			set_sound_volume(100 - new_val);

			soundflag = IsDlgButtonChecked(hDlg, IDC_CHECK_PLAY_SOUNDS);
			musicflag = IsDlgButtonChecked(hDlg, IDC_CHECK_PLAY_MUSIC);

			soundlevdoneflag = FALSE;   /* prevent game from locking up if you toggle sound (using menu/dialog box) while the 'level-done' tune is g_IsRecordingPlayback */

			new_val = GetDlgItemInt(hDlg, IDC_EDIT_BUFFER_SIZE, &success, FALSE);
			if (new_val > 0)
				WriteINIInt(INI_SOUND_SETTINGS, "BufferSize", new_val, g_DiggerIniPath);
			new_val = GetDlgItemInt(hDlg, IDC_EDIT_SAMPLE_RATE, &success, FALSE);
			if (new_val > 0)
				WriteINIInt(INI_SOUND_SETTINGS, "Rate", new_val, g_DiggerIniPath);
			EndDialog(hDlg, TRUE);
			return TRUE;
		}
		break;

	case WM_VSCROLL:
		control = (HWND)lParam;
		switch ((int)LOWORD(wParam))
		{
		case SB_THUMBPOSITION:
			SetScrollPos(control, SB_CTL, (short int)HIWORD(wParam), TRUE);
			break;
		case SB_LINEUP:
			new_val = GetScrollPos(control, SB_CTL) - 1;
			SetScrollPos(control, SB_CTL, new_val, TRUE);
			break;
		case SB_LINEDOWN:
			new_val = GetScrollPos(control, SB_CTL) + 1;
			SetScrollPos(control, SB_CTL, new_val, TRUE);
			break;
		case SB_PAGEUP:
			new_val = GetScrollPos(control, SB_CTL) - 10;
			SetScrollPos(control, SB_CTL, new_val, TRUE);
			break;
		case SB_PAGEDOWN:
			new_val = GetScrollPos(control, SB_CTL) + 10;
			SetScrollPos(control, SB_CTL, new_val, TRUE);
			break;
		case SB_TOP:
			SetScrollPos(control, SB_CTL, 0, TRUE);
			break;
		case SB_BOTTOM:
			SetScrollPos(control, SB_CTL, 100, TRUE);
			break;
		}
		return TRUE;
	}
	return FALSE;
}

LRESULT CALLBACK help_about_dialog_proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char info_string[200];
	switch (uMsg)
	{
	case WM_INITDIALOG:
		cur_dialog_box = hDlg;
		strcpy(info_string, DIGGER_VERSION);
		strcat(info_string, "\n");
		strcat(info_string, "Win32 version (DirectX optional)");
		SetDlgItemText(hDlg, IDC_STATIC_VERSION, info_string);
		if (g_bWindowed)
		{
			strcpy(info_string, "Running in Windowed Mode\n");
			strcat(info_string, palettized_desktop ? "(256 Colors/Using Palette)\n" : "(HighColor/TrueColor)\n");
		}
		else
		{
			strcpy(info_string, "Running in Full Screen Mode\n");
		}
		strcat(info_string, hDirectDrawInstance ? "DirectDraw detected\n" : "DirectDraw NOT detected\n");
		if (!g_IsUsingDirectDraw)
			strcat(info_string, "Not using DirectDraw");
		else if (g_IsUsingDirectDraw)
			strcat(info_string, "Using DirectDraw");
		SetDlgItemText(hDlg, IDC_GRAPHICS_INFO, info_string);
		strcpy(info_string, g_IsWaveDeviceAvailable ? "Available\n" : "Unavailable\n");
		strcat(info_string, hDirectSoundInstance ? "DirectSound detected" : "DirectSound NOT detected");
		SetDlgItemText(hDlg, IDC_SOUND_INFO, info_string);
		return TRUE;

	case WM_SYSCOMMAND:
		switch (wParam)
		{
		case SC_CLOSE:
			EndDialog(hDlg, FALSE);
			return TRUE;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hDlg, TRUE);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

void InitializeDirectX()
{
	hDirectDrawInstance = LoadLibrary("ddraw.dll");
	hDirectSoundInstance = LoadLibrary("dsound.dll");

	if (hDirectDrawInstance)
	{
		lpDirectDrawCreate = (long(__stdcall *)(struct _GUID *, struct IDirectDraw **, struct IUnknown *)) GetProcAddress(hDirectDrawInstance, "DirectDrawCreate");
		if (lpDirectDrawCreate)
			g_IsUsingDirectDraw = 0;
	}
	else
		g_IsUsingDirectDraw = 0;

	if (hDirectSoundInstance)
	{
		lpDirectSoundCreate = (long(__stdcall *)(LPCGUID, struct IDirectSound **, struct IUnknown *)) GetProcAddress(hDirectSoundInstance, "DirectSoundCreate");
	}
}

void release_direct_x()
{
	if (hDirectDrawInstance)
		FreeLibrary(hDirectDrawInstance);
	if (hDirectSoundInstance)
		FreeLibrary(hDirectSoundInstance);
}

HRESULT WINAPI DirectDrawCreate(GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter)
{
	return (lpDirectDrawCreate)(lpGUID, lplpDD, pUnkOuter);
}

HRESULT WINAPI DirectSoundCreate(LPCGUID pcGuidDevice, LPDIRECTSOUND * lplpDS, LPUNKNOWN pUnkOuter)
{
	return (lpDirectSoundCreate)(pcGuidDevice, lplpDS, pUnkOuter);
}

LRESULT CALLBACK levels_dialog_proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	bool success;
	UINT rval;
	HWND control;
	int i;
	char str[4];
	char dlf_filename[STRBUF_512];

	switch (uMsg)
	{
	case WM_INITDIALOG:
		cur_dialog_box = hDlg;
		SetDlgItemInt(hDlg, IDC_START_LEVEL, g_StartingLevel, FALSE);
		control = GetDlgItem(hDlg, IDC_START_LEVEL);
		for (i = 1; i < 16; i++)
		{
			ComboBox_AddString(control, _itoa(i, str, 10));
		}
		CheckRadioButton(hDlg, IDC_RADIO_USE_BUILT_IN, IDC_RADIO_USE_EXTERNAL, g_LevelFileFlag ? IDC_RADIO_USE_EXTERNAL : IDC_RADIO_USE_BUILT_IN);
		control = GetDlgItem(hDlg, IDC_EDIT_FILENAME);
		Edit_Enable(control, g_LevelFileFlag);
		Edit_SetText(control, g_LevelFileName);
		return TRUE;

	case WM_SYSCOMMAND:
		switch (wParam)
		{
		case SC_CLOSE:
			EndDialog(hDlg, FALSE);
			return TRUE;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			return TRUE;
		case IDOK:
			rval = GetDlgItemInt(hDlg, IDC_START_LEVEL, &success, FALSE);
			if (rval > 0)
				g_StartingLevel = rval;
			if (IsDlgButtonChecked(hDlg, IDC_RADIO_USE_EXTERNAL))
			{
				control = GetDlgItem(hDlg, IDC_EDIT_FILENAME);
				Edit_GetText(control, dlf_filename, STRBUF_512);
				load_level_file(dlf_filename);
			}
			else
			{
				restore_original_level_data();
			}
			refresh_screen_info();
			EndDialog(hDlg, TRUE);
			return TRUE;
		case IDC_BROWSE_FILENAME:
			CheckRadioButton(hDlg, IDC_RADIO_USE_BUILT_IN, IDC_RADIO_USE_EXTERNAL, IDC_RADIO_USE_EXTERNAL);
			control = GetDlgItem(hDlg, IDC_EDIT_FILENAME);
			Edit_GetText(control, dlf_filename, STRBUF_512);
			if (get_open_save_filename(OPEN, "Load Extra Levels", "Digger Level Files (*.dlf)\0*.DLF\0All Files (*.*)\0*.*\0", "DLF", dlf_filename))
				Edit_SetText(control, dlf_filename);
			control = GetDlgItem(hDlg, IDC_EDIT_FILENAME);
			Edit_Enable(control, TRUE);
			SetActiveWindow(hDlg);
			break;
		case IDC_RADIO_USE_EXTERNAL:
			control = GetDlgItem(hDlg, IDC_EDIT_FILENAME);
			Edit_Enable(control, TRUE);
			break;
		case IDC_RADIO_USE_BUILT_IN:
			control = GetDlgItem(hDlg, IDC_EDIT_FILENAME);
			Edit_Enable(control, FALSE);
			break;
		}
		break;
	}
	return FALSE;
}

UINT FAR PASCAL CommonDialogBoxHook(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	/* TODO: This function *should* center the File Open/Save dialog box on the screen*/
	/*       Currently, the dialog box always appears at the top-left corner when in full-screen mode.  */
	/*       Without this Hook function though, the File Open/Save dialog box often appears off-screen. */

	/*
	  RECT        rc;
	  POINT       pt,pt2;

	  if (WM_NOTIFY==iMsg)
	  {
		if ( ((LPOFNOTIFY) lParam)->hdr.code == CDN_INITDONE )
		{
		  SetWindowPos(hDlg, NULL, 100, 100, 0, 0,
					 SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
		  return FALSE;
		}
	  }
	*/
	return FALSE;
}

/* use Windows Common Dialog Box to get a filename for save/open operations */
bool get_open_save_filename(bool save, char* title, char* filter, char* defext, char* filename)
{
	OPENFILENAME ofn;
	char fn[STRBUF_512];
	char dir[STRBUF_512];
	bool result;

	if ((!g_bWindowed) && (!cur_dialog_box))
	{
		attach_clipper();
		IDirectDrawSurface_SetPalette(g_pDDSPrimary, NULL);
	}
	SetMenu(g_hWnd, GetMenu(g_hWnd));
	show_mouse_cursor();
	strcpy(fn, filename);
	GetCurrentDirectory(STRBUF_512 - 1, dir);
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = g_hWnd;
	ofn.hInstance = (HINSTANCE)g_hInstance;
	ofn.lpstrFilter = filter;
	ofn.lpstrCustomFilter = (LPTSTR)NULL;
	ofn.nMaxCustFilter = 0L;
	ofn.nFilterIndex = 1L;
	ofn.lpstrFile = fn;
	ofn.nMaxFile = STRBUF_512 - 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = dir;
	ofn.lpstrTitle = title;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = "drf";
	ofn.lCustData = 0;
	ofn.lpfnHook = CommonDialogBoxHook;
	if (save)
	{
		ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
		if (!g_bWindowed)
			ofn.Flags |= OFN_ENABLEHOOK;
		result = GetSaveFileName(&ofn);
	}
	else
	{
		ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
		if (!g_bWindowed)
			ofn.Flags |= OFN_ENABLEHOOK;
		result = GetOpenFileName(&ofn);
	}
	if ((!g_bWindowed) && (!cur_dialog_box))
	{
		release_clipper();
		gpal(cur_palette);
	}
	resume_windows_sound_playback();
	InvalidateRect(g_hWnd, NULL, FALSE);
	UpdateWindow(g_hWnd);
	if (result)
		strcpy(filename, fn);
	return result;
}

void restore_original_level_data()
{
	memcpy(g_LevelsData, g_OriginalLevelsData, 8 * MHEIGHT*MWIDTH);
	g_BonusScore = g_OriginalBonusScore;
	strcpy(g_LevelFileName, "");
	g_LevelFileFlag = FALSE;
}

/* load a DLF file */
void load_level_file(char* fn)
{
	FILE* levf;
	levf = fopen(fn, "rb");
	if (levf == NULL) {
		strcat(fn, ".DLF");
		levf = fopen(fn, "rb");
	}
	if (levf == NULL)
	{
		g_LevelFileFlag = FALSE;
		restore_original_level_data();
		strcpy(g_LevelFileName, "");
	}
	else
	{
		fread(&g_BonusScore, 2, 1, levf);
		fread(g_LevelsData, 1200, 1, levf);
		fclose(levf);
		strcpy(g_LevelFileName, fn);
		g_LevelFileFlag = TRUE;
	}
}

void init_joystick()
{
	/* use the standard Win32 joystick functions */
	JOYINFO ji;
	if (joyGetNumDevs())
		if (joyGetPos(0, &ji) == JOYERR_NOERROR)
			g_UseJoystick = TRUE;
}

/// <summary>
/// Global initialize.
/// </summary>
void GlobalInit()
{
	// get executable directory
	GetCurrentDirectory(STRBUF_512, g_DiggerDirectory);

	// if directory aqure is successful
	if (strlen(g_DiggerDirectory) > 0)
	{
		// append final '\' if necessary
		if (g_DiggerDirectory[strlen(g_DiggerDirectory) - 1] != '\\')
			strcat(g_DiggerDirectory, "\\");
	}

	// set ini path
	strcpy(g_DiggerIniPath, g_DiggerDirectory);
	strcat(g_DiggerIniPath, "windig.ini");

	// default mode is VGA
	g_DrawSymbolFunc = VgaWrite;
}
