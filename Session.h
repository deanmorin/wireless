/*------------------------------------------------------------------------------
-- FILE:			Session.h                 
-- PROGRAM:		Dean and the Rockets' Wireless Protocol Testing and Evaluation 
-- DATE:        Dec 03, 2010
-- DESIGNER:    Dean Morin, Ian Lee, Daniel Wright, Marcel Vangrootheest
------------------------------------------------------------------------------*/
#ifndef SESSION_H
#define SESSION_H

#include "Menu.h"
#include "osi.h"
#include "WndExtra.h"


/*---------------------------Function Prototypes------------------------------*/
BOOL    Connect(HWND hWnd);
VOID    Disconnect(HWND hWnd);
VOID    SelectPort(HWND hWnd, INT iSelected);

#endif
