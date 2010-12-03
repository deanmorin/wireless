/*------------------------------------------------------------------------------
-- SOURCE FILE:     Application.c - Contains all the OSI "application layer"
--                                  functions for the program.
--
-- PROGRAM:     Dean and the Rockets' Wireless Protocol Testing and Evaluation 
--              Facilitator
--
-- FUNCTIONS:
--				BOOL CALLBACK UpdateStats(HWND)
--				BOOL CALLBACK Stats(HWND, UINT, WPARAM, LPARAM)
--              VOID    InitTerminal(HWND)
--              VOID    Paint(HWND)
--              VOID    PerformMenuAction(HWND, WPARAM)
--				VOID	PrintStats(HWND hWnd, DTRINFO dtrInfo)
--              VOID    MakeColumns(VOID)
--				VOID	UpdateStatStruct(HWND, WPARAM, LPARAM)
--
--
-- DATE:        Oct 19, 2010
--
-- REVISIONS:   Nov 06, 2010 - removed SetBell(), added MakeColumns()
--				Dec 02, 2010 - added stats functions
--				
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- NOTES:
-- Contains application level functions for the program. These
-- are the functions that deal with the program display, as well as initializing
-- the program.
------------------------------------------------------------------------------*/
#include "Application.h"

/*------------------------------------------------------------------------------
-- FUNCTION:    InitTerminal
--
-- DATE:        Oct 19, 2010
--
-- REVISIONS:   Dec 02, 2010 - Added a crc initialization
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

    //create tables for crc
    crcInit();

    //print out headers for terminal
    MakeColumns(hWnd);

    
}

/*------------------------------------------------------------------------------
-- FUNCTION:    PerformMenuAction
--
-- DATE:        Oct 19, 2010
--
-- REVISIONS:   Dec 02, 2010 - added a case for opening a transmit file
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
-- REVISIONS:	Daniel Wright - Dec 2, 2010
--					Modified to display columns for wireless protocol.
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
-- REVISIONS:   Dec 02, 2010 - Changed calculations for better representation of
--								data rate.
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
-- NOTES:	Updates the data rate statistics.
--              
--
------------------------------------------------------------------------------*/
VOID UpdateStats(HWND hWnd) {
    static FLOAT totalTime = 0, lastTime = 0, firstTime = 0, overallTime = 0;
    FLOAT thisTime;
    static INT lastdFrames = 0, lastuFrames = 0, lastedFrames = 0, lasteuFrames = 0;
    static INT count = 0;
	PWNDDATA	pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);
	
	pwd->dtrInfo.dRate = 0;
	pwd->dtrInfo.uRate = 0;
	pwd->dtrInfo.edRate = 0;
	pwd->dtrInfo.euRate = 0;
	pwd->dtrInfo.odRate = 0;
	pwd->dtrInfo.ouRate = 0;
	pwd->dtrInfo.oedRate = 0;
	pwd->dtrInfo.oeuRate = 0;

    if (lastTime == 0) {
        lastTime = (FLOAT) (GetTickCount() / 1000.0); 
    }
	if (firstTime == 0) {
		firstTime = (FLOAT) (GetTickCount() / 1000.0); 
	}
	overallTime = (FLOAT) (GetTickCount() / 1000.0) - firstTime; 
    thisTime = (FLOAT) (GetTickCount() / 1000.0);
    totalTime = thisTime - lastTime;
    lastTime = thisTime;

    if (UP_FRAMES != lastuFrames || DOWN_FRAMES != lastdFrames) {
        if (count++ == 0) {
            pwd->dtrInfo.uRate = 0;
            pwd->dtrInfo.dRate = 0;
            pwd->dtrInfo.edRate = 0;
            pwd->dtrInfo.euRate = 0;
        } else if (totalTime != 0) {
            if ((UP_FRAMES - lastuFrames)!= 0) {
                pwd->dtrInfo.uRate = ((UP_FRAMES - lastuFrames) * 1024 * 8) / totalTime;
                lastuFrames = UP_FRAMES;
            }
            if ((DOWN_FRAMES - lastdFrames)!= 0) {
                pwd->dtrInfo.dRate = ((DOWN_FRAMES - lastdFrames) * 1024 * 8) / totalTime;
                lastdFrames = DOWN_FRAMES;
            }
            if ((DOWN_FRAMES_ACKD - lastedFrames)!= 0) {
                pwd->dtrInfo.edRate = ((DOWN_FRAMES_ACKD - lastedFrames) * 1019 * 8) / totalTime;
                lastedFrames = DOWN_FRAMES_ACKD;
            }
            if ((UP_FRAMES_ACKD - lasteuFrames)!= 0) {
                pwd->dtrInfo.euRate = ((UP_FRAMES_ACKD - lasteuFrames) * 1019 * 8) / totalTime;
                lasteuFrames = UP_FRAMES_ACKD;
            }
        }
    }

	pwd->dtrInfo.odRate = (UP_FRAMES * 1024 * 8) / overallTime;
	pwd->dtrInfo.ouRate = (DOWN_FRAMES * 1024 * 8) / overallTime;
	pwd->dtrInfo.oedRate = (DOWN_FRAMES_ACKD * 1024 * 8) / overallTime;
	pwd->dtrInfo.oeuRate = (UP_FRAMES_ACKD * 1024 * 8) / overallTime;
	PrintStats(hWnd);
}

/*------------------------------------------------------------------------------
-- FUNCTION:    PrintStats
--
-- DATE:        Dec 02, 2010
--
-- REVISIONS:   
--
-- DESIGNER: 	Marcel Vangrootheest
--
-- PROGRAMMER:  Marcel Vangrootheest
--
-- INTERFACE:   BOOL CALLBACK PrintStats(HWND hWnd)
--                              hWnd  - handle to the window
--								dRate - download rate
--								uRate - upload rate
--								edRate - effective download rate
--								euRate - effective upload rate
--
-- RETURNS:     VOID
-- 
-- NOTES:	Updates the statistics in the Dialog window.
--              
--
------------------------------------------------------------------------------*/
VOID PrintStats(HWND hWnd) {
    TCHAR text[20];
	INT framesDiscarded = 0, framesResent = 0;
    PWNDDATA	pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

	
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

    _stprintf(text, _T("%d"), UP_FRAMES);
    SetDlgItemText(pwd->hDlgStats, IDC_FRAMESENT, text);

    _stprintf(text, _T("%d"), DOWN_FRAMES);
    SetDlgItemText(pwd->hDlgStats, IDC_FRAMEREC, text);

    _stprintf(text, _T("%.2f"), pwd->dtrInfo.dRate);
    SetDlgItemText(pwd->hDlgStats, IDC_DRATE, text);

    _stprintf(text, _T("%.2f"), pwd->dtrInfo.uRate);
    SetDlgItemText(pwd->hDlgStats, IDC_URATE, text);
    
    _stprintf(text, _T("%.2f"), pwd->dtrInfo.edRate);
    SetDlgItemText(pwd->hDlgStats, IDC_EDRATE, text);

    _stprintf(text, _T("%.2f"), pwd->dtrInfo.euRate);
    SetDlgItemText(pwd->hDlgStats, IDC_EURATE, text);

	_stprintf(text, _T("%.2f"), pwd->dtrInfo.odRate);
    SetDlgItemText(pwd->hDlgStats, IDC_ODRATE, text);

    _stprintf(text, _T("%.2f"), pwd->dtrInfo.ouRate);
    SetDlgItemText(pwd->hDlgStats, IDC_OURATE, text);
    
    _stprintf(text, _T("%.2f"), pwd->dtrInfo.oedRate);
    SetDlgItemText(pwd->hDlgStats, IDC_EODRATE, text);

    _stprintf(text, _T("%.2f"), pwd->dtrInfo.oeuRate);
    SetDlgItemText(pwd->hDlgStats, IDC_EOURATE, text);

    framesDiscarded = DOWN_FRAMES - DOWN_FRAMES_ACKD;
    _stprintf(text, _T("%d"), framesDiscarded);
    SetDlgItemText(pwd->hDlgStats, IDC_FRAME_DISCARDED, text);

	framesResent = UP_FRAMES - UP_FRAMES_ACKD;
    _stprintf(text, _T("%d"), framesResent);
    SetDlgItemText(pwd->hDlgStats, IDC_FRAMES_RESENT, text);
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
				UpdateStats(GetParent(hDlg));
				return TRUE;
		}
		return FALSE;
	case WM_CLOSE:
		ShowWindow(hDlg, SW_HIDE);
		return TRUE;
	}
	return FALSE;
}

/*------------------------------------------------------------------------------
-- FUNCTION:    UpdateStatStruct
--
-- DATE:        Dec 02, 2010
--
-- REVISIONS:   
--				
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   UpdateStatStruct(HWND hWnd, WPARAM stat, LPARAM attribute)
--						hWnd		- the handle to the window
--						stat		- the stat category to update
--						attribute	- info on how to alter the stat
--
-- RETURNS:     VOID.
--
-- NOTES:
--              Updates the stats structure.
------------------------------------------------------------------------------*/
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
