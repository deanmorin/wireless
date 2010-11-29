#include "FileIO.h"

FRAME CreateFrame(HWND hWnd, PBYTE psBuf, DWORD dwLength){
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
	myFrame.crc = crcFast((PBYTE)&myFrame,FRAME_SIZE - sizeof(crc));
	if(crcFast((PBYTE)&myFrame,FRAME_SIZE)!=0){
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
	HANDLE		hMutex = {0};
	pwd = (PWNDDATA)GetWindowLongPtr(hWnd, 0);
	
	while(pwd->PTFQueueSize != 0){
		hMutex = CreateMutex(NULL, FALSE, TEXT("PTFMutex"));
		tempFrame = RemoveFromFrameQueue(&pwd->PTFBuffHead, 1);
		ReleaseMutex(hMutex);
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
	HANDLE hMutex = {0};

	PBYTE ReadBuffer = (PBYTE) malloc(sizeof(BYTE) *1019);

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
		} else if(dwSizeOfFile/pwd->NumOfReads > 1019){
			if(!ReadFile(pwd->hFileTransmit, ReadBuffer, 1019, &dwBytesRead, NULL)){
				DISPLAY_ERROR("Failed to read from file");
			}
			pwd->NumOfReads+=1;
		} else if(dwSizeOfFile/pwd->NumOfReads == 1019){
			frame = CreateNullFrame(hWnd);
			AddToFrameQueue(&pwd->FTPBuffHead, &pwd->FTPBuffTail, frame);
			pwd->FTPQueueSize+=1;
			CloseFileTransmit(hWnd);
			ReleaseMutex(hMutex);
			return;
		} else if(dwSizeOfFile/pwd->NumOfReads > 0){
			if(!ReadFile(pwd->hFileTransmit, ReadBuffer, dwSizeOfFile%pwd->NumOfReads, &dwBytesRead, NULL)){
				DISPLAY_ERROR("Failed to read from file");
			}
			CloseFileTransmit(hWnd);
			//MessageBox(hWnd, TEXT("File Read Complete"), 0, MB_OK);
		}
		else{

			return;
		}
				
		frame = CreateFrame(hWnd, ReadBuffer, dwBytesRead);

		hMutex = CreateMutex(NULL, FALSE, TEXT("FTPMutex"));
		AddToFrameQueue(&pwd->FTPBuffHead, &pwd->FTPBuffTail, frame);
		pwd->FTPQueueSize+=1;
		/*if(dwSizeOfFile/1019 == pwd->NumOfReads){
			frame = CreateNullFrame(hWnd);
			AddToFrameQueue(&pwd->FTPBuffHead, &pwd->FTPBuffTail, frame);
			pwd->FTPQueueSize+=1;
			CloseFileTransmit(hWnd);
			ReleaseMutex(hMutex);
			return;
		}*/
		ReleaseMutex(hMutex);
	}
}

FRAME CreateNullFrame(HWND hWnd){
	FRAME nullFrame = CreateFrame(hWnd, 0, 0);
	return nullFrame;
}
