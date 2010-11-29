#ifndef DEBUG_H
#define DEBUG_H

#include "FileIO.h"
#include "osi.h"
#include "WndExtra.h"


/*---------------------------Function Prototypes------------------------------*/
UINT    DebugT1(HWND hWnd, PSTATEINFO psi, PBYTE pReadBuf, DWORD dwLength);
UINT    DebugT3(HWND hWnd, PSTATEINFO psi, PBYTE pReadBuf, DWORD dwLength);
UINT    DebugIDLE(HWND hWnd, PSTATEINFO psi, PBYTE pReadBuf, DWORD dwLength);
UINT    DebugR2(HWND hWnd, PSTATEINFO psi, PBYTE pReadBuf, DWORD dwLength);
VOID    MakeDebugFrameOne(HWND hWnd);
VOID    MakeDebugFrameTwo(HWND hWnd);
VOID    MakeDebugFrameThree(HWND hWnd);

#endif
