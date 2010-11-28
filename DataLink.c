/*------------------------------------------------------------------------------
-- SOURCE FILE:     DataLink.c      Implements the half-duplex data link level
--									protocol, as designed by the class.
--                      
-- PROGRAM:     Dean and the Rockets' Wireless Protocol Testing and Evaluation 
--              Facilitator
--
-- FUNCTIONS:
--              VOID    ProcessTimeout(HWND, PSTATEINFO);
--				VOID    ProcessWrite(HWND, PBYTE, DWORD);
--				UINT    ProcessRead(HWND, PSTATEINFO, PBYTE, DWORD);
--				UINT    ReadIDLE(HWND, PSTATEINFO, PBYTE, DWORD);
--				UINT    ReadR2(HWND, PSTATEINFO psi, PBYTE, DWORD);
--				UINT    ReadT1(HWND, PSTATEINFO psi, PBYTE, DWORD);
--				UINT    ReadT3(HWND, PSTATEINFO, PBYTE, DWORD);
--				VOID    SendFrame(HWND, PSTATEINFO);
--
--
-- DATE:        Nov 28, 2010
--
-- REVISIONS:   
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- NOTES:       COMP 3890 wireless data link protocol implementation.
------------------------------------------------------------------------------*/
#include "DataLink.h"

/*------------------------------------------------------------------------------
-- FUNCTION:    ProcessWrite
--
-- DATE:        Nov 28, 2010
--
-- REVISIONS:
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   ProcessWrite(HWND hWnd, PBYTE pFrame, DWORD dwLength)
--                      hWnd			- a handle to the window
--                      pReadBuf		- a frame to write to the serial port
--						dwLength	- the length of the frame
--
-- RETURNS:     VOID.
--
-- NOTES:       Writes the frame, pFrame, to the serial port.
------------------------------------------------------------------------------*/
VOID ProcessWrite(HWND hWnd, PBYTE pFrame, DWORD dwLength) {
    OVERLAPPED  ol			= {0};
	DWORD		dwWritten	= 0;
    PWNDDATA	pwd			= (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    if (!WriteFile(pwd->hPort, pFrame, dwLength, NULL, &ol)) {
		ProcessCommError(pwd->hPort);
		GetOverlappedResult(pwd->hPort, &ol, &dwWritten, TRUE);
	}
	if (dwWritten != dwLength) {
		DISPLAY_ERROR("Full frame was not written");
	}
}

/*------------------------------------------------------------------------------
-- FUNCTION:    ProcessRead
--
-- DATE:        Nov 28, 2010
--
-- REVISIONS:
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   ProcessRead(HWND hWnd, PSTATEINFO psi, PBYTE pReadBuf, 
--							DWORD dwLength)
--                      hWnd			- a handle to the window
--                      psi			- contains info about the current state of 
--									  the communication
--                      pReadBuf		- bytes read from the serial port
--						dwLength	- the length of pReadBuf
--
-- RETURNS:     0 if an incomplete frame was passed to the read function.
--
-- NOTES:       Calls a function to process pReadBuf, based on the current
--				protocol state.
------------------------------------------------------------------------------*/
UINT ProcessRead(HWND hWnd, PSTATEINFO psi, PBYTE pReadBuf, DWORD dwLength) {  
    static UINT( *pReadFunc[READ_STATES])(HWND, PSTATEINFO, PBYTE, DWORD) 
            = { ReadT1,  ReadT3,  ReadIDLE,  ReadR2 };
    PWNDDATA pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);
    
    return pReadFunc[psi->iState](hWnd, psi, pReadBuf, dwLength);
}

/*------------------------------------------------------------------------------
-- FUNCTION:    ReadT1
--
-- DATE:        Nov 28, 2010
--
-- REVISIONS:
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   ReadT1(HWND hWnd, PSTATEINFO psi, PBYTE pReadBuf, 
--					   DWORD dwLength)
--                      hWnd			- a handle to the window
--                      psi			- contains info about the current state of 
--									  the communication
--                      pReadBuf		- bytes read from the serial port
--						dwLength	- the length of pReadBuf
--
-- RETURNS:     A non-zero value to indicate that a complete frame was passed
--				to the read function.
--
-- NOTES:       Transitions to state T3 via T2 if an ACK is received, sending
--				a frame along the way. Otherwise, returns to IDLE.
------------------------------------------------------------------------------*/
UINT ReadT1(HWND hWnd, PSTATEINFO psi, PBYTE pReadBuf, DWORD dwLength) {
    static BYTE pCtrlFrame[CTRL_FRAME_SIZE] = {0}; 
    PWNDDATA pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    if (pReadBuf[CTRL_CHAR_INDEX] == ACK) {
        PostMessage(hWnd, WM_STAT, ACK, REC);
        PostMessage(hWnd, WM_STAT, STAT_STATE, STATE_T3);
        psi->iState				= STATE_T3;
		psi->dwTimeout			= TOR2;
        psi->itoCount			= 0;
		psi->iFailedENQCount		= 0;
		SendFrame(hWnd, psi);

    } else {	// unexpected character was received
        PostMessage(hWnd, WM_STAT, STAT_STATE, STATE_IDLE);
		psi->iState     = STATE_IDLE;
        psi->dwTimeout	= TOR0;
    }
    return CTRL_FRAME_SIZE;
}

/*------------------------------------------------------------------------------
-- FUNCTION:    ReadT3
--
-- DATE:        Nov 28, 2010
--
-- REVISIONS:
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   ReadT3(HWND hWnd, PSTATEINFO psi, PBYTE pReadBuf, 
--					   DWORD dwLength)
--                      hWnd			- a handle to the window
--                      psi			- contains info about the current state of 
--									  the communication
--                      pReadBuf		- bytes read from the serial port
--						dwLength	- the length of pReadBuf
--
-- RETURNS:     A non-zero value to indicate that a complete frame was passed
--				to the read function.
--
-- NOTES:       Sends another frame if an ACK is received, or gives up the line
--				if an RVI is received.
------------------------------------------------------------------------------*/
UINT ReadT3(HWND hWnd, PSTATEINFO psi, PBYTE pReadBuf, DWORD dwLength) {
	HANDLE hMutex = 0;
    static BYTE pCtrlFrame[CTRL_FRAME_SIZE] = {0}; 
    PWNDDATA pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    if (pReadBuf[CTRL_CHAR_INDEX] == ACK) {
		
		hMutex = CreateMutex(NULL, FALSE, TEXT("FTPMutex"));
		RemoveFromFrameQueue(&pwd->FTPBuffHead, 1);
		ReleaseMutex(hMutex);

		PostMessage(hWnd, WM_FILLFTPBUF, 0, 0);    
        PostMessage(hWnd, WM_STAT, ACK, REC);
        PostMessage(hWnd, WM_STAT, STAT_FRAMEACKD, SENT);
        SendFrame(hWnd, psi);

    } else if (pReadBuf[CTRL_CHAR_INDEX] == RVI) {

		hMutex = CreateMutex(NULL, FALSE, TEXT("FTPMutex"));
		RemoveFromFrameQueue(&pwd->FTPBuffHead, 1);
		ReleaseMutex(hMutex);

		pCtrlFrame[CTRL_CHAR_INDEX] = ACK;
        ProcessWrite(hWnd, pCtrlFrame, CTRL_FRAME_SIZE);
		PostMessage(hWnd, WM_FILLFTPBUF, 0, 0);
		PostMessage(hWnd, WM_STAT, STAT_FRAMEACKD, SENT);
		PostMessage(hWnd, WM_STAT, RVI, REC);
        PostMessage(hWnd, WM_STAT, ACK, SENT);
        PostMessage(hWnd, WM_STAT, STAT_STATE, STATE_R2);
		psi->iState     = STATE_R2;
        psi->dwTimeout  = TOR3;
        psi->itoCount   = 0;

    } else {
		PostMessage(hWnd, WM_STAT, STAT_STATE, STATE_IDLE);
		psi->iState = STATE_IDLE;
	}
    return CTRL_FRAME_SIZE;
}

/*------------------------------------------------------------------------------
-- FUNCTION:    ReadIDLE
--
-- DATE:        Nov 28, 2010
--
-- REVISIONS:
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   ReadIDLE(HWND hWnd, PSTATEINFO psi, PBYTE pReadBuf, 
--						 DWORD dwLength)
--                      hWnd			- a handle to the window
--                      psi			- contains info about the current state of 
--									  the communication
--                      pReadBuf		- bytes read from the serial port
--						dwLength	- the length of pReadBuf
--
-- RETURNS:     A non-zero value to indicate that a complete frame was passed
--				to the read function.
--
-- NOTES:       ACKs an ENQ, if one is received.
------------------------------------------------------------------------------*/
UINT ReadIDLE(HWND hWnd, PSTATEINFO psi, PBYTE pReadBuf, DWORD dwLength) {
    static BYTE pCtrlFrame[CTRL_FRAME_SIZE] = {0}; 
    PWNDDATA pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    if (pReadBuf[CTRL_CHAR_INDEX] == ENQ) {
        pCtrlFrame[CTRL_CHAR_INDEX] = ACK;
        ProcessWrite(hWnd, pCtrlFrame, CTRL_FRAME_SIZE);
        PostMessage(hWnd, WM_STAT, ACK, SENT);
		PostMessage(hWnd, WM_STAT, STAT_STATE, STATE_R2);
        psi->iState     = STATE_R2;
        psi->dwTimeout  = TOR3;
    }
    return CTRL_FRAME_SIZE;
}

/*------------------------------------------------------------------------------
-- FUNCTION:    ReadR2
--
-- DATE:        Nov 28, 2010
--
-- REVISIONS:
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   ReadR2(HWND hWnd, PSTATEINFO psi, PBYTE pReadBuf, 
--					   DWORD dwLength)
--                      hWnd			- a handle to the window
--                      psi			- contains info about the current state of 
--									  the communication
--                      pReadBuf		- bytes read from the serial port
--						dwLength	- the length of pReadBuf
--
-- RETURNS:     0 if an incomplete frame was passed to the read function.
--
-- NOTES:       Expects to receive either an EOT or a data frame. If a data
--				frame is received it is checked for correct sequence number and
--				CRC, then ACK'd or RVI'd, depending on whether this side has
--				anything to send. The function returns 0 if a partial data
--				frame is received.
------------------------------------------------------------------------------*/
UINT ReadR2(HWND hWnd, PSTATEINFO psi, PBYTE pReadBuf, DWORD dwLength) {
	HANDLE hMutex = 0;
    static BYTE pCtrlFrame[CTRL_FRAME_SIZE] = {0};
    PWNDDATA pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);
	
    if (pReadBuf[CTRL_CHAR_INDEX] == EOT) {
        PostMessage(hWnd, WM_STAT, EOT, REC);
        PostMessage(hWnd, WM_STAT, STAT_STATE, STATE_IDLE);
		psi->iState		= STATE_IDLE;
        psi->dwTimeout	= TOR0;
        return CTRL_FRAME_SIZE;
    }
    if (pReadBuf[0] != SOH) {		// an unexpected frame arrived
        PostMessage(hWnd, WM_STAT, STAT_STATE, STATE_IDLE);
		psi->iState		= STATE_IDLE;
        return INVALID_FRAME;
    }
    if (dwLength < FRAME_SIZE) {
        return UNFINISHED_FRAME;	// a full frame has not arrived at the port yet
    }
	PostMessage(hWnd, WM_STAT, STAT_FRAME, REC);

	if (crcFast(pReadBuf, dwLength) == 0  &&  pReadBuf[1] == psi->rxSeq) {

		hMutex = CreateMutex(NULL, FALSE, TEXT("PTFMutex"));
		AddToFrameQueue(&pwd->PTFBuffHead, &pwd->PTFBuffTail, *((PFRAME) pReadBuf));
		ReleaseMutex(hMutex);

		PostMessage(hWnd, WM_EMPTYPTFBUF, 0, 0);
		PostMessage(hWnd, WM_STAT, STAT_FRAMEACKD, REC);

        if (pwd->FTPQueueSize) {
            pCtrlFrame[CTRL_CHAR_INDEX] = RVI;
            ProcessWrite(hWnd, pCtrlFrame, CTRL_FRAME_SIZE);
            PostMessage(hWnd, WM_STAT, RVI, SENT);		            
            PostMessage(hWnd, WM_STAT, STAT_STATE, STATE_T1);
			psi->iState = STATE_T1;
        } else {
            pCtrlFrame[CTRL_CHAR_INDEX] = ACK;
            ProcessWrite(hWnd, pCtrlFrame, CTRL_FRAME_SIZE);
            PostMessage(hWnd, WM_STAT, ACK, SENT);
        }
		psi->rxSeq = (psi->rxSeq + 1) % 2;
    }
    return FRAME_SIZE;
}

/*------------------------------------------------------------------------------
-- FUNCTION:    SendFrame
--
-- DATE:        Nov 28, 2010
--
-- REVISIONS:
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   SendFrame(HWND hWnd, PSTATEINFO psi)
--                      hWnd		- a handle to the window
--                      psi		- contains info about the current state of 
--								  the communication
--
-- RETURNS:     VOID.
--
-- NOTES:       Sends a data frame if there is one in the file-to-port queue,
--				otherwise sends an EOT.
------------------------------------------------------------------------------*/
VOID SendFrame(HWND hWnd, PSTATEINFO psi) {
	HANDLE hMutex = 0;
    static BYTE pCtrlFrame[CTRL_FRAME_SIZE] = {0}; 
    PWNDDATA pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    if (pwd->FTPQueueSize == 0) {
        pCtrlFrame[CTRL_CHAR_INDEX] = EOT;
        ProcessWrite(hWnd, pCtrlFrame, CTRL_FRAME_SIZE);
        PostMessage(hWnd, WM_STAT, EOT, SENT);
        PostMessage(hWnd, WM_STAT, STAT_STATE, STATE_IDLE);
		psi->iState     = STATE_IDLE;
		psi->dwTimeout  = TOR0;

    } else {
		hMutex = CreateMutex(NULL, FALSE, TEXT("FTPMutex"));
        ProcessWrite(hWnd, (PBYTE) &pwd->FTPBuffHead->f, FRAME_SIZE);
		ReleaseMutex(hMutex);

        PostMessage(hWnd, WM_STAT, STAT_FRAME, SENT);
    }
}

/*------------------------------------------------------------------------------
-- FUNCTION:    ProcessTimeout
--
-- DATE:        Nov 28, 2010
--
-- REVISIONS:
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   ProcessTimeout(HWND hWnd, PSTATEINFO psi)
--                      hWnd		- a handle to the window
--                      psi		- contains info about the current state of 
--								  the communication
--
-- RETURNS:     VOID.
--
-- NOTES:       Sets the new protocol state and timeout, based on the current 
--				protocol state. If the timeout occured in state T1, it checks
--				to see if the program's ENQ haven't been answered for a while
--				and disconnects if this is the case.
------------------------------------------------------------------------------*/
VOID ProcessTimeout(HWND hWnd, PSTATEINFO psi) {
    static BYTE pCtrlFrame[CTRL_FRAME_SIZE] = {0}; 
    PWNDDATA pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    switch (psi->iState) {
        
        case STATE_IDLE:
            pCtrlFrame[CTRL_CHAR_INDEX] = ENQ;
            ProcessWrite(hWnd, pCtrlFrame, CTRL_FRAME_SIZE);
            psi->dwTimeout  = TOR1;
            psi->iState     = STATE_T1;
            PostMessage(hWnd, WM_STAT, STAT_STATE, STATE_T1);
            return;

        case STATE_T1:
            if (++(psi->iFailedENQCount) >= MAX_FAILED_ENQS) {
				PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_DISCONNECT, 0), 0);
                DISPLAY_ERROR("Connection can not be established.\nDisconnecting..."); 
            }
            psi->dwTimeout  = TOR0;
            psi->iState     = STATE_IDLE;
            PostMessage(hWnd, WM_STAT, STAT_STATE, STATE_IDLE);
            return;
        
        case STATE_T3:
            psi->dwTimeout *= TOR2_INCREASE_FACTOR;

            if (++(psi->itoCount) >= MAX_TIMEOUTS) {
                psi->dwTimeout  = TOR0;
                psi->iState     = STATE_IDLE;
            } else { 
                SendFrame(hWnd, psi);
            }
            PostMessage(hWnd, WM_STAT, STAT_STATE, psi->iState);
            return;
        
        case STATE_R2:
            psi->dwTimeout *= TOR3_INCREASE_FACTOR;

            if (++(psi->itoCount) >= MAX_TIMEOUTS) {
                psi->dwTimeout  = TOR0;
                psi->iState     = STATE_IDLE;
                PostMessage(hWnd, WM_STAT, STAT_STATE, STATE_IDLE);
            } 
            return;
        
        default:
            DISPLAY_ERROR("Invalid state for timeout");
            return;
    }
}
