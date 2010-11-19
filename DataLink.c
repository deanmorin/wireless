#include "DataLink.h"

VOID ProcessWrite(HWND hWnd, PSTATEINFO psi) {
    PWNDDATA    pwd                     = NULL;
    OVERLAPPED  ol                      = {0};
    BYTE        pEnq[CTRL_CHAR_SIZE]    = {0};
    pwd     = (PWNDDATA) GetWindowLongPtr(hWnd, 0);
    pEnq[0] = ENQ;

    WriteFile(pwd->hPort, pEnq, CTRL_CHAR_SIZE, NULL, &ol);
    psi->iState = STATE_T1;
    srand(GetTickCount());
    psi->dwTimeout = rand() % TOR1 + 200;             // adjust this later
}


VOID ProcessRead(HWND hWnd, PSTATEINFO psi, BYTE* pReadBuf, DWORD dwLength) {  
    
    static VOID(*pReadFunc[READ_STATES])(HWND, PSTATEINFO, BYTE*, DWORD) 
            = { ReadT1, ReadT3, ReadIDLE, ReadR2 };
    // call the read function related to the current state
    pReadFunc[psi->iState](hWnd, psi, pReadBuf, dwLength);
}


VOID ReadT1(HWND hWnd, PSTATEINFO psi, BYTE* pReadBuf, DWORD dwLength) {
    PWNDDATA    pwd = NULL;
    OVERLAPPED  ol  = {0};
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    if (pReadBuf[0] == ACK) {
        WriteFile(pwd->hPort, TEXT("MYFILE"), CTRL_CHAR_SIZE, NULL, &ol);
        psi->iState     = STATE_T3;
		psi->itoCount   = 0;
		psi->dwTimeout  = TOR2;
//        SetEvent(OpenEvent(DELETE | SYNCHRONIZE, FALSE, TEXT("fillFTPBuffer")));
    } else {
        psi->iState     = STATE_IDLE;
    }
}


VOID ReadT3(HWND hWnd, PSTATEINFO psi, BYTE* pReadBuf, DWORD dwLength) {
    PWNDDATA    pwd = NULL;
    OVERLAPPED  ol  = {0};
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    if (pReadBuf[0] == ACK) {
        //if (ftpQueueSize == 0)
        // WriteFile(pwd->hPort, TEXT("PUTA EOT"), CTRL_CHAR_SIZE, NULL, &ol);
    } else {
        //Get next frame
        SetEvent(CreateEvent(NULL, FALSE, FALSE, TEXT("fillftpBuffer")));
        WriteFile(pwd->hPort, TEXT("A FRAME"), CTRL_CHAR_SIZE, NULL, &ol);
		psi->itoCount = 0;
    }
}


VOID ReadIDLE(HWND hWnd, PSTATEINFO psi, BYTE* pReadBuf, DWORD dwLength) {
    PWNDDATA    pwd = NULL;
    OVERLAPPED  ol  = {0};
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    if (pReadBuf[0] == ENQ) {
        WriteFile(pwd->hPort, TEXT("AN ACK"), CTRL_CHAR_SIZE, NULL, &ol);
        psi->iState     = STATE_R2;
        psi->dwTimeout  = TOR3;
    }
}


VOID ReadR2(HWND hWnd, PSTATEINFO psi, BYTE* pReadBuf, DWORD dwLength) {
    PWNDDATA    pwd = NULL;
    OVERLAPPED  ol  = {0};
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    if (pReadBuf[0] == EOT) {
        psi->iState = STATE_IDLE;
    } 
    //else if (portToQueueSize >= FULL_BUFFER) {
        // clear buffer
    //}
    else if (crcFast(pReadBuf, dwLength) == 0) {
 //       if (FileToPortQueueSize) {
 //           WriteFile(pwd->hPort, TEXT("AN RVI"), CTRL_CHAR_SIZE, NULL, &ol);
   //     } else {
            WriteFile(pwd->hPort, TEXT("AN ACK"), CTRL_CHAR_SIZE, NULL, &ol);
            psi->iState = STATE_T1;
     //   }
    }
}


VOID ProcessTimeout(PSTATEINFO psi) {
   
    switch (psi->iState) {
        
        case STATE_T1:
            psi->dwTimeout  = INFINITE;
            psi->iState     = STATE_IDLE;
            return;

        case STATE_T3:
            psi->dwTimeout *= (DWORD) pow((long double) 2, ++(psi->itoCount));
            psi->iState     = (psi->itoCount >= 3) ? STATE_IDLE : STATE_T2;
            return;
        
        case STATE_R2:
            psi->dwTimeout *= (DWORD) pow(2.0, ++(psi->itoCount));
            psi->iState     = (psi->itoCount >= 3) ? STATE_IDLE : STATE_R2;
            return;
        
        default:
            DISPLAY_ERROR("Invalid state for timeout");
            return;
    }
}

FRAME CreateFrame(HWND hWnd, BYTE* psBuf, DWORD dwLength){
	DWORD		i;
	DWORD		j;
	FRAME myFrame;
	BYTE* myData;
	PWNDDATA    pwd                 = NULL;
	pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);
	myData = (BYTE*) malloc(sizeof(BYTE) * FRAME_SIZE);
	
	myFrame.soh = 0x1;
	myFrame.sequence = pwd->TxSequenceNumber++;
	myFrame.length = (SHORT)dwLength;
	myFrame.payload = (BYTE*) calloc(MAX_PAYLOAD_SIZE,sizeof(BYTE));
	
	for (i = 0; i<dwLength;i++) {
		myFrame.payload[i] = *(psBuf++);
	}
	myFrame.crc =0;
	i = 0;
	j = 0;
	myData[i++] = myFrame.soh;
	myData[i++] = myFrame.sequence;
	myData[i++] = myFrame.length;
	myData[i++]	= myFrame.length>>sizeof(BYTE)*8;
	for(j = 0; j< MAX_PAYLOAD_SIZE;j++){
		myData[i++] = myFrame.payload[j];
	}
	myData[i++] = myFrame.crc;
	//memcpy(myData,&myFrame,sizeof(BYTE) * FRAME_SIZE);
	myFrame.crc = crcFast(myData,FRAME_SIZE - sizeof(crc));


	return myFrame;
}
