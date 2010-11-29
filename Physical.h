#ifndef PHYSICAL_H
#define PHYSICAL_H

#include "List.h"
#include "osi.h"
#include "WndExtra.h"

/*--------------------------------Macros--------------------------------------*/
#define READ_BUFSIZE    4096


/*---------------------------Function Prototypes------------------------------*/
DWORD WINAPI    PortIOThreadProc(HWND hWnd);
DWORD WINAPI	FileIOThreadProc(HWND hWnd);
VOID    InitStateInfo (PSTATEINFO psi);
VOID    ProcessCommError(HANDLE hPort);
VOID    ReadFromPort(HWND hWnd, PSTATEINFO psi, OVERLAPPED ol, DWORD cbInQue);

#endif
