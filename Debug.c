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
	pwd->FTPQueueSize+=1;
}


VOID MakeDebugFrameTwo(HWND hWnd){
	int i;
	int count = 48;
	BYTE* data = (BYTE*) malloc (sizeof(BYTE)*MAX_PAYLOAD_SIZE);
	PWNDDATA    pwd                 = NULL;

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
	AddToFrameQueue(&pwd->FTPBuffHead,&pwd->FTPBuffTail,CreateFrame(hWnd,data,MAX_PAYLOAD_SIZE));
	pwd->FTPQueueSize+=1;
}


UINT DebugT1(HWND hWnd, PSTATEINFO psi, BYTE* pReadBuf, DWORD dwLength) {
    PWNDDATA    pwd = NULL;
    static BYTE pCtrlFrame[CTRL_FRAME_SIZE] = {0}; 
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    if (pReadBuf[CTRL_CHAR_INDEX] == ACK) {
        REC_ACK+=1;
        psi->iFailedENQCount = 0;
        psi->iState     = STATE_T3;
        DL_STATE        = psi->iState;
		psi->dwTimeout  = DTOR;
        SendFrame(hWnd, psi);
    } else {
        psi->iState     = STATE_IDLE;
        DL_STATE        = psi->iState;
        srand(GetTickCount());
        psi->dwTimeout = DTOR + rand() % TOR0_RANGE;
    }
    return CTRL_FRAME_SIZE;
}


UINT DebugT3(HWND hWnd, PSTATEINFO psi, BYTE* pReadBuf, DWORD dwLength) {
    PWNDDATA    pwd = NULL;
    static BYTE pCtrlFrame[CTRL_FRAME_SIZE] = {0}; 
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    if (pReadBuf[CTRL_CHAR_INDEX] == ACK) {         // last frame acknowledged
        REC_ACK+=1;
        UP_FRAMES_ACKD+=1;
        RemoveFromFrameQueue(&pwd->FTPBuffHead, 1); // remove ack'd frame from
        pwd->FTPQueueSize--;                        //      the queue
        SendFrame(hWnd, psi);                       
    } else if (pReadBuf[CTRL_CHAR_INDEX] == RVI) {  // receiver wants to send
                                                    //      a frame
        REC_RVI+=1;
        DL_STATE        = STATE_R4;
        WaitForSingleObject(CreateEvent(NULL, TRUE, FALSE, TEXT("ackPushed")), 
                                        INFINITE);
        SENT_ACK+=1;
        psi->iState     = STATE_R2;
        DL_STATE        = psi->iState;
        psi->dwTimeout  = DTOR;
        psi->itoCount  = 0;
    }
    return CTRL_FRAME_SIZE;
}


UINT DebugIDLE(HWND hWnd, PSTATEINFO psi, BYTE* pReadBuf, DWORD dwLength) {
    PWNDDATA    pwd = NULL;
    static BYTE pCtrlFrame[CTRL_FRAME_SIZE] = {0}; 
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    if (pReadBuf[CTRL_CHAR_INDEX] == ENQ) {
        DL_STATE        = STATE_R1;
        WaitForSingleObject(CreateEvent(NULL, TRUE, FALSE, 
                                            TEXT("ackPushed")), INFINITE);
        SENT_ACK+=1;
        psi->iState     = STATE_R2;
        DL_STATE        = psi->iState;
        psi->dwTimeout  = DTOR;
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
        REC_EOT+=1;
        psi->iState = STATE_IDLE;
        DL_STATE    = psi->iState;
        srand(GetTickCount());
        psi->dwTimeout = DTOR + rand() % TOR0_RANGE;
        return CTRL_FRAME_SIZE;
    } 
   
    if (dwLength < FRAME_SIZE) {
        return 0;   // a full frame has not arrived at the port yet
    }
    DOWN_FRAMES+=1;

    if (crcFast(pReadBuf, dwLength) == 0) { // ALSO CHECK FOR SEQUENCE #     
        DOWN_FRAMES_ACKD+=1;
        DL_STATE = STATE_R3;

        if (pwd->FTPQueueSize) {
            WaitForSingleObject(CreateEvent(NULL, TRUE, FALSE, 
                                            TEXT("rviPushed")), INFINITE);
            SENT_RVI+=1;
        } else {
            WaitForSingleObject(CreateEvent(NULL, TRUE, FALSE, 
                                            TEXT("ackPushed")), INFINITE);
            psi->iState = STATE_T1;
            DL_STATE    = psi->iState;
            SENT_ACK+=1;
        }
    }
    return FRAME_SIZE;
}
