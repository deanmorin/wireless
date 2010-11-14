#include "DataLink.h"

VOID ProcessWrite(HWND hWnd, INT* piState, DWORD* pdwTimeout) {
    PWNDDATA    pwd                     = NULL;
    OVERLAPPED  olWrite                 = {0};
    BYTE        pEnq[CTRL_CHAR_SIZE]    = {0};
    pwd     = (PWNDDATA) GetWindowLongPtr(hWnd, 0);
    pEnq[0] = ENQ;

    WriteFile(pwd->hPort, pEnq, CTRL_CHAR_SIZE, NULL, &olWrite);
    *piState = STATE_T1;
    srand(GetTickCount());
    *pdwTimeout = rand() % TOR1 + 200;             // adjust this later
}


VOID ProcessTimeout(DWORD* pdwTimeout, INT* piTOCount, INT* piState) {
   
    switch (*piState) {
        
        case STATE_T1:
            *pdwTimeout  = INFINITE;
            *piState     = STATE_T2;
            return;

        case STATE_T3:
            *pdwTimeout *= pow((long double) 2, ++(*piTOCount));
            *piState     = (*piTOCount >= 3) ? STATE_IDLE : STATE_T2;
            return;
        
        case STATE_R2:
            *pdwTimeout *= pow((long double) 2, ++(*piTOCount));
            *piState     = (*piTOCount >= 3) ? STATE_IDLE : STATE_R2;
            return;
        
        default:
            DISPLAY_ERROR("Invalid state for timeout");
            return;
    }
}

    //void(*p[2])(int)

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