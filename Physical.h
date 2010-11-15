#ifndef PHYSICAL_H
#define PHYSICAL_H
#include "List.h"
#include "Main.h"
#include "DataLink.h"

#define READ_BUFSIZE    2048

DWORD WINAPI    PortIOThreadProc(HWND hWnd);
VOID    ProcessCommError(HANDLE hPort);
//VOID    ReadFromPort(HWND hWnd, PSTATEINFO psi, OVERLAPPED ol, DWORD cbInQue);

#endif
