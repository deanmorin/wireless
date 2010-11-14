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


VOID ReadFromPort(HWND hWnd, PSTATEINFO psi, OVERLAPPED ol, DWORD cbInQue) {

    PWNDDATA    pwd                     = NULL;
    CHAR        psReadBuf[READ_BUFSIZE] = {0};
    DWORD       dwBytesRead             = 0;
    DWORD	    dwPacketLength 		    = 0;
	CHAR*		pcPacket	            = NULL;
    CHAR_LIST*  pHead                   = NULL;
    DWORD       dwQueueSize             = 0;
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);
    
    if (!ReadFile(pwd->hPort, psReadBuf, cbInQue, &dwBytesRead, &ol)) {
        // read is incomplete or had an error
        ProcessCommError(pwd->hPort);
        GetOverlappedResult(pwd->hThread, &ol, &dwBytesRead, TRUE);
    }

    if (psi->iState == STATE_R2  &&  dwBytesRead < FRAME_SIZE) {
    }

    dwQueueSize = AddToBack(&pHead, psReadBuf, dwBytesRead);
            
    if (dwQueueSize >= dwPacketLength) {
        pcPacket = RemoveFromFront(&pHead, dwPacketLength);
	 	ProcessPacket(hWnd, pcPacket, dwPacketLength);
        memset(psReadBuf, 0, READ_BUFSIZE);
        free(pcPacket);
	}
	InvalidateRect(hWnd, NULL, FALSE);    
    ResetEvent(ol.hEvent);
}

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
    
    PWNDDATA    pwd                 = NULL;
    OVERLAPPED  ol                  = {0};
    DWORD       dwEvent             = 0;
    DWORD       dwError             = 0;
    COMSTAT     cs                  = {0};
    HANDLE*     hEvents             = NULL;
    INT         iEventsSize         = 0;
    PSTATEINFO  psi             = NULL;
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);
    
    CreateEvent(NULL, TRUE, FALSE, TEXT("dataToWrite"));         //REMOVE
    if ((ol.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL) {
        DISPLAY_ERROR("Error creating event in read thread");
    }
    hEvents     = (HANDLE*) malloc(sizeof(HANDLE) * 3);
    hEvents[0]  = OpenEvent(DELETE | SYNCHRONIZE, FALSE, TEXT("disconnected"));
    hEvents[1]  = ol.hEvent;                               // "dataAtPort"
    hEvents[2]  = OpenEvent(DELETE | SYNCHRONIZE, FALSE, TEXT("dataToWrite"));

    psi             = (PSTATEINFO) malloc(sizeof(STATEINFO));
    psi->iState     = STATE_IDLE;
    psi->dwTimeout  = INFINITE;
    psi->iTOCount   = 0;

    while (pwd->bConnected) {
        
        SetCommMask(pwd->hPort, EV_RXCHAR);      
        if (!WaitCommEvent(pwd->hPort, &dwEvent, &ol)) {
            ProcessCommError(pwd->hPort);
        }
        iEventsSize = (psi->iState == STATE_IDLE) ? 3 : 2;
        dwEvent = WaitForMultipleObjects(iEventsSize, hEvents, FALSE, psi->dwTimeout);
        ClearCommError(pwd->hPort, &dwError, &cs);
 
        if (dwEvent == WAIT_OBJECT_0) {
            // the connection was severed
            break;
        }
        else if (dwEvent == WAIT_OBJECT_0 + 1  &&  cs.cbInQue) {
            // data arrived at the port
            ReadFromPort(hWnd, psi, ol, cs.cbInQue);
            //ProcessRead(hWnd, &state, &toCount);
        }
        else if (dwEvent == WAIT_OBJECT_0 + 2) {
            // in idle state, with data ready to write to port
            ProcessWrite(hWnd, psi);
        }
        else if (dwEvent == WAIT_TIMEOUT) {
            // a timeout occured
            ProcessTimeout(psi);
        }
        else {
            // change this before release
            DISPLAY_ERROR("Invalid event occured in the Port I/O thread");
        }
    }


    if (!PurgeComm(pwd->hPort, PURGE_RXCLEAR)) {
        DISPLAY_ERROR("Error purging read buffer");
    }
    free(hEvents);
    CloseHandle(ol.hEvent);
    return 0;
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
