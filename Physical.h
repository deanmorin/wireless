#ifndef PHYSICAL_H
#define PHYSICAL_H

#include "List.h"
#include "Main.h"
#include "DataLink.h"

#define READ_BUFSIZE    2048
#define WAIT_TIME       100

DWORD WINAPI    PortIOThreadProc(HWND hWnd);
VOID            ProcessCommError(HANDLE hPort);

#endif
