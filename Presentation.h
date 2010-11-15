#ifndef PRESENTATION_H
#define PRESENTATION_H

#include <stdio.h>
#include "WndExtra.h"

#define TAB_LENGTH  8
#define CLR_UP      -1
#define CLR_DOWN    1
#define CLR_LEFT    -1
#define CLR_RIGHT   1


VOID	EchoTag(HWND hWnd, CHAR* pcToken, DWORD dwTokenLength, CHAR* pcData, 
                DWORD dwDataLength);
VOID    ClearLine(HWND hWnd, UINT cxCoord, UINT cyCoord, INT iDirection);
VOID    ClearScreen(HWND hWnd, UINT cxCoord, UINT cyCoord, INT iDirection);
VOID    FormFeed(HWND hWnd);
VOID    HorizontalTab(HWND hWnd);
VOID    MoveCursor(HWND hWnd, INT cxCoord, INT cyCoord, BOOL bScroll);
VOID    ScrollDown(HWND hWnd);
VOID    ScrollUp(HWND hWnd);
VOID    SetScrollRegion(HWND hWnd, INT cyTop, INT cyBottom); 
VOID    UpdateDisplayBuf(HWND hWnd, CHAR cCharacter);
VOID	ProcessPacket(HWND hWnd, CHAR* pcPacket, DWORD dwLength);

#endif
