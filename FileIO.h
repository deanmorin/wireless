/*------------------------------------------------------------------------------
-- FILE:			FileIO.h                 
-- PROGRAM:		Dean and the Rockets' Wireless Protocol Testing and Evaluation 
-- DATE:        Dec 03, 2010
-- DESIGNER:    Dean Morin, Ian Lee, Daniel Wright, Marcel Vangrootheest
------------------------------------------------------------------------------*/
#ifndef FILEIO_H
#define FILEIO_H

#include <Windows.h>
#include "osi.h"
#include "WndExtra.h"


/*---------------------------Function Prototypes------------------------------*/
FRAME   CreateFrame(HWND hWnd, PBYTE psBuf, DWORD dwLength);
FRAME	CreateNullFrame(HWND hWnd);
VOID	CloseFileReceive(HWND hWnd);
VOID	CloseFileTransmit(HWND hWnd);
BOOL 	OpenFileReceive(HWND hWnd);
VOID 	OpenFileTransmit(HWND hWnd);
VOID	ReadFromFile(HWND hWnd);
VOID	WriteToFile(HWND hWnd);

#endif
