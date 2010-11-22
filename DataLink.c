#include "DataLink.h"

VOID ProcessWrite(HWND hWnd, BYTE* pFrame, DWORD dwLength) {
    PWNDDATA    pwd = NULL;
    OVERLAPPED  ol  = {0};
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    WriteFile(pwd->hPort, pFrame, dwLength, NULL, &ol);
}


UINT ProcessRead(HWND hWnd, PSTATEINFO psi, BYTE* pReadBuf, DWORD dwLength) {  
    
    static UINT(*pReadFunc[READ_STATES])(HWND, PSTATEINFO, BYTE*, DWORD) 
            = { ReadT1, ReadT3, ReadIDLE, ReadR2 };
    // call the read function related to the current state
    pReadFunc[psi->iState](hWnd, psi, pReadBuf, dwLength);
}


UINT ReadT1(HWND hWnd, PSTATEINFO psi, BYTE* pReadBuf, DWORD dwLength) {
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


UINT ReadT3(HWND hWnd, PSTATEINFO psi, BYTE* pReadBuf, DWORD dwLength) {
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


UINT ReadIDLE(HWND hWnd, PSTATEINFO psi, BYTE* pReadBuf, DWORD dwLength) {
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


UINT ReadR2(HWND hWnd, PSTATEINFO psi, BYTE* pReadBuf, DWORD dwLength) {
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

    if (crcFast(pReadBuf, dwLength) == 0) { // also check for sequence #     
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


VOID SendFrame(HWND hWnd, PSTATEINFO psi) {
    PWNDDATA    pwd = NULL;
    static BYTE pCtrlFrame[CTRL_FRAME_SIZE] = {0}; 
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    if (pwd->FTPQueueSize == 0) {
        pCtrlFrame[CTRL_CHAR_INDEX] = EOT;
        ProcessWrite(hWnd, pCtrlFrame, CTRL_FRAME_SIZE);
        SENT_EOT++;
    } else {
        ProcessWrite(hWnd, NULL, CTRL_FRAME_SIZE); //PEEK NEXT FRAME
        UP_FRAMES++;
        SetEvent(CreateEvent(NULL, FALSE, FALSE, TEXT("fillFTPBuffer")));
		psi->itoCount = 0;
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
            DL_STATE        = psi->iState;
            return;

        case STATE_T1:
            /*if (++(psi->iFailedENQCount) >= MAX_FAILED_ENQS) {
                DISPLAY_ERROR("Connection cannot be established"); 
            }*/                   // NEED TO SET APPROPRIATE EVENT
            srand(GetTickCount());
            psi->dwTimeout  = TOR0_BASE + rand() % TOR0_RANGE;
            psi->iState     = STATE_IDLE;
            DL_STATE        = psi->iState;
            return;
        
        case STATE_T3:
            psi->dwTimeout *= TOR2_INCREASE_FACTOR;

            if (++(psi->itoCount) >= MAX_TIMEOUTS) {
                srand(GetTickCount());
                psi->dwTimeout  = TOR0_BASE + rand() % TOR0_RANGE;
                psi->iState     = STATE_IDLE;
            } else { 
                psi->iState     = STATE_T3;
                SendFrame(hWnd, psi);
            }
            DL_STATE = psi->iState;
            return;
        
        case STATE_R2:
            psi->dwTimeout *= TOR3_INCREASE_FACTOR;

            if (++(psi->itoCount) >= MAX_TIMEOUTS) {
                srand(GetTickCount());
                psi->dwTimeout  = TOR0_BASE + rand() % TOR0_RANGE;
                psi->iState     = STATE_IDLE;
                DL_STATE        = psi->iState;
            } 
            return;
        
        default:
            DISPLAY_ERROR("Invalid state for timeout");
            return;
    }
}


FRAME CreateFrame(HWND hWnd, BYTE* psBuf, DWORD dwLength){
	DWORD		i;
	FRAME myFrame;
	PWNDDATA    pwd                 = NULL;

	pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

	myFrame.soh = 0x1;
	myFrame.sequence = pwd->TxSequenceNumber++;
	myFrame.length = (SHORT)dwLength;

	for (i = 0; i<dwLength;i++) {
		myFrame.payload[i] = *(psBuf++);
	}
	while(i<MAX_PAYLOAD_SIZE){
		myFrame.payload[i++] =0;
	}
	myFrame.crc =0;
	myFrame.crc = crcFast((BYTE*)&myFrame,FRAME_SIZE - sizeof(crc));
	if(crcFast((BYTE*)&myFrame,FRAME_SIZE)!=0){
		DISPLAY_ERROR("Failure Creating Frame");
	}
	/*else{
			DISPLAY_ERROR("Success Creating Frame");
	}*/
	return myFrame;
}

VOID OpenFileTransmit(HWND hWnd){
	PWNDDATA pwd = {0};
	OPENFILENAME ofn;
	char szFile[100] = {0};
	pwd = (PWNDDATA)GetWindowLongPtr(hWnd, 0);

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = TEXT('\0');
	ofn.lpstrFilter = TEXT("All\0*.*\0");
	ofn.nMaxFile = sizeof(szFile);


	GetOpenFileName(&ofn);
	//pwd->lpszTransmitName = ofn.lpstrFile;
	pwd->hFileTransmit =CreateFile(ofn.lpstrFile, GENERIC_READ | GENERIC_WRITE,
							FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
							OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	SetEvent(CreateEvent(NULL, FALSE, FALSE, TEXT("fillFTPBuffer")));
	SetWindowLongPtr(hWnd, 0, (LONG_PTR) pwd);
	//MessageBox(hWnd, pwd->lpszTransmitName, "File", MB_OK);
	
}

VOID CloseFileTransmit(HWND hWnd){
	PWNDDATA pwd = {0};
	pwd = (PWNDDATA)GetWindowLongPtr(hWnd, 0);
	
	if(pwd->hFileTransmit != NULL){
		if(!CloseHandle(pwd->hFileTransmit)){
			DISPLAY_ERROR("Failed to close Transmit File");
		}
		//pwd->lpszTransmitName = "";
		pwd->hFileTransmit = NULL;
		pwd->NumOfReads = 0;
		SetWindowLongPtr(hWnd, 0, (LONG_PTR) pwd);
		
	}
}

VOID OpenFileReceive(HWND hWnd){
	PWNDDATA pwd = {0};
	OPENFILENAME ofn;
	char szFile[100] = {0};
	pwd = (PWNDDATA)GetWindowLongPtr(hWnd, 0);

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = TEXT('\0');
	ofn.lpstrFilter = TEXT("All\0*.*\0");
	ofn.nMaxFile = sizeof(szFile);


	GetSaveFileName(&ofn);
	//pwd->lpszReceiveName = ofn.lpstrFile;
	pwd->hFileReceive = CreateFile(ofn.lpstrFile, GENERIC_READ | GENERIC_WRITE,
									FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
									OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	
	SetWindowLongPtr(hWnd, 0, (LONG_PTR) pwd);
	
}

VOID CloseFileReceive(HWND hWnd){
	PWNDDATA pwd = {0};
	pwd = (PWNDDATA)GetWindowLongPtr(hWnd, 0);
	if(pwd->hFileReceive){
		if(!CloseHandle(pwd->hFileReceive)){
			DISPLAY_ERROR("Failed to close Receive File");
		}
		//pwd->lpszReceiveName = "";
		pwd->hFileReceive = NULL;
		SetWindowLongPtr(hWnd, 0, (LONG_PTR) pwd);
		
	}
}

VOID WriteToFile(HWND hWnd, PFRAME frame){
	PWNDDATA pwd = {0};
	DWORD dwBytesWritten = 0;
	pwd = (PWNDDATA)GetWindowLongPtr(hWnd, 0);

	if(!WriteFile(pwd->hFileReceive, frame->payload, frame->length, &dwBytesWritten, NULL)){
		DISPLAY_ERROR("Failed to write to file");
	}
}

VOID ReadFromFile(HWND hWnd){
	PWNDDATA pwd = {0};
	DWORD dwBytesRead = 0;
	DWORD dwBytesWritten = 0;
	DWORD	dwSizeOfFile = 0;
   	FRAME frame;

	BYTE* ReadBuffer = (BYTE*) malloc(sizeof(BYTE) *1019);

	pwd = (PWNDDATA)GetWindowLongPtr(hWnd, 0);
	

	dwSizeOfFile = GetFileSize(pwd->hFileTransmit, NULL);
	while(pwd->FTPQueueSize < FULL_BUFFER && pwd->hFileTransmit != NULL){
		if(pwd->NumOfReads == 0 && dwSizeOfFile >= 1019){
			if(!ReadFile(pwd->hFileTransmit, ReadBuffer, 1019, &dwBytesRead, NULL)){
				DISPLAY_ERROR("Failed to read from file");
			}
			pwd->NumOfReads++;
			SetEvent(CreateEvent(NULL, FALSE, FALSE, TEXT("dataToWrite")));
		} else if(pwd->NumOfReads == 0 && dwSizeOfFile < 1019){
			if(!ReadFile(pwd->hFileTransmit, ReadBuffer, dwSizeOfFile, &dwBytesRead, NULL)){
				DISPLAY_ERROR("Failed to read from file");
			}
			pwd->NumOfReads++;
		} else if(dwSizeOfFile/pwd->NumOfReads >= 1019){
			if(!ReadFile(pwd->hFileTransmit, ReadBuffer, 1019, &dwBytesRead, NULL)){
				DISPLAY_ERROR("Failed to read from file");
			}
			pwd->NumOfReads++;
		} else if(dwSizeOfFile/pwd->NumOfReads > 0){
			if(!ReadFile(pwd->hFileTransmit, ReadBuffer, dwSizeOfFile%pwd->NumOfReads, &dwBytesRead, NULL)){
				DISPLAY_ERROR("Failed to read from file");
			}
			CloseFileTransmit(hWnd);
			MessageBox(hWnd, TEXT("File Read Complete"), 0, MB_OK);
		}

		/*if(!WriteFile(pwd->hFileReceive, ReadBuffer, dwBytesRead, &dwBytesWritten, NULL)){
			DISPLAY_ERROR("Failed to write to file");
		}*/		
		
		//AddToFrameQueue(PPFRAME_NODE pHead, PPFRAME_NODE pTail, CreateFrame(hWnd, *ReadBuffer, dwBytesRead);
		//				 (FTPhead, FTPtail, CreateFrame(hWnd, *ReadBuffer, dwBytesRead)


				
		frame = CreateFrame(hWnd, ReadBuffer, dwBytesRead);
		
		//TODO: Enter FTP crit section
		AddToFrameQueue(pwd->FTPBuffHead, pwd->FTPBuffTail, frame);
		//TODO: exit FTP crit section



	}
	SetWindowLongPtr(hWnd, 0, (LONG_PTR) pwd);
	
}

