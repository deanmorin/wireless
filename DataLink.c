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
        //if (fileToPortQueueSize == 0)
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

/*
ProcessRead(HWND hWnd, INT* state, INT* toCount){
	data = ReadFile

	switch (state){				
		case STATE_T1:
			if (data == ACK){
				GetNextFrame()
				SetEvent(bufferFrame)
				WriteFile()
				state = STATE_T3
				toCount = 0
				timeout = TOR2
			}
			else if (data == WACK){
				state = STATE_IDLE
			}
		case STATE_T3:
			if (data == ACK0 / 1){
				if (fileToPortQueueSize == 0){
					WriteFile(EOT);
				}
				else {
					GetNextFrame()
					SetEvent(bufferFrame)
					WriteFile()					// state doesn’t change
					toCount = 0
				}
			}
			else if (data){
			}
		case STATE_IDLE:
			if (data == ENQ){
				if (busyWithOtherTasks){
					WriteFile(WACK)		
				}// state doesn’t change
				else{
					WriteFile(ACK)
			   		state = STATE_R2
					timeout = TOR3
				}
			}
		case STATE_R2:
			if (data == frame){
				if (data == EOT){
					state = STATE_IDLE
					return
				}
				if (portToQueueSize >= FULL_BUFFER){
					clear buffer	// either through an event, or call function
				}
				if (CheckCRC() == no error){
					if (fileToPortQueueSize == 0){
						WriteFile(ACK)			// state doesn’t change
					}
					else {
						WriteFile(RVI0/1)
						state = T1
					}
				}
				else if (data == EOT){
					state = IDLE
				}
			}
	
	}
}
*/				
				/*if (dwQueueSize >= 2) {*/

               /* dwPacketLength = GetFromList(pHead, 2);*/ /*
			switch(GetFromList(pHead,1)){
				case 0x1://SOH  ie a frame.
					dwPacketLength = 1024;
					if (dwQueueSize >= dwPacketLength) {

						pcPacket = RemoveFromFront(&pHead, dwPacketLength);
						ProcessPacket(hWnd, pcPacket, dwPacketLength);
						memset(psReadBuf, 0, READ_BUFSIZE);
						/*requestPending = FALSE;*/
//						free(pcPacket);
//					}
/*
					break;
				case 0x4://EOT
					dwPacketLength = 1;
					//process EOT
					RemoveFromFront(&pHead, dwPacketLength);
					break;
				case 0x5://ENQ
					dwPacketLength = 1;
					//process ENQ
					RemoveFromFront(&pHead, dwPacketLength);
					break;
				case 0x6://ACK0
					dwPacketLength = 1;
					//process ACK0
					RemoveFromFront(&pHead, dwPacketLength);
					break;
				case 0x11://ACK1
					dwPacketLength = 1;
					//process ACK1
					RemoveFromFront(&pHead, dwPacketLength);
					break;
				case 0x12://WACK
					dwPacketLength = 1;
					//process WACK
					RemoveFromFront(&pHead, dwPacketLength);
					break;
				case 0x13://RVI
					dwPacketLength = 1;
					//process RVI
					RemoveFromFront(&pHead, dwPacketLength);
					break;
			}

            */