#include "Debug.h"

VOID MakeDebugFrameOne(HWND hWnd){
	int i;
	BYTE* data = (BYTE*) malloc (sizeof(BYTE)*25);
	PWNDDATA    pwd                 = NULL;

	pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);


	for (i =0;i<25;i++){
		data[i] = i+65;
	}
	AddToFrameQueue(&pwd->FTPBuffHead,&pwd->FTPBuffTail,CreateFrame(hWnd,data,25));
	pwd->FTPQueueSize++;
}



VOID MakeDebugFrameTwo(HWND hWnd){
	int i;
	BYTE* data = (BYTE*) malloc (sizeof(BYTE)*MAX_PAYLOAD_SIZE);
	PWNDDATA    pwd                 = NULL;

	pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);


	for (i =0;i<MAX_PAYLOAD_SIZE;i++){
		data[i] = (i%26)+65;
	}
	AddToFrameQueue(&pwd->FTPBuffHead,&pwd->FTPBuffTail,CreateFrame(hWnd,data,MAX_PAYLOAD_SIZE));
	pwd->FTPQueueSize++;
}


UINT DebugT1(HWND hWnd, PSTATEINFO psi, BYTE* pReadBuf, DWORD dwLength) {
    PWNDDATA    pwd = NULL;
    static BYTE pCtrlFrame[CTRL_FRAME_SIZE] = {0}; 
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    if (pReadBuf[CTRL_CHAR_INDEX] == ACK) {
        REC_ACK++;
        psi->iFailedENQCount = 0;
        psi->iState     = STATE_T3;
        DL_STATE        = psi->iState;
		psi->dwTimeout  = TOR2;
        SendFrame(hWnd, psi);
    } else {
        psi->iState     = STATE_IDLE;
        DL_STATE        = psi->iState;
        srand(GetTickCount());
        psi->dwTimeout = TOR0_BASE + rand() % TOR0_RANGE;
    }
    return CTRL_FRAME_SIZE;
}


UINT DebugT3(HWND hWnd, PSTATEINFO psi, BYTE* pReadBuf, DWORD dwLength) {
    PWNDDATA    pwd = NULL;
    static BYTE pCtrlFrame[CTRL_FRAME_SIZE] = {0}; 
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    if (pReadBuf[CTRL_CHAR_INDEX] == ACK) {         // last frame acknowledged
        REC_ACK++;
        UP_FRAMES_ACKD++;
        // pop ack'd frame
        SendFrame(hWnd, psi);
    } else if (pReadBuf[CTRL_CHAR_INDEX] == RVI) {  // receiver wants to send
                                                    // a frame
        REC_RVI++;
        pCtrlFrame[CTRL_CHAR_INDEX] = ACK;
        ProcessWrite(hWnd, pCtrlFrame, CTRL_FRAME_SIZE);
        SENT_ACK++;
        psi->iState     = STATE_R2;
        DL_STATE        = psi->iState;
        psi->dwTimeout  = TOR3;
        psi->itoCount  = 0;
    }
    return CTRL_FRAME_SIZE;
}


UINT DebugIDLE(HWND hWnd, PSTATEINFO psi, BYTE* pReadBuf, DWORD dwLength) {
    PWNDDATA    pwd = NULL;
    static BYTE pCtrlFrame[CTRL_FRAME_SIZE] = {0}; 
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    if (pReadBuf[CTRL_CHAR_INDEX] == ENQ) {
        pCtrlFrame[CTRL_CHAR_INDEX] = ACK;
        ProcessWrite(hWnd, pCtrlFrame, CTRL_FRAME_SIZE);
        SENT_ACK++;
        psi->iState     = STATE_R2;
        DL_STATE        = psi->iState;
        psi->dwTimeout  = TOR3;
    }
    return CTRL_FRAME_SIZE;
}


UINT DebugR2(HWND hWnd, PSTATEINFO psi, BYTE* pReadBuf, DWORD dwLength) {
    PWNDDATA    pwd = NULL;
    static BYTE pCtrlFrame[CTRL_FRAME_SIZE] = {0};
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    if (pwd->PTFQueueSize >= FULL_BUFFER) {
        SetEvent(CreateEvent(NULL, FALSE, FALSE, TEXT("emptyPTFBuffer")));
    }

    if (pReadBuf[CTRL_CHAR_INDEX] == EOT) {
        REC_EOT++;
        psi->iState = STATE_IDLE;
        DL_STATE    = psi->iState;
        srand(GetTickCount());
        psi->dwTimeout = TOR0_BASE + rand() % TOR0_RANGE;
        return CTRL_FRAME_SIZE;
    } 
   
    if (dwLength < FRAME_SIZE) {
        return 0;   // a full frame has not arrived at the port yet
    }
    DOWN_FRAMES++;

    if (crcFast(pReadBuf, dwLength) == 0) { // ALSO CHECK FOR SEQUENCE #     
        DOWN_FRAMES_ACKD++;

        if (pwd->FTPQueueSize) {
            pCtrlFrame[CTRL_CHAR_INDEX] = RVI;
            ProcessWrite(hWnd, pCtrlFrame, CTRL_FRAME_SIZE);
            SENT_RVI++;
        } else {
            pCtrlFrame[CTRL_CHAR_INDEX] = ACK;
            ProcessWrite(hWnd, pCtrlFrame, CTRL_FRAME_SIZE);
            psi->iState = STATE_T1;
            DL_STATE    = psi->iState;
            SENT_ACK++;
        }
    }
    return FRAME_SIZE;
}
