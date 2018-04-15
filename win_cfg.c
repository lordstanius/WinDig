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
#include "win_cfg.h"
#include "win_snd.h"
#include "win_vid.h"
#include "resource.h"
#include "hardware.h"
#include <windowsx.h>
#include <commctrl.h>
#include "Sprite.h" //for now
#include <stdio.h>
#include "Input.h"

/**********************************************************************/
/* some external variables that are read/changed on the config screen */
/**********************************************************************/
extern bool soundflag, musicflag;
extern uint g_GameFrameTime;
extern ushort size;

/**********************************************************************/
/* g_hWndConfigWindow         handle of the main configuration window */
/*    |__ g_hWndTabControl    handle of the tab control               */
/*           |___ g_hDlg       handle of the currently displayed page  */
/**********************************************************************/
HWND g_hWndConfigWindow = NULL;
HWND g_hWndTabControl = NULL;
HWND g_hDlg = NULL;

RECT rcClientRect;
RECT tempRect;

#define WM_FILL_TREEVIEW WM_USER+1

struct KEY_NAME_STRUCT
{
	char *key_name;
	int key_code;
};

char UNKNOWN_KEY_NAME[] = "(unknown key)";

struct KEY_NAME_STRUCT key_list[] = {
									"NumPad 0",VK_NUMPAD0,
									"NumPad 1",VK_NUMPAD1,
									"NumPad 2",VK_NUMPAD2,
									"NumPad 3",VK_NUMPAD3,
									"NumPad 4",VK_NUMPAD4,
									"NumPad 5",VK_NUMPAD5,
									"NumPad 6",VK_NUMPAD6,
									"NumPad 7",VK_NUMPAD7,
									"NumPad 8",VK_NUMPAD8,
									"NumPad 9",VK_NUMPAD9,
									"NumPad *",VK_MULTIPLY,
									"NumPad /",VK_DIVIDE,
									"NumPad +",VK_ADD,
									"NumPad -",VK_SUBTRACT,
									"NumPad .",VK_DECIMAL,
									"F1"      ,VK_F1,
									"F2"      ,VK_F2,
									"F3"      ,VK_F3,
									"F4"      ,VK_F4,
									"F5"      ,VK_F5,
									"F6"      ,VK_F6,
									"F7"      ,VK_F7,
									"F8"      ,VK_F8,
									"F9"      ,VK_F9,
									"F10"     ,VK_F10,
									"F11"     ,VK_F11,
									"F12"     ,VK_F12,
									"F13"     ,VK_F13,
									"F14"     ,VK_F14,
									"F15"     ,VK_F15,
									"F16"     ,VK_F16,
									"F17"     ,VK_F17,
									"F18"     ,VK_F18,
									"F19"     ,VK_F19,
									"F20"     ,VK_F20,
									"F21"     ,VK_F21,
									"F22"     ,VK_F22,
									"F23"     ,VK_F23,
									"F24"     ,VK_F24,
									"ArrowLeft" ,VK_LEFT,
									"ArrowRight",VK_RIGHT,
									"ArrowUp"   ,VK_UP,
									"ArrowDown" ,VK_DOWN,
									"A"         ,'A',
									"B"         ,'B',
									"C"         ,'C',
									"D"         ,'D',
									"E"         ,'E',
									"F"         ,'F',
									"G"         ,'G',
									"H"         ,'H',
									"I"         ,'I',
									"J"         ,'J',
									"K"         ,'K',
									"L"         ,'L',
									"M"         ,'M',
									"N"         ,'N',
									"O"         ,'O',
									"P"         ,'P',
									"Q"         ,'Q',
									"R"         ,'R',
									"S"         ,'S',
									"T"         ,'T',
									"U"         ,'U',
									"V"         ,'V',
									"W"         ,'W',
									"X"         ,'X',
									"Y"         ,'Y',
									"Z"         ,'Z',
									"Tab"       ,VK_TAB,
									"Backspace" ,VK_BACK,
									"Enter"     ,VK_RETURN,
									"CapsLock"  ,VK_CAPITAL,
									"NumLock"   ,VK_NUMLOCK,
									"ScrollLock",VK_SCROLL,
									"Shift"     ,VK_SHIFT,
									"Control"   ,VK_CONTROL,
									"Pause"     ,VK_PAUSE,
									"Space"     ,VK_SPACE,
									"Escape"    ,VK_ESCAPE,
									"1"         ,'1',
									"2"         ,'2',
									"3"         ,'3',
									"4"         ,'4',
									"5"         ,'5',
									"6"         ,'6',
									"7"         ,'7',
									"8"         ,'8',
									"9"         ,'9',
									"Clear"     ,VK_CLEAR,
									"`"          ,0x00C0

};

struct KEY_MAPPING_STRUCT
{
	int key_code;
	struct KEY_MAPPING_STRUCT* next;
};

// 18=save DRF, 19=toggle # players
struct KEY_MAPPING_STRUCT *key_mappings[MAX_KEYBOARD_ACTIONS] = { 0 };

#define SIZEOF_KEY_MAPPINGS_LIST (sizeof(key_mappings)/sizeof(key_mappings[0]))

int cur_key_map_idx = 0;

struct CFG_AUDIO_SETTINGS_STRUCT
{
	bool disabled;
	enum CFG_AUDIO_SETTING_OUTPUT_DEVICE_ENUM output_device;
	LONG volume;
	bool play_sounds;
	bool play_music;
	ushort buffer_size;
	DWORD freq;
};
struct CFG_VIDEO_SETTINGS_STRUCT
{
	bool  use_directdraw;
	bool  hardware_video_mode;  /* video mode to use for the video adapter           */
	bool  graphics_mode;         /* which set of sprites, etc. to use: cga, vga, etc. */
	bool  use_async_updates;     /* FALSE = like original game with flashing sprites  */
	int preferred_video_mode_index;
};
struct CFG_GAME_SETTINGS_STRUCT
{
	uint speed;
};
struct CFG_SETTINGS_STRUCT
{
	struct CFG_AUDIO_SETTINGS_STRUCT audio;
	struct CFG_VIDEO_SETTINGS_STRUCT video;
	struct CFG_GAME_SETTINGS_STRUCT game;
} cfg;

extern void(*gpal)(short pal);

LRESULT CALLBACK ConfigWndProc(HWND hWndConfigWindow, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ConfigTabDlgWndProc(HWND hWndPage, UINT message, WPARAM wParam, LPARAM lParam);
void init_video_config_page(HWND hWndTabControl);
void init_audio_config_page(HWND hWndTabControl);
void init_input_config_page(HWND hWndTabControl);
void hide_video_config_page(HWND hWndTabControl);
void hide_audio_config_page(HWND hWndTabControl);
void hide_input_config_page(HWND hWndConfigWindow);
void apply_new_cfg_settings();
void retrieve_current_cfg_settings();
char* get_key_name(int code);
bool is_key_bound(int key_code, int function);
void load_mapped_key_list(HWND control);
void clear_mapped_keys(int idx);
void store_mapped_keys();

/***********************************/
/* function definitions            */
/***********************************/
bool create_config_window()
{
	WNDCLASS wndClass;
	int width;
	int height;

	retrieve_current_cfg_settings();

	suspend_game = TRUE;
	pause_windows_sound_playback();
	if (!g_bWindowed)
	{
		attach_clipper();
		IDirectDrawSurface_SetPalette(g_pDDSPrimary, NULL);
	}
	SetMenu(g_hWnd, GetMenu(g_hWnd));
	show_mouse_cursor();

	width = 400;
	height = 300;
	rcClientRect.left = 0;
	rcClientRect.top = 0;
	rcClientRect.right = rcClientRect.left + width;
	rcClientRect.bottom = rcClientRect.top + height;
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = ConfigWndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = g_hInstance;
	wndClass.hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wndClass.hCursor = LoadCursor((HINSTANCE)NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = "DiggerInputConfig";
	RegisterClass(&wndClass);
	cur_dialog_box = g_hWndConfigWindow = CreateWindowEx(
		WS_EX_WINDOWEDGE,
		"DiggerInputConfig",
		"Controls",
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_BORDER,
		(GetSystemMetrics(SM_CXFULLSCREEN) - width) / 2,
		(GetSystemMetrics(SM_CYFULLSCREEN) - height) / 2,
		width,
		height,
		(HWND)g_hWnd,
		(HMENU)NULL,
		(HINSTANCE)g_hInstance,
		(void FAR*) NULL);

	ShowWindow(g_hWndConfigWindow, SW_SHOW);
	UpdateWindow(g_hWndConfigWindow);
	EnableWindow(g_hWnd, FALSE);

	return TRUE;
}

LRESULT CALLBACK bind_key_from_list_dialog_proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// bool success; unreferenced
	// UINT rval; unreferenced
	TV_ITEM tree_item;
	TV_INSERTSTRUCT tree_item_struct;

	char strbuf[80];
	HWND control;

	HTREEITEM root_item;
	HTREEITEM new_item;
	int i;
	int state;
	static bool list_drawn = FALSE;
	// struct KEY_MAPPING_STRUCT *next; unreferenced

	switch (uMsg)
	{
	case WM_INITDIALOG:
		list_drawn = FALSE;
		/*  Setting the 'checked' state of an item in TreeView does
			not seem to work within WM_INITDIALOG for some reason,
			so set a flag indicating that TreeView needs to be filled later.
			The list is actually filled after the dialog box is drawn
			on the screen for the first time.
		*/
		return TRUE;

	case WM_PAINT:
		if (!list_drawn)
		{
			list_drawn = TRUE;
			PostMessage(hDlg, WM_FILL_TREEVIEW, 0, 0);
		}
		break;
	case WM_FILL_TREEVIEW:
		control = GetDlgItem(hDlg, IDC_TREE_AVAILABLE_CONTROLS);

		// put "Keyboard" heading in root of tree
		tree_item_struct.hParent = NULL;
		tree_item_struct.hInsertAfter = TVI_LAST;

		tree_item_struct.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE;
		tree_item_struct.item.pszText = "Keyboard";
		tree_item_struct.item.lParam = 0;
		tree_item_struct.item.state = INDEXTOSTATEIMAGEMASK(1);

		tree_item_struct.item.stateMask = TVIS_STATEIMAGEMASK;
		root_item = TreeView_InsertItem(control, &tree_item_struct);

		// add key names from key_list
		tree_item_struct.hParent = root_item;
		for (i = 0; i < (sizeof(key_list) / sizeof(key_list[0])); i++)
		{
			tree_item_struct.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE;
			if (is_key_bound(key_list[i].key_code, cur_key_map_idx))
				tree_item_struct.item.state = INDEXTOSTATEIMAGEMASK(2);
			else
				tree_item_struct.item.state = INDEXTOSTATEIMAGEMASK(1);
			tree_item_struct.item.stateMask = TVIS_STATEIMAGEMASK;
			tree_item_struct.item.pszText = key_list[i].key_name;
			tree_item_struct.item.lParam = key_list[i].key_code;
			new_item = TreeView_InsertItem(control, &tree_item_struct);
		}
		break;

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
			clear_mapped_keys(cur_key_map_idx);

			/* map selected keys to this function */
			control = GetDlgItem(hDlg, IDC_TREE_AVAILABLE_CONTROLS);
			root_item = TreeView_GetRoot(control);
			new_item = TreeView_GetNextItem(control, root_item, TVGN_CHILD);
			while (new_item)
			{
				tree_item.hItem = new_item;
				tree_item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_STATE;
				tree_item.stateMask = TVIS_STATEIMAGEMASK;
				tree_item.pszText = strbuf;
				TreeView_GetItem(control, &tree_item);
				state = tree_item.state >> 12;
				if (state == 2)
				{
					add_mapped_key(cur_key_map_idx, tree_item.lParam);
				}
				new_item = TreeView_GetNextItem(control, new_item, TVGN_NEXT);
			};
			EndDialog(hDlg, TRUE);
			return TRUE;
		}
		break;
	}
	return FALSE;
}


LRESULT CALLBACK ConfigWndProc(HWND hWndConfigWindow, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND control;
	int iPage;                  /* currently displayed page/tab */
	TC_ITEM tc_item;
	memset(&tc_item, 0, sizeof(TC_ITEM));
	tc_item.mask = TCIF_TEXT;
	switch (message)
	{
	case WM_KEYDOWN:
		return DefWindowProc(hWndConfigWindow, message, wParam, lParam);

	case WM_CREATE:
		GetClientRect(hWndConfigWindow, &rcClientRect);
		g_hWndTabControl = CreateWindow(
			WC_TABCONTROL, "",
			WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | WS_TABSTOP | WS_GROUP,
			rcClientRect.left, rcClientRect.top, rcClientRect.right - rcClientRect.left, rcClientRect.bottom - rcClientRect.top - 34,
			hWndConfigWindow, NULL, g_hInstance, NULL
		);
		control = CreateWindow("BUTTON", "OK", BS_DEFPUSHBUTTON | WS_CHILD, rcClientRect.right - 60, rcClientRect.bottom - 32, 60, 30, hWndConfigWindow, (HMENU)IDC_BUTTON_OK, g_hInstance, NULL);
		ShowWindow(control, SW_SHOW);
		tc_item.pszText = "Video";
		TabCtrl_InsertItem(g_hWndTabControl, 0, &tc_item);
		tc_item.pszText = "Audio";
		TabCtrl_InsertItem(g_hWndTabControl, 1, &tc_item);
		tc_item.pszText = "Controls";
		TabCtrl_InsertItem(g_hWndTabControl, 2, &tc_item);
		tc_item.pszText = "System";
		TabCtrl_InsertItem(g_hWndTabControl, 3, &tc_item);
		TabCtrl_SetCurSel(g_hWndTabControl, 0);
		GetClientRect(g_hWndTabControl, &tempRect);
		TabCtrl_AdjustRect(g_hWndTabControl, FALSE, &tempRect);
		init_video_config_page(hWndConfigWindow);
		return 0;

	case WM_NOTIFY:
		switch (((NMHDR FAR *)lParam)->code)
		{
		case TCN_SELCHANGING:    /* current page is about of be removed */
			if (g_hDlg != NULL)
			{
				iPage = TabCtrl_GetCurSel(g_hWndTabControl);
				switch (iPage)
				{
				case 0:
					hide_video_config_page(hWndConfigWindow);
					break;
				case 1:
					hide_audio_config_page(hWndConfigWindow);
					break;
				case 2:
					hide_input_config_page(hWndConfigWindow);
					break;
				}
				if (DestroyWindow(g_hDlg))
					g_hDlg = 0;
			}
			return FALSE;
		case TCN_SELCHANGE:   /* new page is about to be display */
			iPage = TabCtrl_GetCurSel(g_hWndTabControl);
			switch (iPage)
			{
			case 0:
				init_video_config_page(hWndConfigWindow);
				break;
			case 1:
				init_audio_config_page(hWndConfigWindow);
				break;
			case 2:
				init_input_config_page(hWndConfigWindow);
				break;
			}
			return FALSE;
		case NM_SETFOCUS:
			if ((int)wParam == IDC_INVISIBLE_TABSTOP)
			{
				return FALSE;
			}
			break;
		default:
			return FALSE;
		}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BUTTON_CANCEL:
			EndDialog(hWndConfigWindow, FALSE);
			return TRUE;
		case IDC_BUTTON_OK:
			PostMessage(hWndConfigWindow, WM_CLOSE, (WPARAM)0, (LPARAM)0);
			return TRUE;
		}
		break;

	case WM_CLOSE:
		iPage = TabCtrl_GetCurSel(g_hWndTabControl);
		switch (iPage)
		{
		case 0:
			hide_video_config_page(hWndConfigWindow);
			break;
		case 1:
			hide_audio_config_page(hWndConfigWindow);
			break;
		case 2:
			break;
		}
		if (DestroyWindow(g_hDlg))
			g_hDlg = 0;

		apply_new_cfg_settings();
		EnableWindow(g_hWnd, TRUE);
		DestroyWindow(hWndConfigWindow);
		cur_dialog_box = hWndConfigWindow = NULL;
		SetForegroundWindow(g_hWnd);
		suspend_game = FALSE;
		hide_mouse_cursor();
		if (!g_bWindowed)
		{
			release_clipper();
			gpal(cur_palette);
		}
		resume_windows_sound_playback();
		InvalidateRect(g_hWnd, NULL, FALSE);
		UpdateWindow(g_hWnd);

		return 0;
	}
	return DefWindowProc(hWndConfigWindow, message, wParam, lParam);
}

LRESULT CALLBACK ConfigTabDlgWndProc(HWND hWndPage, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND control;

	switch (message)
	{
	case WM_KEYDOWN:
		return DefWindowProc(hWndPage, message, wParam, lParam);
	case WM_SIZE:
		//GetClientRect(g_hWndTabControl,&tempRect);
		//TabCtrl_AdjustRect(g_hWndTabControl,FALSE, &tempRect);
		MoveWindow(g_hDlg, tempRect.left, tempRect.top, tempRect.right - tempRect.left, tempRect.bottom - tempRect.top, FALSE);
		return 0;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BUTTON_BIND_SELECT:
			control = GetDlgItem(hWndPage, IDC_LIST_KEY_ASSIGNMENTS);
			cur_key_map_idx = ListBox_GetCurSel(control);
			if (cur_key_map_idx < 0)
				cur_key_map_idx = 0;
			do_dialog_box(g_hInstance, MAKEINTRESOURCE(IDD_DIALOG_SELECT_KEY),
				hWndPage, (DLGPROC)bind_key_from_list_dialog_proc);
			load_mapped_key_list(GetDlgItem(g_hDlg, IDC_LIST_KEY_ASSIGNMENTS));

			return TRUE;
		}

	}
	return DefWindowProc(hWndPage, message, wParam, lParam);
}

extern bool g_IsStarted;
void init_video_config_page(HWND hWndTabControl)
{
	HWND control;
	char strbuf[40];
	struct FULLSCREEN_VID_MODE_INFO* vmode;

	//RECT tempRect;
	g_hDlg = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_DIALOG_CFG_VIDEO), g_hWndTabControl, ConfigTabDlgWndProc);
	if (g_hDlg)
	{
		//MoveWindow(g_hDlg,0,30,300,200,TRUE);

		control = GetDlgItem(g_hDlg, IDC_COMBO_FULLSCREEN_VIDEO_MODE);
		vmode = supported_video_modes;
		while (vmode != NULL)
		{
			sprintf(strbuf, "%d x %d (%d bit)", vmode->width, vmode->height, vmode->bpp);
			ComboBox_AddString(control, strbuf);
			vmode = vmode->next_mode;
		}
		ComboBox_SetCurSel(control, cfg.video.preferred_video_mode_index - 1);
		control = GetDlgItem(g_hDlg, IDC_COMBO_GAME_GRAPHICS);
		ComboBox_AddString(control, "CGA (Original)");
		ComboBox_AddString(control, "VGA (Digger Remastered)");
		ComboBox_SetCurSel(control, cfg.video.graphics_mode);
		EnableWindow(control, !g_IsStarted);

		control = GetDlgItem(g_hDlg, IDC_CHECK_USE_ASYNC);
		Button_SetCheck(control, cfg.video.use_async_updates ? 1 : 0);

		control = GetDlgItem(g_hDlg, IDC_CHECK_USE_DIRECTDRAW);
		Button_SetCheck(control, cfg.video.use_directdraw ? 1 : 0);

		MoveWindow(g_hDlg, tempRect.left, tempRect.top, tempRect.right - tempRect.left, tempRect.bottom - tempRect.top, TRUE);

		ShowWindow(g_hDlg, SW_SHOW);
		if (GetFocus() != g_hWndTabControl)
			SetFocus(g_hDlg);
	}
	else
		TerminateOnFatalError(0, "init_video_config_page() failed");
}

void init_audio_config_page(HWND hWndTabControl)
{
	HWND control;
	char strbuf[20];
	g_hDlg = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_DIALOG_CFG_AUDIO), g_hWndTabControl, ConfigTabDlgWndProc);
	if (g_hDlg)
	{
		MoveWindow(g_hDlg, 0, 30, 300, 200, TRUE);

		control = GetDlgItem(g_hDlg, IDC_COMBO_AUDIO_DEVICE);
		ComboBox_AddString(control, "[Disabled");
		ComboBox_AddString(control, "Use DirectSound");
		ComboBox_AddString(control, "Use Wave Out");
		ComboBox_SetCurSel(control, cfg.audio.output_device);
		_itoa(cfg.audio.buffer_size, strbuf, 10);
		SetDlgItemText(g_hDlg, IDC_EDIT_BUFFER_SIZE, strbuf);
		_itoa(cfg.audio.freq, strbuf, 10);
		SetDlgItemText(g_hDlg, IDC_EDIT_SAMPLE_RATE, strbuf);

		control = GetDlgItem(g_hDlg, IDC_CHECK_PLAY_MUSIC);
		Button_SetCheck(control, cfg.audio.play_music);
		control = GetDlgItem(g_hDlg, IDC_CHECK_PLAY_SOUNDS);
		Button_SetCheck(control, cfg.audio.play_sounds);

		control = GetDlgItem(g_hDlg, IDC_SLIDER_VOLUME);
		SendMessage(control, TBM_SETRANGEMAX, (WPARAM)FALSE, 100);
		SendMessage(control, TBM_SETRANGEMIN, (WPARAM)FALSE, 0);
		SendMessage(control, TBM_SETPOS, (WPARAM)TRUE, 100 - cfg.audio.volume);

		ShowWindow(g_hDlg, SW_SHOW);
		if (GetFocus() != g_hWndTabControl)
			SetFocus(g_hDlg);
	}
	else
		TerminateOnFatalError(0, "init_audio_config_page() failed");

}

extern char *keynames[];
void load_mapped_key_list(HWND control)
{
	SIZE sz;
	int max_text_width = 0;
	HDC dc;
	int i;
	// int list_size; unreferenced
	struct KEY_MAPPING_STRUCT *next;
	char strbuf[200];

	while (ListBox_GetCount(control))
		ListBox_DeleteString(control, 0);

	dc = GetDC(control);
	for (i = 0; i < 19; i++)
	{
		if (i < 5)
			sprintf(strbuf, "Player1 %s:\t", keynames[i]);
		else
		{
			if (i < 10)
				sprintf(strbuf, "Player2 %s\t", keynames[i]);
			else
				sprintf(strbuf, "%s:\t", keynames[i]);

		}
		if (key_mappings[i])
		{
			strcat(strbuf, get_key_name(key_mappings[i]->key_code));
			next = key_mappings[i]->next;
			while (next)
			{
				strcat(strbuf, ", ");
				strcat(strbuf, get_key_name(next->key_code));
				next = next->next;
			}
		}
		else
			strcat(strbuf, "(no key assigned)");

		GetTextExtentPoint32(dc, strbuf, strlen(strbuf), &sz);
		if (sz.cx > max_text_width)
			max_text_width = sz.cx;
		ListBox_AddString(control, strbuf);
	}
	ReleaseDC(control, dc);
	ListBox_SetHorizontalExtent(control, max_text_width);
}

void init_input_config_page(HWND hWndTabControl)
{
	HWND control;
	// int i; unreferenced
	int tabstops[1] = { 64 };

	//RECT tempRect;
	g_hDlg = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_DIALOG_CFG_INPUT), g_hWndTabControl, ConfigTabDlgWndProc);
	if (g_hDlg)
	{

		MoveWindow(g_hDlg, tempRect.left, tempRect.top, tempRect.right - tempRect.left, tempRect.bottom - tempRect.top, TRUE);

		control = GetDlgItem(g_hDlg, IDC_LIST_KEY_ASSIGNMENTS);
		ListBox_SetTabStops(control, 1, tabstops);

		load_mapped_key_list(control);
		ShowWindow(g_hDlg, SW_SHOW);
		if (GetFocus() != g_hWndTabControl)
			SetFocus(g_hDlg);
	}
	else
		TerminateOnFatalError(0, "init_input_config_page() failed");
}

void hide_video_config_page(HWND hWndTabControl)
{
	HWND control;
	control = GetDlgItem(g_hDlg, IDC_COMBO_FULLSCREEN_VIDEO_MODE);
	cfg.video.hardware_video_mode = (ComboBox_GetCurSel(control) == 1);
	control = GetDlgItem(g_hDlg, IDC_COMBO_GAME_GRAPHICS);
	cfg.video.graphics_mode = ComboBox_GetCurSel(control);
	control = GetDlgItem(g_hDlg, IDC_CHECK_USE_ASYNC);
	cfg.video.use_async_updates = Button_GetCheck(control);
	control = GetDlgItem(g_hDlg, IDC_CHECK_USE_DIRECTDRAW);
	cfg.video.use_directdraw = Button_GetCheck(control);

	control = GetDlgItem(g_hDlg, IDC_COMBO_FULLSCREEN_VIDEO_MODE);
	cfg.video.preferred_video_mode_index = ComboBox_GetCurSel(control) + 1;
}

void hide_audio_config_page(HWND hWndTabControl)
{
	char strbuf[20];
	HWND control;
	if (GetDlgItemText(g_hDlg, IDC_EDIT_BUFFER_SIZE, strbuf, 20))
		cfg.audio.buffer_size = atoi(strbuf);
	if (GetDlgItemText(g_hDlg, IDC_EDIT_SAMPLE_RATE, strbuf, 20))
		cfg.audio.freq = atoi(strbuf);
	control = GetDlgItem(g_hDlg, IDC_COMBO_AUDIO_DEVICE);
	cfg.audio.output_device = ComboBox_GetCurSel(control);
	control = GetDlgItem(g_hDlg, IDC_CHECK_PLAY_MUSIC);
	cfg.audio.play_music = Button_GetCheck(control);
	control = GetDlgItem(g_hDlg, IDC_CHECK_PLAY_SOUNDS);
	cfg.audio.play_sounds = Button_GetCheck(control);
	control = GetDlgItem(g_hDlg, IDC_SLIDER_VOLUME);
	cfg.audio.volume = 100 - SendMessage(control, TBM_GETPOS, 0, 0);
}

void hide_input_config_page(HWND hWndTabControl)
{

}

void retrieve_current_cfg_settings()
{
	struct FULLSCREEN_VID_MODE_INFO* vmode;
	int i, preferred_vid_mode_index;

	cfg.audio.buffer_size = size / 2;
	cfg.audio.disabled = FALSE;
	cfg.audio.freq = get_sound_freq();
	cfg.audio.play_music = musicflag;
	cfg.audio.play_sounds = soundflag;
	cfg.audio.volume = dx_sound_volume;
	cfg.audio.output_device = sound_output_device;

	cfg.game.speed = g_GameFrameTime;

	cfg.video.hardware_video_mode = use_640x480_fullscreen;
	cfg.video.use_async_updates = use_async_screen_updates;
	cfg.video.use_directdraw = g_IsUsingDirectDraw;
	cfg.video.graphics_mode = video_mode;

	preferred_vid_mode_index = 0;
	vmode = supported_video_modes;
	i = 1;
	while (vmode != NULL)
	{
		if (vmode->width == preferred_video_mode.width &&
			vmode->height == preferred_video_mode.height &&
			vmode->bpp == preferred_video_mode.bpp)
			preferred_vid_mode_index = i;
		if (!preferred_vid_mode_index && vmode->width == 640 && vmode->height >= 400)   // if nothing in the list matches the preferred mode, then select a mode 640x400 or 640x480
			preferred_vid_mode_index = i;
		vmode = vmode->next_mode;
		i++;
	}
	cfg.video.preferred_video_mode_index = preferred_vid_mode_index;
}

void apply_new_cfg_settings()
{
	struct FULLSCREEN_VID_MODE_INFO* vmode;
	int i;//,preferred_vid_mode_index; unreferenced

	release_sound_card();
	if ((cfg.audio.buffer_size != size / 2) && (cfg.audio.buffer_size > 0))
	{
		sound_length = size = cfg.audio.buffer_size * 2;
	}
	if ((cfg.audio.freq != g_SoundRate) && (cfg.audio.freq > 0))
	{
		if (cfg.audio.freq > 65000)
		{
			// TODO note, frequency must be less then int16.MaxValue
		}
		g_SoundRate = (ushort)cfg.audio.freq;
	}
	if (cfg.audio.output_device != sound_output_device)
	{
		sound_output_device = cfg.audio.output_device;
	}

	vmode = supported_video_modes;
	for (i = 1; i < cfg.video.preferred_video_mode_index; i++)
	{
		vmode = vmode->next_mode;
	}
	if (vmode->width != preferred_video_mode.width ||
		vmode->height != preferred_video_mode.height ||
		vmode->bpp != preferred_video_mode.bpp)
		set_preferred_vidmode(vmode->width, vmode->height, vmode->bpp);

	if (video_mode != cfg.video.graphics_mode)
		change_graphics_routines(cfg.video.graphics_mode);

	capture_sound_card();
	set_sound_volume(dx_sound_volume = cfg.audio.volume);
	musicflag = cfg.audio.play_music;
	soundflag = cfg.audio.play_sounds;
	store_mapped_keys();
}


char* get_key_name(int code)
{
	int i;

	for (i = 0; i < (sizeof(key_list) / sizeof(key_list[0])); i++)
	{
		if (code == key_list[i].key_code)
			return key_list[i].key_name;
	}
	return UNKNOWN_KEY_NAME;
}

int translate_key_code_to_function(int code)
{
	int i;
	int j;

	for (i = 10; i < MAX_KEYBOARD_ACTIONS; i++)
	{
		//if (is_key_bound(code, i))
		//{
		//    return i + 1;
		//}

		for (j = 0; j < MAX_ACTION_KEYS; j++)
		{
			if (g_KeyboardKeysSettings[i][j] == code)
			{
				return i + 1;
			}
		}
	}

	return 0;
}

bool is_key_bound(int key_code, int function)
{
	struct KEY_MAPPING_STRUCT *cur;

	// get head
	cur = key_mappings[function];

	// search if map found
	while (cur)
	{
		if (cur->key_code == key_code)
		{
			return 1;
		}

		cur = cur->next;
	}

	return 0;
}

/* checks to see if any of the keys mapped to a specific function have been preseed.  */
bool key_pressed(int function, bool check_current_state_only)
{
	struct KEY_MAPPING_STRUCT *cur;
	SHORT keystate;
	cur = key_mappings[function];
	while (cur)
	{
		keystate = GetAsyncKeyState(cur->key_code);
		if (check_current_state_only)
			keystate = keystate & 0x8000;
		if (keystate)
			return TRUE;
		cur = cur->next;
	}
	return FALSE;
}

void clear_mapped_keys(int idx)
{
	struct KEY_MAPPING_STRUCT *cur, *prev;

	/* remove existing keys mapped to this function */
	cur = key_mappings[idx];
	key_mappings[idx] = NULL;
	while (cur)
	{
		prev = cur;
		cur = cur->next;
		free(prev);
	}
}

void add_mapped_key(int idx, int key_code)
{
	struct KEY_MAPPING_STRUCT *cur;

	if (!key_code)  //if not a valid Virtual Key Code then return
		return;

	if (key_mappings[idx])
	{
		cur = key_mappings[idx];
		while (cur->next)
		{
			cur = cur->next;
		}
		cur->next = malloc(sizeof(struct KEY_MAPPING_STRUCT));
		cur = cur->next;
		cur->key_code = key_code;
		cur->next = NULL;
	}
	else
	{
		cur = key_mappings[idx] = malloc(sizeof(struct KEY_MAPPING_STRUCT));
		cur->key_code = key_code;
		cur->next = NULL;
	}

}

void store_mapped_keys()
{
	int i;
	char kbuf[80], vbuf[256];
	char tempnum[32];

	struct KEY_MAPPING_STRUCT *cur;

	for (i = 0; i < SIZEOF_KEY_MAPPINGS_LIST; i++)
	{
		cur = key_mappings[i];
		sprintf(kbuf, "%s%c", keynames[i], (i >= 5 && i < 10) ? '2' : 0);
		vbuf[0] = '\0';
		while (cur)
		{
			strcat(vbuf, _itoa(cur->key_code, tempnum, 10));
			cur = cur->next;
			if (cur)
				strcat(vbuf, "/");
		}
		WritePrivateProfileString(INI_KEY_SETTINGS, kbuf, vbuf, g_DiggerIniPath);
	}
}
