#include "DataLink.h"

INT GetStateEvents(HANDLE* hEvents, INT state) {
    
    switch (state) {
        case STATE_IDLE:
        case STATE_T1:
        case STATE_T3:
        case STATE_R2:
            break;
    }
	return 0;
}

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