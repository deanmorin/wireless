#include "Debug.h"

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
