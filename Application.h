#ifndef APPLICATION_H
#define APPLICATION_H

#include "osi.h"
#include "WndExtra.h"
#include <tchar.h>

static const COLORREF TXT_COLOURS[16] = {RGB(0,0,0),		RGB(128,0,0),
								         RGB(0,128,0),		RGB(128,128,0),
								         RGB(0,0,128),		RGB(128,0,128),
								         RGB(0,128,128),	RGB(192,192,192),
								         RGB(128,128,128),  RGB(255,0,0),
								         RGB(0,255,0),		RGB(255,255,0),
								         RGB(0,0,255),		RGB(255,0,255),
								         RGB(0,255,255),	RGB(255,255,255)};
#define TIME_LENGTH	500

VOID			InitTerminal(HWND hWnd);
VOID 			MakeColumns(HWND hWnd);
VOID			Paint(HWND hWnd);
VOID			PerformMenuAction(HWND hWnd, WPARAM wParam);
VOID			SetBell(HWND hWnd, INT iSelected);
BOOL CALLBACK	Stats (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK	Debug (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
VOID			UpdateStats(HWND hWnd);

#endif
