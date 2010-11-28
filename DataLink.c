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


FRAME CreateFrame(HWND hWnd, BYTE* psBuf, DWORD dwLength){
	DWORD		i;
	FRAME myFrame;
	PWNDDATA    pwd                 = NULL;

	pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

	myFrame.soh = 0x1;
	myFrame.sequence = pwd->TxSequenceNumber;
    pwd->TxSequenceNumber= (pwd->TxSequenceNumber==0)?1:0;
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
	TCHAR szFile[100] = {0};
	pwd = (PWNDDATA)GetWindowLongPtr(hWnd, 0);

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = TEXT('\0');
	ofn.lpstrFilter = TEXT("All\0*.*\0");
	ofn.nMaxFile = sizeof(szFile);


	GetOpenFileName(&ofn);
	pwd->hFileTransmit =CreateFile(ofn.lpstrFile, GENERIC_READ | GENERIC_WRITE,
							FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
							OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	pwd->NumOfReads = 0;
	PostMessage(hWnd, WM_FILLFTPBUF, 0, 0);
}

VOID CloseFileTransmit(HWND hWnd){
	PWNDDATA pwd = {0};
	pwd = (PWNDDATA)GetWindowLongPtr(hWnd, 0);
	
	if(pwd->hFileTransmit != NULL){
		if(!CloseHandle(pwd->hFileTransmit)){
			DISPLAY_ERROR("Failed to close Transmit File");
		}
		pwd->hFileTransmit = NULL;
		pwd->NumOfReads = 0;
		SetWindowLongPtr(hWnd, 0, (LONG_PTR) pwd);
	}
}

BOOL OpenFileReceive(HWND hWnd){
	PWNDDATA pwd = {0};
	OPENFILENAME ofn;
	TCHAR szFile[100] = {0};                  	
    pwd = (PWNDDATA)GetWindowLongPtr(hWnd, 0);

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = TEXT('\0');
	ofn.lpstrFilter = TEXT("All\0*.*\0");
	ofn.nMaxFile = sizeof(szFile);


	if(GetSaveFileName(&ofn) == 0){
		return FALSE;
	} 
	
	
	pwd->hFileReceive = CreateFile(ofn.lpstrFile, GENERIC_READ | GENERIC_WRITE,
									FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
									CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	
	SetWindowLongPtr(hWnd, 0, (LONG_PTR) pwd);
	return TRUE;
}

VOID CloseFileReceive(HWND hWnd){
	PWNDDATA pwd = {0};
	pwd = (PWNDDATA)GetWindowLongPtr(hWnd, 0);
	if(pwd->hFileReceive){
		if(!CloseHandle(pwd->hFileReceive)){
			DISPLAY_ERROR("Failed to close Receive File");
		}
		pwd->hFileReceive = NULL;
			
	}
}

VOID WriteToFile(HWND hWnd){
	PWNDDATA pwd = {0};
	DWORD dwBytesWritten = 0;
	PFRAME		tempFrame = {0};
	pwd = (PWNDDATA)GetWindowLongPtr(hWnd, 0);
	
	while(pwd->PTFQueueSize != 0){
		tempFrame = RemoveFromFrameQueue(&pwd->PTFBuffHead, 1);
		if(tempFrame->length != 0){
			DisplayFrameInfo(hWnd, *tempFrame);
			pwd->NumOfFrames++;
			SetWindowLongPtr(hWnd, 0, (LONG_PTR) pwd);
		}
		if(!WriteFile(pwd->hFileReceive, tempFrame->payload, tempFrame->length, &dwBytesWritten, NULL)){
			DISPLAY_ERROR("Failed to write to file");
		} else {
			pwd->PTFQueueSize--;
		}
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
			pwd->NumOfReads+=1;
			SetEvent(CreateEvent(NULL, FALSE, FALSE, TEXT("dataToWrite")));
		} else if(pwd->NumOfReads == 0 && dwSizeOfFile < 1019){
			if(!ReadFile(pwd->hFileTransmit, ReadBuffer, dwSizeOfFile, &dwBytesRead, NULL)){
				DISPLAY_ERROR("Failed to read from file");
			}
			pwd->NumOfReads+=1;
			SetEvent(CreateEvent(NULL, FALSE, FALSE, TEXT("dataToWrite")));
		} else if(dwSizeOfFile/pwd->NumOfReads >= 1019){
			if(!ReadFile(pwd->hFileTransmit, ReadBuffer, 1019, &dwBytesRead, NULL)){
				DISPLAY_ERROR("Failed to read from file");
			}
			pwd->NumOfReads+=1;
		} else if(dwSizeOfFile/pwd->NumOfReads > 0){
			if(!ReadFile(pwd->hFileTransmit, ReadBuffer, dwSizeOfFile%pwd->NumOfReads, &dwBytesRead, NULL)){
				DISPLAY_ERROR("Failed to read from file");
			}
			CloseFileTransmit(hWnd);
			SetWindowLongPtr(hWnd, 0, (LONG_PTR) pwd);
			MessageBox(hWnd, TEXT("File Read Complete"), 0, MB_OK);
		}
				
		frame = CreateFrame(hWnd, ReadBuffer, dwBytesRead);

		//TODO: Enter FTP crit section
		AddToFrameQueue(&pwd->FTPBuffHead, &pwd->FTPBuffTail, frame);
		pwd->FTPQueueSize+=1;
		//TODO: exit FTP crit section
	}
}

