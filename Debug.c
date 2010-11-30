#include "Debug.h"

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
				return TRUE;
			
            case IDC_BUTTONF2:
				MakeDebugFrameTwo(GetParent(hDlg));
				return TRUE;

			case IDC_BUTTONF3:
				MakeDebugFrameThree(GetParent(hDlg));
				return TRUE;

            case IDC_BUTTONF4:
                RemoveFromFrameQueue(&pwd->FTPBuffHead, 1);
                pwd->FTPQueueSize--;
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


VOID MakeDebugFrameOne(HWND hWnd){
	int i;
	PBYTE		data	= (PBYTE) malloc (sizeof(BYTE)*25);
	FRAME		frame	= {0};
	PWNDDATA    pwd     = NULL;

	pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);


	for (i =0;i<25;i++){
		data[i] = i+65;
	}
	frame = CreateFrame(hWnd,data,25);
	frame.sequence = 0;
	AddToFrameQueue(&pwd->FTPBuffHead,&pwd->FTPBuffTail,frame);
	pwd->FTPQueueSize+=1;
}


VOID MakeDebugFrameTwo(HWND hWnd){
	int i;
	int count = 48;
	PBYTE		data	= (PBYTE) malloc (sizeof(BYTE)*MAX_PAYLOAD_SIZE);
	FRAME		frame	= {0};
	PWNDDATA    pwd     = NULL;

	pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);


	for (i =0;i<MAX_PAYLOAD_SIZE;i++){
		if ((i+5) % 16 == 0) {
			data[i] = count++;
			if (count == 58) {
				count = 48;
			}
		} else {
			data[i] = (i%26)+65;
		}
	}
	frame = CreateFrame(hWnd,data,MAX_PAYLOAD_SIZE);
	frame.sequence = 1;
	AddToFrameQueue(&pwd->FTPBuffHead,&pwd->FTPBuffTail,frame);
	pwd->FTPQueueSize+=1;
}


VOID MakeDebugFrameThree(HWND hWnd){
	int i;
	int count = 48;
	PBYTE		data	= (PBYTE) malloc (sizeof(BYTE)*MAX_PAYLOAD_SIZE);
	FRAME		frame	= {0};
	PWNDDATA    pwd     = NULL;

	pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);


	for (i =0;i<MAX_PAYLOAD_SIZE;i++){
		if ((i+5) % 16 == 0) {
			data[i] = count++;
			if (count == 58) {
				count = 48;
			}
		} else {
			data[i] = (i%26)+65;
		}
	}
	frame = CreateFrame(hWnd,data,MAX_PAYLOAD_SIZE);
	frame.crc = 4;	// not the right crc
	AddToFrameQueue(&pwd->FTPBuffHead,&pwd->FTPBuffTail,frame);
	pwd->FTPQueueSize+=1;
}
