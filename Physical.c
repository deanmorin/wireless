/*------------------------------------------------------------------------------
-- SOURCE FILE:     Physical.c - Contains all the OSI "physical layer"
--                               functions for the wireless protocol tester. 
--                               The definitions for the OSI layers
--                               have been loosened somewhat, since the purpose 
--                               is to organize the functions intuitively, 
--                               rather than pedantically.
--                      
-- PROGRAM:     Dean and the Rockets' Wireless Protocol Testing and Evaluation 
--              Facilitator
--
-- FUNCTIONS:
--              DWORD WINAPI    PortIOThreadProc(HWND);
--				VOID	InitStateInfo(PSTATEINFO);
--              VOID    ProcessCommError(HANDLE);
--				VOID	ReadFromPort(HWND, PSTATEINFO, OVERLAPPED, DWORD)
--
--
-- DATE:        Oct 13, 2010
--
-- REVISIONS:   Nov 05, 2010	
--				Modified ReadThreadProc to work more appropriately for the RFID 
--				reader. Added RequestPacket().
--				
--				Nov 29, 2010
--				Renamed ReadThreadProc() to PortIOThreadProc(), and modified it 
--				to work more appropriately for the wireless protocol tester.
--				Removed RequestPacket(). Added InitStateInfo().
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin, Daniel Wright
--
-- NOTES:
-- Contains physical level functions for the wireless protocol reader.
------------------------------------------------------------------------------*/

#include "Physical.h"

/*------------------------------------------------------------------------------
-- FUNCTION:    PortIOThreadProc
--
-- DATE:        Oct 13, 2010
--
-- REVISIONS:   Nov 05, 2010
--              Modified the function to also listen for a "disconnect" event,
--              and to break in that case. ProcessRead() is now called once a 
--				complete packet is confirmed (as opposed to sending the 
--				contents of the buffer to ProcessRead() as soon as they arrive).
--
--				Nov 29, 2010
--				Renamed function from ReadThreadProc(). The event handling is
--				from the original function, but the response to each event has
--				changed.
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin, Daniel Wright
--
-- INTERFACE:   DWORD WINAPI PortIOThreadProc(HWND hWnd)
--                      hWnd - the handle to the window
--
-- RETURNS:     0 because threads are required to return a DWORD.
--
-- NOTES:
--              While connected, this thread will loop and wait for characters
--              to arrive at the port, or for a timeout to occur, then call the
--				appropriate function. This function uses overlapped I/O.
------------------------------------------------------------------------------*/
DWORD WINAPI PortIOThreadProc(HWND hWnd) {
    OVERLAPPED  ol		= {0};
    DWORD       dwEvent = 0;
    DWORD       dwError = 0;
    COMSTAT     cs      = {0};
    HANDLE*     hEvents = NULL;
    PSTATEINFO  psi     = NULL;
    PWNDDATA	pwd		= (PWNDDATA) GetWindowLongPtr(hWnd, 0);
    

    if ((ol.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL) {
        DISPLAY_ERROR("Error creating event in read thread");
    }
    hEvents     = (HANDLE*) malloc(sizeof(HANDLE) * PORT_IO_EVENTS);
    hEvents[0]  = CreateEvent(NULL, FALSE, FALSE, TEXT("disconnected"));
    hEvents[1]  = ol.hEvent;
    
    psi = (PSTATEINFO) malloc(sizeof(STATEINFO));
    InitStateInfo(psi);
    DL_STATE = psi->iState;


    while (pwd->bConnected) {
        
        SetCommMask(pwd->hPort, EV_RXCHAR);      
        if (!WaitCommEvent(pwd->hPort, &dwEvent, &ol)) {
            ProcessCommError(pwd->hPort);
        }
        dwEvent = WaitForMultipleObjects(PORT_IO_EVENTS, hEvents, FALSE, 
                                         psi->dwTimeout);
        ClearCommError(pwd->hPort, &dwError, &cs);
 
        if (dwEvent == WAIT_OBJECT_0) {
            // the connection was severed
            break;
        }
        else if (dwEvent == WAIT_OBJECT_0 + 1  &&  cs.cbInQue) {
            // data arrived at the port
            ReadFromPort(hWnd, psi, ol, cs.cbInQue);
        }
        else if (dwEvent == WAIT_TIMEOUT) {
            // a timeout occured
            ProcessTimeout(hWnd, psi);
        }
        else if (dwEvent == WAIT_FAILED) {
            DISPLAY_ERROR("Invalid event occured in the Port I/O thread");
        }
        ResetEvent(ol.hEvent);
    }

    if (!PurgeComm(pwd->hPort, PURGE_RXCLEAR)) {
        DISPLAY_ERROR("Error purging read buffer");
    }
    CloseHandle(ol.hEvent);
	CloseHandle(hEvents[0]);
	free(hEvents);
    return 0;
}

/*------------------------------------------------------------------------------
-- FUNCTION:    InitStateInfo
--
-- DATE:        Nov 29, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   VOID InitStateInfo(PSTATEINFO psi)
--                      psi			- contains info about the current state of 
--									  the communication
--
-- RETURNS:     VOID.
--
-- NOTES:
--              Initializes the protocol state info to its default values.
------------------------------------------------------------------------------*/
VOID InitStateInfo (PSTATEINFO psi) {
	srand(GetTickCount());
	psi->rxSeq				= 0;
    psi->iState				= STATE_IDLE;
    psi->itoCount			= 0;
    psi->dwTimeout			= TOR0;
    psi->iFailedENQCount		= 0;    
}

/*------------------------------------------------------------------------------
-- FUNCTION:    ReadFromPort
--
-- DATE:        Nov 29, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   VOID ReadFromPort(HWND hWnd, PSTATEINFO psi, OVERLAPPED ol, 
--								  DWORD cbInQue)
--						hWnd	- a handle to the window
--                      psi		- contains info about the current state of the 
--								  communication
--						ol		- the overlapped structure used to wait for 
--								  characters at the serial port
--						cbinQue	- the number of characters that are at the port
--
-- RETURNS:     VOID.
--
-- NOTES:
--              Reads from the serial port and calls ProcessRead(). If
--				ProcessRead() returns a zero, it means that only a partial
--				frame was passed. In that case, it waits for more characters
--				to arrive at the port, then appends these new characters to the
--				the partial frame, and calls ProcessRead() again.
------------------------------------------------------------------------------*/
VOID ReadFromPort(HWND hWnd, PSTATEINFO psi, OVERLAPPED ol, DWORD cbInQue) {
    
    static DWORD    dwQueueSize             = 0;
    PWNDDATA        pwd                     = NULL;
    BYTE            pReadBuf[READ_BUFSIZE]  = {0};
	PBYTE			pQueue					= NULL;
    DWORD           dwBytesRead             = 0;
    DWORD           i                       = 0;
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);


    if (!ReadFile(pwd->hPort, pReadBuf, READ_BUFSIZE, &dwBytesRead, &ol)) {
        // read is incomplete or had an error
        ProcessCommError(pwd->hPort);
        GetOverlappedResult(pwd->hThread, &ol, &dwBytesRead, TRUE);
    }


    if (dwQueueSize == 0) {
        // the last port read sent an entire frame to ProcessRead()
        
        if (dwBytesRead >= CTRL_FRAME_SIZE  &&
			ProcessRead(hWnd, psi, pReadBuf, dwBytesRead)) {
            
			// read completed successfully
            return;
     
        } else {
            // a full frame is not yet at the port
            for (i = 0; i < dwBytesRead; i++) {
                AddToByteQueue(&pwd->pReadBufHead, &pwd->pReadBufTail, pReadBuf[i]);
				dwQueueSize++;
            }
			if (dwQueueSize != dwBytesRead) {
				DISPLAY_ERROR("Port read is out of sync");
			}
        }
    
    } else {
        // the previous port read was not finished
        
        for (i = 0; i < dwBytesRead; i++) {
            AddToByteQueue(&pwd->pReadBufHead, &pwd->pReadBufTail, pReadBuf[i]);
			dwQueueSize++;
        }
		// checks for 1 byte read, in case the tx side is out of sync
        if (dwQueueSize >= FRAME_SIZE  ||  dwBytesRead == CTRL_FRAME_SIZE) {

            pQueue = RemoveFromByteQueue(&pwd->pReadBufHead, dwQueueSize);

            ProcessRead(hWnd, psi, pQueue, dwQueueSize);
            // read completed successfully
            dwQueueSize         = 0;
            DeleteByteQueue(pwd->pReadBufHead);
            pwd->pReadBufHead   = NULL;
            pwd->pReadBufTail   = NULL;
			for (i = 0; i < dwQueueSize; i++) {
				free(pQueue);
			}
		}
    }
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
--                      hPort - the handle to the open port
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
