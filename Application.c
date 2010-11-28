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
//PBYTE temp;//for testing out Create Frame
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
	DL_STATE				= -1;
    pwd->bConnected         = FALSE;
    CHAR_WIDTH              = tm.tmAveCharWidth;
    CHAR_HEIGHT             = tm.tmHeight;
    CUR_FG_COLOR            = 7;
    WINDOW_BOTTOM           = LINES_PER_SCRN -1;
	pwd->wordWrap			= FALSE;
	pwd->relOrigin			= FALSE;
	pwd->PTFBuffHead        = NULL;
	pwd->PTFBuffTail        = NULL;
	pwd->FTPBuffHead        = NULL;
	pwd->FTPBuffTail        = NULL;
    pwd->FTPQueueSize       = 0;
    pwd->PTFQueueSize       = 0;
	pwd->NumOfFrames        = 0;
	pwd->NumOfReads         = 0;
    pwd->pReadBufHead       = NULL;
    pwd->pReadBufTail       = NULL;
    pwd->bDebug             = FALSE;

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
    
	//pwd->cc.dwSize = sizeof(COMMCONFIG);
    Connect(hWnd);
    //GetCommConfig(pwd->hPort, &pwd->cc, &pwd->cc.dwSize);
    Disconnect(hWnd);
    //FillMemory(&pwd->cc.dcb, sizeof(DCB), 0);
    //pwd->cc.dcb.DCBlength = sizeof(DCB);
    //BuildCommDCB((LPCWSTR)"96,N,8,1", &pwd->cc.dcb);
	

	//create tables for crc
	crcInit();
/*
	//testing out CreateFrame
	pwd->TxSequenceNumber=1;
	temp = (PBYTE) calloc(2*256+1,sizeof(BYTE));
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
	COMMCONFIG	cc		= {0};
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);
    
    switch (LOWORD(wParam)) {
                
        case IDM_CONNECT:       

            if(OpenFileReceive(hWnd)) {
				Connect(hWnd);
				ShowWindow(pwd->hDlgStats, SW_NORMAL);
				ShowWindow(pwd->hDlgDebug, SW_NORMAL);
			}
            return;

        case IDM_DISCONNECT:
            Disconnect(hWnd);
			CloseFileReceive(hWnd);
			CloseFileTransmit(hWnd);
            return;

        case IDM_EXIT:
            PostMessage(hWnd, WM_DESTROY, 0, 0);
            return;

        case IDM_COM1:  
        case IDM_COM2:
        case IDM_COM3:
        case IDM_COM4:
        case IDM_COM5:
        case IDM_COM6:
        case IDM_COM7:
        case IDM_COM8:
        case IDM_COM9:
			SelectPort(hWnd, LOWORD(wParam));
			return;

        case IDM_COMMSET:
			cc.dwSize	= sizeof(COMMCONFIG);
			cc.wVersion = 1;
			GetCommConfig(pwd->hPort, &cc, &cc.dwSize);

            if (!CommConfigDialog(pwd->lpszCommName, hWnd, &cc)) {
                DISPLAY_ERROR("The comm settings dialogue failed.\nThis port may not exist");
            }
		    return;

		case IDM_STATISTICS:       
			ShowWindow(pwd->hDlgStats, SW_NORMAL);
            return;
		case IDM_DEBUGGER:       
			ShowWindow(pwd->hDlgDebug, SW_NORMAL);
            return;

		case ID_OPEN_RECEIVEFILE:
			OpenFileReceive(hWnd);
			//PostMessage(hWnd, WM_EMPTYPTFBUF, 0, 0);
			return;
		case ID_OPEN_TRANSMITFILE:
			OpenFileTransmit(hWnd);
			return;
		
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
    CHAR temp1[10]= "Frame";
    CHAR temp2[15]= "Frame Length";
	CHAR temp3[5]= "CRC";
	DWORD i;

    MoveCursor( hWnd, 1, 1, FALSE);
    for(i=0;i<10;i++){
        UpdateDisplayBuf(hWnd,temp1[i]);
    }
    MoveCursor( hWnd, 12, 1, FALSE);
    for(i=0;i<15;i++){
        UpdateDisplayBuf(hWnd,temp2[i]);
    }
	MoveCursor(hWnd, 29, 1, FALSE);
	for(i = 0; i<5; i++){
		UpdateDisplayBuf(hWnd,temp3[i]);
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
	static FLOAT totalTime = 0;
	FLOAT dRate, uRate, tRate, edRate, euRate, etRate;
	TCHAR text[20];
	PWNDDATA	pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);
	
	totalTime += TIME_LENGTH;
	_stprintf(text, _T("%d"), NUM_FILES);
	SetDlgItemText(pwd->hDlgStats, IDC_FILESUPLOADED, text);

	_stprintf(text, _T("%d"), SENT_ACK);
	SetDlgItemText(pwd->hDlgStats, IDC_ACKSENT, text);

	_stprintf(text, _T("%d"), REC_ACK);
	SetDlgItemText(pwd->hDlgStats, IDC_ACKRECEIVED, text);

	_stprintf(text, _T("%d"), SENT_EOT);
	SetDlgItemText(pwd->hDlgStats, IDC_EOTSENT, text);

	_stprintf(text, _T("%d"), REC_EOT);
	SetDlgItemText(pwd->hDlgStats, IDC_EOTRECEIVED, text);

	_stprintf(text, _T("%d"), SENT_RVI);
	SetDlgItemText(pwd->hDlgStats, IDC_RVISENT, text);

	_stprintf(text, _T("%d"), REC_RVI);
	SetDlgItemText(pwd->hDlgStats, IDC_RVIRECEIVED, text);

	dRate = (DOWN_FRAMES * 1024 * 8) / totalTime;
	uRate = (UP_FRAMES * 1024 * 8) / totalTime;
	tRate = dRate + uRate;
	
	_stprintf(text, _T("%.2f"), dRate);
	SetDlgItemText(pwd->hDlgStats, IDC_DRATE, text);

	_stprintf(text, _T("%.2f"), uRate);
	SetDlgItemText(pwd->hDlgStats, IDC_URATE, text);

	_stprintf(text, _T("%.2f"), tRate);
	SetDlgItemText(pwd->hDlgStats, IDC_TRATE, text);

	edRate = (DOWN_FRAMES_ACKD * 1019 * 8) / totalTime;
	euRate = (UP_FRAMES_ACKD * 1019 * 8) / totalTime;
	etRate = edRate + euRate;

	_stprintf(text, _T("%.2f"), edRate);
	SetDlgItemText(pwd->hDlgStats, IDC_EDRATE, text);

	_stprintf(text, _T("%.2f"), euRate);
	SetDlgItemText(pwd->hDlgStats, IDC_EURATE, text);

	_stprintf(text, _T("%.2f"), etRate);
	SetDlgItemText(pwd->hDlgStats, IDC_ETRATE, text);
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
	PWNDDATA	pwd = (PWNDDATA) GetWindowLongPtr(GetParent(hDlg), 0);
	
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
        {
			case IDC_CLEAR_STATS:
				NUM_FILES = 0;
				SENT_ACK = 0;
				REC_ACK = 0;
				SENT_EOT = 0;
				REC_EOT = 0;
				SENT_RVI = 0;
				REC_RVI = 0;
				DOWN_FRAMES = 0;
				UP_FRAMES = 0;
				DOWN_FRAMES_ACKD = 0;
				UP_FRAMES_ACKD = 0;
				return TRUE;
		}
		return FALSE;
	case WM_CLOSE:
		ShowWindow(hDlg, SW_HIDE);
		return TRUE;
	}
	return FALSE;
}

BOOL CALLBACK Debug (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	static HBITMAP	bmp[16];
	static UINT		i = 0;
	HINSTANCE		hInst = (HINSTANCE)GetWindowLong(GetParent(hDlg), GWL_HINSTANCE);
	PWNDDATA		pwd = (PWNDDATA) GetWindowLongPtr(GetParent(hDlg), 0);

	switch (message)
	{
	case WM_INITDIALOG:
		bmp[0] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_IDLE));
		bmp[1] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_IDLE2));
		bmp[2] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_R1));
		bmp[3] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_R1_2));
		bmp[4] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_R2));
		bmp[5] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_R2_2));
		bmp[6] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_R3));
		bmp[7] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_R3_2));
		bmp[8] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_R4));
		bmp[9] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_R4_2));
		bmp[10] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_T1));
		bmp[11] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_T1_2));
		bmp[12] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_T2));
		bmp[13] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_T2_2));
		bmp[14] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_T3));
		bmp[15] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_T3_2));
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
        {
            case IDC_DEBUGSTART:
                if (pwd->bDebug) {
                    pwd->bDebug = FALSE;
                    SetDlgItemText(pwd->hDlgDebug, IDC_DEBUGSTART,
                               TEXT("Start Debugging"));
                } else {
                    pwd->bDebug = TRUE;
                    SetDlgItemText(pwd->hDlgDebug, IDC_DEBUGSTART,
                               TEXT("Stop Debugging"));
                }
                return TRUE;

			case IDC_BUTTONENQ:
                SendDebugCtrlChar(GetParent(hDlg), ENQ, TEXT("enqPushed"));
				return TRUE;
			
            case IDC_BUTTONACK:
                SendDebugCtrlChar(GetParent(hDlg), ACK, TEXT("ackPushed"));
				return TRUE;
			
            case IDC_BUTTONRVI:
                SendDebugCtrlChar(GetParent(hDlg), RVI, TEXT("rviPushed"));
				return TRUE;
			
            case IDC_BUTTONEOT:
                SendDebugCtrlChar(GetParent(hDlg), EOT, TEXT("eotPushed"));
                return TRUE;
			
            case IDC_BUTTONF1:
				MakeDebugFrameOne(GetParent(hDlg));
                ProcessWrite(GetParent(hDlg), 
                             (PBYTE) RemoveFromFrameQueue(&pwd->FTPBuffHead, 1), 
                             1);
                pwd->FTPQueueSize--;
                SetEvent(CreateEvent(NULL, FALSE, FALSE, TEXT("f1Pushed")));
				return TRUE;
			
            case IDC_BUTTONF2:
                //pCtrlFrame[CTRL_CHAR_INDEX] = ENQ;
                //ProcessWrite(GetParent(hDlg), pCtrlFrame, CTRL_FRAME_SIZE);
                //SetEvent(CreateEvent(NULL, FALSE, FALSE, TEXT("f2Pushed")));
				MakeDebugFrameTwo( GetParent(hDlg));
				return TRUE;
		}
		return FALSE;
	
	case WM_USER:
		SendDlgItemMessage(pwd->hDlgDebug, IDC_STATIC_IDLE, STM_SETIMAGE, IMAGE_BITMAP, (WPARAM)bmp[0]);
		SendDlgItemMessage(pwd->hDlgDebug, IDC_STATIC_R1, STM_SETIMAGE, IMAGE_BITMAP, (WPARAM)bmp[2]);
		SendDlgItemMessage(pwd->hDlgDebug, IDC_STATIC_R2, STM_SETIMAGE, IMAGE_BITMAP, (WPARAM)bmp[4]);
		SendDlgItemMessage(pwd->hDlgDebug, IDC_STATIC_R3, STM_SETIMAGE, IMAGE_BITMAP, (WPARAM)bmp[6]);
		SendDlgItemMessage(pwd->hDlgDebug, IDC_STATIC_R4, STM_SETIMAGE, IMAGE_BITMAP, (WPARAM)bmp[8]);
		SendDlgItemMessage(pwd->hDlgDebug, IDC_STATIC_T1, STM_SETIMAGE, IMAGE_BITMAP, (WPARAM)bmp[10]);
		SendDlgItemMessage(pwd->hDlgDebug, IDC_STATIC_T2, STM_SETIMAGE, IMAGE_BITMAP, (WPARAM)bmp[12]);
		SendDlgItemMessage(pwd->hDlgDebug, IDC_STATIC_T3, STM_SETIMAGE, IMAGE_BITMAP, (WPARAM)bmp[14]);
		switch (DL_STATE) {
			case STATE_IDLE:
				SendDlgItemMessage(pwd->hDlgDebug, IDC_STATIC_IDLE, STM_SETIMAGE, IMAGE_BITMAP, (WPARAM)bmp[1]);
				break;
			case STATE_R1:
				SendDlgItemMessage(pwd->hDlgDebug, IDC_STATIC_R1, STM_SETIMAGE, IMAGE_BITMAP, (WPARAM)bmp[3]);
				break;
			case STATE_R2:
				SendDlgItemMessage(pwd->hDlgDebug, IDC_STATIC_R2, STM_SETIMAGE, IMAGE_BITMAP, (WPARAM)bmp[5]);
				break;
			case STATE_R3:
				SendDlgItemMessage(pwd->hDlgDebug, IDC_STATIC_R3, STM_SETIMAGE, IMAGE_BITMAP, (WPARAM)bmp[7]);
				break;
			case STATE_R4:
				SendDlgItemMessage(pwd->hDlgDebug, IDC_STATIC_R4, STM_SETIMAGE, IMAGE_BITMAP, (WPARAM)bmp[9]);
				break;
			case STATE_T1:
				SendDlgItemMessage(pwd->hDlgDebug, IDC_STATIC_T1, STM_SETIMAGE, IMAGE_BITMAP, (WPARAM)bmp[11]);
				break;
			case STATE_T2:
				SendDlgItemMessage(pwd->hDlgDebug, IDC_STATIC_T2, STM_SETIMAGE, IMAGE_BITMAP, (WPARAM)bmp[13]);
				break;
			case STATE_T3:
				SendDlgItemMessage(pwd->hDlgDebug, IDC_STATIC_T3, STM_SETIMAGE, IMAGE_BITMAP, (WPARAM)bmp[15]);
				break;
		}
		return TRUE;

	case WM_CLOSE:
		ShowWindow(hDlg, SW_HIDE);
		return TRUE;
	}
	return FALSE;
}

/*------------------------------------------------------------------------------
-- FUNCTION:    SendDebugCtrlChar
--
-- DATE:        Nov 24, 2010
--
-- REVISIONS:
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   SendDebugCtrlChar(HWND hWnd, BYTE ctrlChar, LPCWSTR szEventName)
--                      hWnd        - a handle to the window
--                      ctrlChar    - the control character to send
--                      szEventName - the name of the event to trigger
--
-- RETURNS:     VOID.
--
-- NOTES:       Writes the control character, ctrlChar, to the port and signals
--              the event szEventName.
------------------------------------------------------------------------------*/
VOID SendDebugCtrlChar(HWND hWnd, BYTE ctrlChar, LPCWSTR szEventName) {
    static BYTE     pCtrlFrame[CTRL_FRAME_SIZE] = {0}; 
    HANDLE          hEvent = 0;
    
    pCtrlFrame[CTRL_CHAR_INDEX] = ctrlChar;
    ProcessWrite(hWnd, pCtrlFrame, CTRL_FRAME_SIZE);
    hEvent = CreateEvent(NULL, TRUE, FALSE, szEventName);
    SetEvent(hEvent);
    Sleep(0);
    ResetEvent(hEvent);
}

VOID UpdateStatStruct(HWND hWnd, WPARAM stat, LPARAM attribute) {
    PWNDDATA pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);
    
    switch (stat) {

        case STAT_STATE:
            DL_STATE = attribute;
			SendMessage(pwd->hDlgDebug, WM_USER, 0, 0);
            break;
        
        case ACK:
            if (attribute == SENT) {
                SENT_ACK++;
            } else {
                REC_ACK++;
            }
            break;

        case EOT:
            if (attribute == SENT) {
                SENT_EOT++;
            } else {
                REC_EOT++;
            }
            break;
        
        case RVI:
            if (attribute == SENT) {
                SENT_RVI++;
            } else {
                REC_RVI++;
            }
            break;
        
        case STAT_FRAME:
            if (attribute == SENT) {
                UP_FRAMES++;
            } else {
                DOWN_FRAMES++;
            }
            break;

        case STAT_FRAMEACKD:
            if (attribute == SENT) {
				pwd->FTPQueueSize--;
                UP_FRAMES_ACKD++;
            } else {
				pwd->PTFQueueSize++;
                DOWN_FRAMES_ACKD++;
            }
            break;
       
        case STAT_FILE:
            NUM_FILES++;
            break;
    }
	UpdateStats(hWnd);
}
