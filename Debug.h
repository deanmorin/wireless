#ifndef DEBUG_H
#define DEBUG_H

#include "FileIO.h"
#include "osi.h"
#include "WndExtra.h"


/*---------------------------Function Prototypes------------------------------*/
BOOL CALLBACK   Debug (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
VOID    MakeDebugFrameOne(HWND hWnd);
VOID    MakeDebugFrameTwo(HWND hWnd);
VOID    MakeDebugFrameThree(HWND hWnd);
VOID    SendDebugCtrlChar(HWND hWnd, BYTE ctrlChar, LPCWSTR szEventName);

#endif
