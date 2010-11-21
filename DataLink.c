#include "DataLink.h"

VOID ProcessWrite(HWND hWnd, PSTATEINFO psi) {
    PWNDDATA    pwd                     = NULL;
    OVERLAPPED  ol                      = {0};
    BYTE        pEnq[CTRL_CHAR_SIZE]    = {0};  
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);
    
    pEnq[CTRL_CHAR_INDEX] = ENQ;

    WriteFile(pwd->hPort, pEnq, CTRL_CHAR_SIZE, NULL, &ol);
    psi->iState     = STATE_T1;
    psi->dwTimeout  = TOR1;
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

    if (pReadBuf[CTRL_CHAR_INDEX] == ACK) {
        WriteFile(pwd->hPort, TEXT("write a frame"), CTRL_CHAR_SIZE, NULL, &ol);
        psi->iState     = STATE_T3;
		psi->itoCount   = 0;
		psi->dwTimeout  = TOR2;
        SetEvent(CreateEvent(NULL, FALSE, FALSE, TEXT("fillFTPBuffer")));
    } else {
        psi->iState     = STATE_IDLE;
        srand(GetTickCount());
        psi->dwTimeout = TOR0_BASE + rand() % TOR0_RANGE;
    }
}


VOID ReadT3(HWND hWnd, PSTATEINFO psi, BYTE* pReadBuf, DWORD dwLength) {
    PWNDDATA    pwd = NULL;
    OVERLAPPED  ol  = {0};
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    if (pReadBuf[CTRL_CHAR_INDEX] == ACK) {
        if (pwd->FTPQueueSize == 0) {
            WriteFile(pwd->hPort, TEXT("PUTA EOT"), CTRL_CHAR_SIZE, NULL, &ol);
        } else {
            //Get next frame
            SetEvent(CreateEvent(NULL, FALSE, FALSE, TEXT("fillFTPBuffer")));
            WriteFile(pwd->hPort, TEXT("A FRAME"), CTRL_CHAR_SIZE, NULL, &ol);
		    psi->itoCount = 0;
        }
    }
}


VOID ReadIDLE(HWND hWnd, PSTATEINFO psi, BYTE* pReadBuf, DWORD dwLength) {
    PWNDDATA    pwd = NULL;
    OVERLAPPED  ol  = {0};
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    if (pReadBuf[CTRL_CHAR_INDEX] == ENQ) {
        WriteFile(pwd->hPort, TEXT("AN ACK"), CTRL_CHAR_SIZE, NULL, &ol);
        psi->iState     = STATE_R2;
        psi->dwTimeout  = TOR3;
    }
}


VOID ReadR2(HWND hWnd, PSTATEINFO psi, BYTE* pReadBuf, DWORD dwLength) {
    PWNDDATA    pwd = NULL;
    OVERLAPPED  ol  = {0};
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    if (pReadBuf[CTRL_CHAR_INDEX] == EOT) {
        psi->iState = STATE_IDLE;
        srand(GetTickCount());
        psi->dwTimeout = TOR0_BASE + rand() % TOR0_RANGE;
    } 
    else if (pwd->PTFQueueSize >= FULL_BUFFER) {
        SetEvent(CreateEvent(NULL, FALSE, FALSE, TEXT("emptyPTFBuffer")));
    }
    else if (crcFast(pReadBuf, dwLength) == 0) {
        if (pwd->FTPQueueSize) {
            WriteFile(pwd->hPort, TEXT("AN RVI"), CTRL_CHAR_SIZE, NULL, &ol);
        } else {
            WriteFile(pwd->hPort, TEXT("AN ACK"), CTRL_CHAR_SIZE, NULL, &ol);
            psi->iState = STATE_T1;
        }
    }
}


VOID ProcessTimeout(PSTATEINFO psi) {
   
    switch (psi->iState) {
        
        case STATE_T1:
            srand(GetTickCount());
            psi->dwTimeout  = TOR0_BASE + rand() % TOR0_RANGE;
            psi->iState     = STATE_IDLE;
            return;

        case STATE_T3:
            psi->dwTimeout *= (DWORD) pow(2.0, ++(psi->itoCount));
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
	//DWORD		j;
	FRAME myFrame;
	//BYTE* myData;
	PWNDDATA    pwd                 = NULL;
	pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);
	//myData = (BYTE*) malloc(sizeof(BYTE) * FRAME_SIZE);
	
	myFrame.soh = 0x1;
	myFrame.sequence = pwd->TxSequenceNumber++;
	myFrame.length = (SHORT)dwLength;
	//myFrame.payload = psBuff;
	for (i = 0; i<dwLength;i++) {
		myFrame.payload[i] = *(psBuf++);
	}
	while(i<MAX_PAYLOAD_SIZE){
		myFrame.payload[i++] =0;
	}
	myFrame.crc =0;
	/*
	myFrame.payload = (BYTE*) calloc(MAX_PAYLOAD_SIZE,sizeof(BYTE));
	
	
	
	i = 0;
	j = 0;
	myData[i++] = myFrame.soh;
	myData[i++] = myFrame.sequence;
	myData[i++] = (BYTE) myFrame.length;
	myData[i++]	= myFrame.length>>sizeof(BYTE)*8;
	for(j = 0; j< MAX_PAYLOAD_SIZE;j++){
		myData[i++] = myFrame.payload[j];
	}
	myData[i++] = myFrame.crc;
	//memcpy(myData,&myFrame,sizeof(BYTE) * FRAME_SIZE);
	myFrame.crc = crcFast(myData,FRAME_SIZE - sizeof(crc));

*/
	myFrame.crc = crcFast((BYTE*)&myFrame,FRAME_SIZE - sizeof(crc));

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
	BYTE* ReadBuffer[1019] = {0};
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
	}
	SetWindowLongPtr(hWnd, 0, (LONG_PTR) pwd);
	
}

