#include "DataLink.h"

VOID ProcessWrite(HWND hWnd, BYTE* pFrame, DWORD dwLength) {
    PWNDDATA    pwd			= NULL;
    OVERLAPPED  ol			= {0};
	DWORD		dwWritten	= 0;
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    if (!WriteFile(pwd->hPort, pFrame, dwLength, NULL, &ol)) {
		ProcessCommError(pwd->hPort);
		GetOverlappedResult(pwd->hPort, &ol, &dwWritten, TRUE);
	}
	if (dwWritten != dwLength) {
		DISPLAY_ERROR("Full frame was not written");
	}
}


UINT ProcessRead(HWND hWnd, PSTATEINFO psi, BYTE* pReadBuf, DWORD dwLength) {  
    static UINT( *pReadFunc[READ_STATES])(HWND, PSTATEINFO, BYTE*, DWORD) 
            = { ReadT1,  ReadT3,  ReadIDLE,  ReadR2 };
    PWNDDATA pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);
    
    return pReadFunc[psi->iState](hWnd, psi, pReadBuf, dwLength);
}


UINT ReadT1(HWND hWnd, PSTATEINFO psi, BYTE* pReadBuf, DWORD dwLength) {
    PWNDDATA    pwd = NULL;
    static BYTE pCtrlFrame[CTRL_FRAME_SIZE] = {0}; 
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    if (pReadBuf[CTRL_CHAR_INDEX] == ACK) {
        PostMessage(hWnd, WM_STAT, ACK, REC);
        PostMessage(hWnd, WM_STAT, STAT_STATE, STATE_T3);
        psi->iState				= STATE_T3;
		psi->dwTimeout			= TOR2;
        psi->itoCount			= 0;
		psi->iFailedENQCount		= 0;
		SendFrame(hWnd, psi);

    } else {	// unexpected character was received
        PostMessage(hWnd, WM_STAT, STAT_STATE, STATE_IDLE);
		psi->iState     = STATE_IDLE;
        psi->dwTimeout	= TOR0;
    }
    return CTRL_FRAME_SIZE;
}


UINT ReadT3(HWND hWnd, PSTATEINFO psi, BYTE* pReadBuf, DWORD dwLength) {
    PWNDDATA    pwd = NULL;
    static BYTE pCtrlFrame[CTRL_FRAME_SIZE] = {0}; 
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    if (pReadBuf[CTRL_CHAR_INDEX] == ACK) {
		RemoveFromFrameQueue(&pwd->FTPBuffHead, 1);     
		PostMessage(hWnd, WM_FILLFTPBUF, 0, 0);    
        PostMessage(hWnd, WM_STAT, ACK, REC);
        PostMessage(hWnd, WM_STAT, STAT_FRAMEACKD, SENT);
        SendFrame(hWnd, psi);

    } else if (pReadBuf[CTRL_CHAR_INDEX] == RVI) {
		RemoveFromFrameQueue(&pwd->FTPBuffHead, 1);
		PostMessage(hWnd, WM_FILLFTPBUF, 0, 0);
		pCtrlFrame[CTRL_CHAR_INDEX] = ACK;
        ProcessWrite(hWnd, pCtrlFrame, CTRL_FRAME_SIZE);
		PostMessage(hWnd, WM_STAT, RVI, REC);
        PostMessage(hWnd, WM_STAT, ACK, SENT);
        PostMessage(hWnd, WM_STAT, STAT_STATE, STATE_R2);
		psi->iState     = STATE_R2;
        psi->dwTimeout  = TOR3;
        psi->itoCount   = 0;

    } else {
		PostMessage(hWnd, WM_STAT, STAT_STATE, STATE_IDLE);
		psi->iState = STATE_IDLE;
	}
    return CTRL_FRAME_SIZE;
}


UINT ReadIDLE(HWND hWnd, PSTATEINFO psi, BYTE* pReadBuf, DWORD dwLength) {
    PWNDDATA    pwd = NULL;
    static BYTE pCtrlFrame[CTRL_FRAME_SIZE] = {0}; 
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    if (pReadBuf[CTRL_CHAR_INDEX] == ENQ) {
        pCtrlFrame[CTRL_CHAR_INDEX] = ACK;
        ProcessWrite(hWnd, pCtrlFrame, CTRL_FRAME_SIZE);
        PostMessage(hWnd, WM_STAT, ACK, SENT);
		PostMessage(hWnd, WM_STAT, STAT_STATE, STATE_R2);
        psi->iState     = STATE_R2;
        psi->dwTimeout  = TOR3;
    }
    return CTRL_FRAME_SIZE;
}


UINT ReadR2(HWND hWnd, PSTATEINFO psi, BYTE* pReadBuf, DWORD dwLength) {
    static BYTE pCtrlFrame[CTRL_FRAME_SIZE] = {0};
    PWNDDATA pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);
	
    if (pReadBuf[CTRL_CHAR_INDEX] == EOT) {
        PostMessage(hWnd, WM_STAT, EOT, REC);
        PostMessage(hWnd, WM_STAT, STAT_STATE, STATE_IDLE);
		psi->iState		= STATE_IDLE;
        psi->dwTimeout	= TOR0;
        return CTRL_FRAME_SIZE;
    }
    if (pReadBuf[0] != SOH) {		// an unexpected frame arrived
        PostMessage(hWnd, WM_STAT, STAT_STATE, STATE_IDLE);
		psi->iState		= STATE_IDLE;
        return INVALID_FRAME;
    }
    if (dwLength < FRAME_SIZE) {
        return UNFINISHED_FRAME;	// a full frame has not arrived at the port yet
    }
	PostMessage(hWnd, WM_STAT, STAT_FRAME, REC);


	if (crcFast(pReadBuf, dwLength) == 0  &&  pReadBuf[1] == psi->rxSeq) {

		AddToFrameQueue(&pwd->PTFBuffHead, &pwd->PTFBuffTail, *((PFRAME) pReadBuf));
		PostMessage(hWnd, WM_EMPTYPTFBUF, 0, 0);
		PostMessage(hWnd, WM_STAT, STAT_FRAMEACKD, REC);

        if (pwd->FTPQueueSize) {
            pCtrlFrame[CTRL_CHAR_INDEX] = RVI;
            ProcessWrite(hWnd, pCtrlFrame, CTRL_FRAME_SIZE);
            PostMessage(hWnd, WM_STAT, RVI, SENT);		            
            PostMessage(hWnd, WM_STAT, STAT_STATE, STATE_T1);
			psi->iState = STATE_T1;
        } else {
            pCtrlFrame[CTRL_CHAR_INDEX] = ACK;
            ProcessWrite(hWnd, pCtrlFrame, CTRL_FRAME_SIZE);
            PostMessage(hWnd, WM_STAT, ACK, SENT);
        }
		psi->rxSeq = (psi->rxSeq + 1) % 2;
    }
    return FRAME_SIZE;
}


VOID SendFrame(HWND hWnd, PSTATEINFO psi) {
    PWNDDATA    pwd = NULL;
    static BYTE pCtrlFrame[CTRL_FRAME_SIZE] = {0}; 
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    if (pwd->FTPQueueSize == 0) {
        pCtrlFrame[CTRL_CHAR_INDEX] = EOT;
        ProcessWrite(hWnd, pCtrlFrame, CTRL_FRAME_SIZE);
        PostMessage(hWnd, WM_STAT, EOT, SENT);
        PostMessage(hWnd, WM_STAT, STAT_STATE, STATE_IDLE);
		psi->iState     = STATE_IDLE;
		psi->dwTimeout  = TOR0;

    } else {
		// MUTEX
        ProcessWrite(hWnd, (BYTE*) &pwd->FTPBuffHead->f, FRAME_SIZE);
        PostMessage(hWnd, WM_STAT, STAT_FRAME, SENT);
    }
}


VOID ProcessTimeout(HWND hWnd, PSTATEINFO psi) {
    PWNDDATA    pwd = NULL;
    static BYTE pCtrlFrame[CTRL_FRAME_SIZE] = {0}; 
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    switch (psi->iState) {
        
        case STATE_IDLE:
            pCtrlFrame[CTRL_CHAR_INDEX] = ENQ;
            ProcessWrite(hWnd, pCtrlFrame, CTRL_FRAME_SIZE);
            psi->dwTimeout  = TOR1;
            psi->iState     = STATE_T1;
            PostMessage(hWnd, WM_STAT, STAT_STATE, STATE_T1);
            return;

        case STATE_T1:
            if (++(psi->iFailedENQCount) >= MAX_FAILED_ENQS) {
				PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_DISCONNECT, 0), 0);
                DISPLAY_ERROR("Connection cannot be established.\nDisconnecting..."); 
            }
            psi->dwTimeout  = TOR0;
            psi->iState     = STATE_IDLE;
            PostMessage(hWnd, WM_STAT, STAT_STATE, STATE_IDLE);
            return;
        
        case STATE_T3:
            psi->dwTimeout *= TOR2_INCREASE_FACTOR;

            if (++(psi->itoCount) >= MAX_TIMEOUTS) {
                psi->dwTimeout  = TOR0;
                psi->iState     = STATE_IDLE;
            } else { 
                SendFrame(hWnd, psi);
            }
            PostMessage(hWnd, WM_STAT, STAT_STATE, psi->iState);
            return;
        
        case STATE_R2:
            psi->dwTimeout *= TOR3_INCREASE_FACTOR;

            if (++(psi->itoCount) >= MAX_TIMEOUTS) {
                psi->dwTimeout  = TOR0;
                psi->iState     = STATE_IDLE;
                PostMessage(hWnd, WM_STAT, STAT_STATE, STATE_IDLE);
            } 
            return;
        
        default:
            DISPLAY_ERROR("Invalid state for timeout");
            return;
    }
}
