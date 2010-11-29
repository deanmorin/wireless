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
