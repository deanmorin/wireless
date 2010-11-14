#include "DataLink.h"

INT GetStateEvents(HANDLE* hEvents, INT state) {
    
    switch (state) {
        case STATE_IDLE:
            //hEvents;
        case STATE_T1:
        case STATE_T3:
        case STATE_R2:
            break;
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