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

#ifdef TRUE
#undef TRUE
#endif

#include <windows.h>
#define DIGGER_WS_WINDOWED WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_BORDER
#define DIGGER_WS_FULLSCREEN WS_EX_TOPMOST
#define SAVE TRUE
#define OPEN FALSE

extern HWND g_hWnd;
extern SIZE g_WindowSize;
extern HINSTANCE g_hInstance;
extern bool shutting_down;
extern bool suspend_game;

void ProcessWindowsEvents();
void windows_finish();
void refresh_menu_items();
void show_game_menu();
void show_main_menu();
void remove_menu();

void pause_windows_sound_playback();
void resume_windows_sound_playback();
void init_joystick();

void show_mouse_cursor();
void hide_mouse_cursor();

int do_dialog_box(HANDLE hInstance, LPCTSTR lpTemplate, HWND hWndParent,
	DLGPROC lpDialogFunc);
bool get_open_save_filename(bool save, char* title, char* filter, char* defext, char filename[]);
HRESULT TerminateOnFatalError(HRESULT hRet, LPCTSTR szError);
void load_level_file(char* fn);
void restore_original_level_data();

extern bool use_performance_counter;
extern _int64 performance_frequency;
extern void windows_init_sound();

void InitializeDirectX();
void release_direct_x();

extern HINSTANCE g_hInstance;
extern bool g_bActive;
extern bool reset_main_menu_screen;
extern char drf_filename[STRBUF_512];

enum CFG_AUDIO_SETTING_OUTPUT_DEVICE_ENUM
{
	disabled,
	direct_sound,
	wave_out
};

extern HWND cur_dialog_box;
bool key_pressed(int function, bool check_current_state_only);
int translate_key_code_to_function(int code);
extern int command_buffer;
short getcommand();
bool cmdhit();
void add_mapped_key(int idx, int key_code);
extern char g_DiggerDirectory[];   // contains the startup directory
extern char g_DiggerIniPath[];

/// <summary>
/// Global initialize.
/// </summary>
void GlobalInit();

/// <summary>
/// Global shutdown, releases allocated memory.
/// </summary>
void GlobalRelease();