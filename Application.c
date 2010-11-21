/*------------------------------------------------------------------------------
-- SOURCE FILE:     Application.c - Contains all the OSI "application layer"
--                                  functions for the RFID reader.
--
-- PROGRAM:     RFID Reader - Enterprise Edition
--
-- FUNCTIONS:
--              VOID    InitTerminal(HWND);
--              VOID    Paint(HWND);
--              VOID    PerformMenuAction(HWND, WPARAM);
--              VOID    MakeColumns(VOID)
--
--
-- DATE:        Oct 19, 2010
--
-- REVISIONS:   Nov 06, 2010
--              Removed SetBell(), added MakeColumns().
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- NOTES:
-- Contains application level functions for the RFID reader. These
-- are the functions that deal with the program display, as well as initializing
-- the program.
------------------------------------------------------------------------------*/

#include "Application.h"

/*------------------------------------------------------------------------------
-- FUNCTION:    InitTerminal
--
-- DATE:        Oct 19, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   VOID InitTerminal(HWND hWnd)
--                          hWnd- the handle to the window
--
-- RETURNS:     VOID.
--
-- NOTES:
--              Initializes the terminal to its default state.
------------------------------------------------------------------------------*/
VOID InitTerminal(HWND hWnd) {
//BYTE* temp;//for testing out Create Frame
    PWNDDATA    pwd         = {0};
    HDC         hdc         = {0};
    COMMCONFIG  cc          = {0};
    TEXTMETRIC  tm          = {0};
    PAINTSTRUCT ps          = {0};
    RECT        windowRect  = {0};
    RECT        clientRect  = {0};
    UINT        i           = 0;
    UINT        j           = 0;
    LONG        lxDiff      = 0;
    LONG        lyDiff      = 0;

    // create PWNDATA struct and stor it as the window extra
    if ((pwd = (PWNDDATA) calloc(1, sizeof(WNDDATA))) == 0) {
        DISPLAY_ERROR("Error allocating memory for WNDDATA structure");
    }
    pwd->lpszCommName   = TEXT("COM3");
    SetWindowLongPtr(hWnd, 0, (LONG_PTR) pwd);

    // get text attributes and store values into the window extra struct
    hdc = GetDC(hWnd);
	pwd->displayBuf.hFont = (HFONT) GetStockObject(OEM_FIXED_FONT);
    SelectObject(hdc, pwd->displayBuf.hFont);
    GetTextMetrics(hdc, &tm);
    ReleaseDC(hWnd, hdc);

    // initialize variables in PWDDATA struct to defaults
    pwd->bConnected         = FALSE;
    pwd->psIncompleteEsc    = NULL;
    CHAR_WIDTH              = tm.tmAveCharWidth;
    CHAR_HEIGHT             = tm.tmHeight;
    CUR_FG_COLOR            = 7;
    WINDOW_BOTTOM           = LINES_PER_SCRN -1;
	pwd->wordWrap			= FALSE;
	pwd->relOrigin			= FALSE;

    pwd->FTPQueueSize = 0;
    pwd->PTFQueueSize = 0;
	pwd->bMoreData = TRUE;
	pwd->NumOfReads = 0;
    
    // initialize a "blank" display buffer
    for (i = 0; i < LINES_PER_SCRN; i++) {
        pwd->displayBuf.rows[i] = (PLINE) calloc(1, sizeof(LINE));
        for (j = 0; j < CHARS_PER_LINE; j++) {
            CHARACTER(j, i).character   = ' ';
            CHARACTER(j, i).fgColor     = 7;
        }
    }
    // set the window size based off of the font size
    GetWindowRect(hWnd, &windowRect);
    GetClientRect(hWnd, &clientRect);

    lxDiff  = (windowRect.right  - windowRect.left) 
            - (clientRect.right  - clientRect.left);
    lyDiff  = (windowRect.bottom - windowRect.top)
            - (clientRect.bottom - clientRect.top);

    MoveWindow(hWnd,
               windowRect.left, windowRect.top,
               CHAR_WIDTH  * CHARS_PER_LINE + PADDING * 2 + lxDiff,
               CHAR_HEIGHT * LINES_PER_SCRN + PADDING * 2 + lyDiff,
               TRUE);

    // set default comm settings
    pwd->cc.dwSize = sizeof(COMMCONFIG);
    Connect(hWnd);
    GetCommConfig(pwd->hPort, &pwd->cc, &pwd->cc.dwSize);
    Disconnect(hWnd);
    FillMemory(&pwd->cc.dcb, sizeof(DCB), 0);
    pwd->cc.dcb.DCBlength = sizeof(DCB);
    BuildCommDCB((LPCWSTR)"96,N,8,1", &pwd->cc.dcb);

	//create tables for crc
	crcInit();
/*
	//testing out CreateFrame
	pwd->TxSequenceNumber=1;
	temp = (BYTE*) calloc(2*256+1,sizeof(BYTE));
	temp[1] = 0x3;
	CreateFrame(hWnd,temp,2*256+1);
*/
    //print out headers for Tokens and Values
    MakeColumns(hWnd);
}

/*------------------------------------------------------------------------------
-- FUNCTION:    PerformMenuAction
--
-- DATE:        Oct 19, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   VOID PerformMenuAction(HWND hWnd, WPARAM wParam)
--                          hWnd    - the handle to the window
--                          wParam  - contains the menu item that was selected
--
-- RETURNS:     VOID.
--
-- NOTES:
--              Responds to a user's selection of a menu item.
------------------------------------------------------------------------------*/
VOID PerformMenuAction(HWND hWnd, WPARAM wParam) {
    
    PWNDDATA    pwd     = NULL;
    DWORD       dwSize  = 0;
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);
    
    switch (LOWORD(wParam)) {
                
        case IDM_CONNECT:       
            Connect(hWnd);
			//OpenFileReceive(hWnd);
            return;

        case IDM_DISCONNECT:
            Disconnect(hWnd);
			CloseFileReceive(hWnd);
			CloseFileTransmit(hWnd);
            return;

        case IDM_EXIT:
            PostMessage(hWnd, WM_DESTROY, 0, 0);
            return;

        case IDM_COM1:  SelectPort(hWnd, IDM_COM1);  return;
        case IDM_COM2:  SelectPort(hWnd, IDM_COM2);  return;
        case IDM_COM3:  SelectPort(hWnd, IDM_COM3);  return;
        case IDM_COM4:  SelectPort(hWnd, IDM_COM4);  return;
        case IDM_COM5:  SelectPort(hWnd, IDM_COM5);  return;
        case IDM_COM6:  SelectPort(hWnd, IDM_COM6);  return;
        case IDM_COM7:  SelectPort(hWnd, IDM_COM7);  return;
        case IDM_COM8:  SelectPort(hWnd, IDM_COM8);  return;
        case IDM_COM9:  SelectPort(hWnd, IDM_COM9);  return;

        case IDM_COMMSET:
            
            if (!CommConfigDialog(pwd->lpszCommName, hWnd, &pwd->cc)) {
                DISPLAY_ERROR("The comm settings dialogue failed.\nThis port may not exist");
            }
		    return;

		case IDM_STATISTICS:       
			ShowWindow(pwd->hDlgStats, SW_NORMAL);
            return;
		case IDM_DEBUGGER:       
			ShowWindow(pwd->hDlgDebug, SW_NORMAL);
            return;
		/*case ID_TRANSMIT_OPENFILE:
			OpenFileTransmit(hWnd);
			return;
		case ID_TRANSMIT_READ:
			ReadFromFile(hWnd);
			return;
		case ID_TRANSMIT_SETFTP:
			SetEvent(CreateEvent(NULL, FALSE, FALSE, TEXT("fillFTPBuffer")));
			return;
		case ID_TRANSMIT_SETPTF:
			SetEvent(CreateEvent(NULL, FALSE, FALSE, TEXT("emptyPTFBuffer")));
			return;*/
        default:
            return;
    }
}

/*------------------------------------------------------------------------------
-- FUNCTION:    Paint
--
-- DATE:        Oct 19, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   Paint(HWND hWnd)
--                          hWnd - the handle to the window
--
-- RETURNS:     VOID.
--
-- NOTES:
--              Repaints the display buffer.
------------------------------------------------------------------------------*/
VOID Paint(HWND hWnd) {
    PLOGFONT	    plf         = NULL;
    PWNDDATA        pwd         = NULL;
    CHAR            a[2]        = {0};
    HDC             hdc         = {0};
    PAINTSTRUCT     ps          = {0};
    UINT            i           = 0;
    UINT            j           = 0;
    UINT            tempfgColor = 0;
    UINT            tempbgColor = 0;
	UINT            tempStyle	= 0;
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    hdc = BeginPaint(hWnd, &ps) ;
	
    SelectObject(hdc, pwd->displayBuf.hFont);

    tempfgColor = CUR_FG_COLOR;
    tempbgColor = CUR_BG_COLOR;
	tempStyle	= CUR_STYLE;

    SetTextColor(hdc, TXT_COLOURS[CUR_FG_COLOR]);
    SetBkColor(hdc, TXT_COLOURS[CUR_BG_COLOR]);
                             
    for (i = 0; i < LINES_PER_SCRN; i++) {
        for (j = 0; j < CHARS_PER_LINE; j++) {
            
            if (CHARACTER(j, i).fgColor != tempfgColor) {
                SetTextColor(hdc, TXT_COLOURS[CHARACTER(j, i).fgColor]);
                tempfgColor = CHARACTER(j, i).fgColor;
            }
            if (CHARACTER(j, i).bgColor != tempbgColor) {
	            SetBkColor(hdc, TXT_COLOURS[CHARACTER(j, i).bgColor]);
                tempbgColor = CHARACTER(j, i).bgColor;
            }

            a[0] = CHARACTER(j, i).character;
            TextOut(hdc, CHAR_WIDTH * j + PADDING, CHAR_HEIGHT * i + PADDING,
                    (LPCWSTR) a, 1);
        }
    }
	
    EndPaint(hWnd, &ps);
}

/*------------------------------------------------------------------------------
-- FUNCTION:    MakeColumns
--
-- DATE:        Nov 6, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Ian Lee
--
-- PROGRAMMER:  Ian Lee
--
-- INTERFACE:   VOID MakeColumns(HWND hWnd)
--                              hWnd - handle to the window
--
--
-- RETURNS:     VOID.
--
-- NOTES:
--              Prints Column Headers "Token" and "Value"
--
------------------------------------------------------------------------------*/
VOID MakeColumns(HWND hWnd){
    CHAR temp1[10]= "Token";
    CHAR temp2[10]= "Value";
	DWORD i;

    MoveCursor( hWnd, 1, 1, FALSE);
    for(i=0;i<10;i++){
        UpdateDisplayBuf(hWnd,temp1[i]);
    }
    MoveCursor( hWnd, 12, 1, FALSE);
    for(i=0;i<10;i++){
        UpdateDisplayBuf(hWnd,temp2[i]);
    }    
}

/*------------------------------------------------------------------------------
-- FUNCTION:    UpdateStats
--
-- DATE:        Nov 20, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:	Marcel Vangrootheest
--
-- PROGRAMMER:  Marcel Vangrootheest
--
-- INTERFACE:   BOOL CALLBACK UpdateStats(HWND hWnd)
--                              hWnd- handle to the window
--
-- RETURNS:     VOID
-- 
-- NOTES:	Updates the statistics box values.
--              
--
------------------------------------------------------------------------------*/
VOID UpdateStats(HWND hWnd) {
	
	TCHAR text[20];
	PWNDDATA	pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);
	_stprintf(text, _T("%d"), ++pwd->statsInfo.numFiles);
	SetDlgItemText(pwd->hDlgStats, IDC_FILESUPLOADED, text);
}

/*------------------------------------------------------------------------------
-- FUNCTION:    Stats
--
-- DATE:        Nov 18, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:	Marcel Vangrootheest
--
-- PROGRAMMER:  Marcel Vangrootheest
--
-- INTERFACE:   BOOL CALLBACK Stats (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
--                              hDlg	- handle to the Dialog
--								message - the message received
--								wParam  - contents vary based on the message
--								lParam  - contents vary based on the message
--
-- RETURNS:     BOOL - returns true if the message was handled.
-- 
-- NOTES:	This is the Dialog process for the Statistics Box.
--              
--
------------------------------------------------------------------------------*/
BOOL CALLBACK Stats (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;
	case WM_CLOSE:
		ShowWindow(hDlg, SW_HIDE);
		return TRUE;
	}
	return FALSE;
}

BOOL CALLBACK Debug (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;
	case WM_CLOSE:
		ShowWindow(hDlg, SW_HIDE);
		return TRUE;
	}
	return FALSE;
}
