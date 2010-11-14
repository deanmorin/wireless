/*------------------------------------------------------------------------------
-- SOURCE FILE:     Physical.c - Contains all the OSI "physical layer"
--                               functions for the RFID reader. 
--                               The definitions for the OSI layers
--                               have been loosened somewhat, since the purpose 
--                               is to organize the functions intuitively, 
--                               rather than pedantically.
--                      
-- PROGRAM:     RFID Reader - Enterprise Edition
--
-- FUNCTIONS:
--              DWORD WINAPI    ReadThreadProc(HWND);
--				VOID	        RequestPacket(HWND hWnd);
--              VOID            ProcessCommError(HANDLE);
--
--
-- DATE:        Oct 13, 2010
--
-- REVISIONS:   Nov 05, 2010
--              Modified ReadThreadProc to work more appropriately for the RFID
--              reader. Added RequestPacket()
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin, Daniel Wright
--
-- NOTES:
-- Contains physical level functions for the RFID reader.
------------------------------------------------------------------------------*/

#include "Physical.h"

/*------------------------------------------------------------------------------
-- FUNCTION:    ReadThreadProc
--
-- DATE:        Oct 13, 2010
--
-- REVISIONS:   Nov 05, 2010
--              Modified the function to also listen for a "disconnect" event,
--              and to break in that case.
--              ProcessRead() is now called once a complete packet is confirmed
--              (as opposed to sending the contents of the buffer to 
--              ProcessRead() as soon as they arrive).
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin, Daniel Wright
--
-- INTERFACE:   DWORD WINAPI ReadThreadProc(HWND hWnd)
--                          hWnd - the handle to the window
--
-- RETURNS:     0 because threads are required to return a DWORD.
--
-- NOTES:
--              While connected, this thread will loop and wait for characters
--              to arrive at the port. Once the EV_RXCHAR event is triggered,
--              ReadFile() is called to get however many characters have arrived
--              at the port by that time. This function uses overlapped I/O.
------------------------------------------------------------------------------*/
DWORD WINAPI PortIOThreadProc(HWND hWnd) {
    
    PWNDDATA        pwd                     = NULL;
    CHAR            psReadBuf[READ_BUFSIZE] = {0};
    OVERLAPPED      overlap                 = {0};
    DWORD           dwBytesRead             = 0;
    DWORD           dwEvent                 = 0;
    DWORD           dwError                 = 0;
    COMSTAT         cs                      = {0};
    HANDLE*         hEvents                 = NULL;
    INT             hEventsSize             = 0;
	BOOL			requestPending 			= FALSE;
	DWORD			dwPacketLength 			= 0;
	CHAR*			pcPacket			    = NULL;
    CHAR_LIST*      pHead                   = NULL;
    DWORD           dwQueueSize             = 0;
	DWORD           i                       = 0;
    INT             state                   = STATE_IDLE;
    INT             timeout                 = INFINITE;
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);
    
    //GetStateEvents(hEvents, state);
	
    if ((overlap.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL) {
        DISPLAY_ERROR("Error creating event in read thread");
    }
    hEvents[0] = overlap.hEvent;
    hEvents[1] = OpenEvent(DELETE | SYNCHRONIZE, FALSE, TEXT("disconnected"));

	
    while (pwd->bConnected) {

		/*
		if(!requestPending){
			RequestPacket(hWnd);
			requestPending = TRUE;
		}*/

        SetCommMask(pwd->hPort, EV_RXCHAR);
        if (!WaitCommEvent(pwd->hPort, &dwEvent, &overlap)) {
            ProcessCommError(pwd->hPort);
        }
        dwEvent = WaitForMultipleObjects(hEventsSize, hEvents, FALSE, timeout);
        if (dwEvent == WAIT_OBJECT_0 + 1) {
            // the connection was severed
            break;
        }	
        ClearCommError(pwd->hPort, &dwError, &cs);    
		
        // ensures that there is a character at the port
        if (cs.cbInQue) {  
            if (!ReadFile(pwd->hPort, psReadBuf, cs.cbInQue, 
                          &dwBytesRead, &overlap)) {
                // read is incomplete or had an error
                ProcessCommError(pwd->hPort);
                GetOverlappedResult(pwd->hThread, &overlap, &dwBytesRead, TRUE);
            }

            dwQueueSize = AddToBack(&pHead, psReadBuf, dwBytesRead);
            
                if (dwQueueSize >= dwPacketLength) {

                    pcPacket = RemoveFromFront(&pHead, dwPacketLength);
				    ProcessPacket(hWnd, pcPacket, dwPacketLength);
                    memset(psReadBuf, 0, READ_BUFSIZE);
                    free(pcPacket);
			    }

			
                InvalidateRect(hWnd, NULL, FALSE);
        }
        ResetEvent(overlap.hEvent);
    }


    if (!PurgeComm(pwd->hPort, PURGE_RXCLEAR)) {
        DISPLAY_ERROR("Error purging read buffer");
    }
    CloseHandle(overlap.hEvent);
    return 0;
}

/*------------------------------------------------------------------------------
-- FUNCTION:    RequestPacket
--
-- DATE:        Nov 4, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Daniel Wright
--
-- PROGRAMMER:  Daniel Wright
--
-- INTERFACE:   BOOL RequestPacket(HWND hWnd)
--                          hWnd        - the handle to the window
--                          
-- RETURNS:     True if the port write was successful.
--
-- NOTES:
--              Writes a string representing a packet request to the port.
------------------------------------------------------------------------------*/
BOOL RequestPacket(HWND hWnd) {
 
    PWNDDATA    pwd             = {0};
    CHAR        psWriteBuf[10]  = {0};
    OVERLAPPED  overlap         = {0};
    DWORD       dwBytesRead     = 0;
    UINT        bufLength       = 9;
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    psWriteBuf[0] = 0x01;
	psWriteBuf[1] = 0x09;
	psWriteBuf[2] = 0x00;
	psWriteBuf[3] = 0x03;
	psWriteBuf[4] = 0x01;
	psWriteBuf[5] = 0x41;
	psWriteBuf[6] = 0x00; 
	psWriteBuf[7] = 0x4B; 
	psWriteBuf[8] = 0xB4; 

    if (!WriteFile(pwd->hPort, psWriteBuf, bufLength, &dwBytesRead, &overlap)) {
		if (GetLastError() != ERROR_IO_PENDING) {
            return FALSE;
        }
    }
    return TRUE;
}

/*------------------------------------------------------------------------------
-- FUNCTION:    ProcessCommError
--
-- DATE:        Oct 13, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   VOID ProcessCommError(HANDLE hPort)
--                          hPort - the handle to the open port
--
-- RETURNS:     VOID.
--
-- NOTES:
--              Displays messages for various communication errors. Most of the
--              time, this function will process ERROR_IO_PENDING and return 
--              early since this is expected in overlapped I/O.
------------------------------------------------------------------------------*/
VOID ProcessCommError(HANDLE hPort) {
    DWORD dwError;

    if (GetLastError() == ERROR_IO_PENDING) {
        return;
    }
    ClearCommError(hPort, &dwError, NULL);

    switch (dwError) {

        case CE_BREAK:
            DISPLAY_ERROR("The hardware detected a break condition");
        case CE_FRAME:
            DISPLAY_ERROR("The hardware detected a framing error");
        case CE_OVERRUN:
            DISPLAY_ERROR("A character-buffer overrun has occurred. The next character is lost.");
        case CE_RXOVER:
            DISPLAY_ERROR("An input buffer overflow has occurred. There is either no room in the input buffer, or a character was received after the end-of-file (EOF) character");
        case CE_RXPARITY:
            DISPLAY_ERROR("The hardware detected a parity error");
        default:
            DISPLAY_ERROR("A communication error occured");
    }
}
