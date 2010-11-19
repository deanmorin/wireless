#ifndef PHYSICAL_H
#define PHYSICAL_H

#include "List.h"
#include "osi.h"
#include "WndExtra.h"

#define READ_BUFSIZE    2048

DWORD WINAPI    PortIOThreadProc(HWND hWnd);
DWORD WINAPI FileIOThreadProc(HWND hWnd);
VOID    ProcessCommError(HANDLE hPort);
VOID    ReadFromPort(HWND hWnd, PSTATEINFO psi, OVERLAPPED ol, DWORD cbInQue);

#endif
