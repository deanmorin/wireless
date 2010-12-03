/*------------------------------------------------------------------------------
-- FILE:			Debug.h                 
-- PROGRAM:		Dean and the Rockets' Wireless Protocol Testing and Evaluation 
-- DATE:        Dec 03, 2010
-- DESIGNER:    Dean Morin, Ian Lee, Daniel Wright, Marcel Vangrootheest
------------------------------------------------------------------------------*/
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
