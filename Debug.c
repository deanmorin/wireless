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
