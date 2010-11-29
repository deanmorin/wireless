#ifndef FILEIO_H
#define FILEIO_H

#include <Windows.h>
#include "osi.h"
#include "WndExtra.h"


FRAME   CreateFrame(HWND hWnd, PBYTE psBuf, DWORD dwLength);
BOOL 	OpenFileReceive(HWND hWnd);
VOID 	OpenFileTransmit(HWND hWnd);
VOID	CloseFileReceive(HWND hWnd);
VOID	CloseFileTransmit(HWND hWnd);
VOID	WriteToFile(HWND hWnd);
VOID	ReadFromFile(HWND hWnd);
FRAME	CreateNullFrame(HWND hWnd);
#endif