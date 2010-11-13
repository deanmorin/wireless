#include "ErrorDetect.h"

/*---------------------------------------------------------------
-- SOURCE FILE:    ErrorDetect.c
--
-- PROGRAM:     RFID Reader - Enterprise Edition
--
-- FUNCTIONS:
--              BOOL DetectLRCError(CHAR* pcPacket, DWORD dwLength)
--
--
-- DATE:        Nov 2, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Dean Morin, Marcel Vangrootheest
--
-- PROGRAMMER:	Ian Lee
--
-- NOTES:
--
--				Handles the LRC error checking on frames passed into the program
--
----------------------------------------------------------------*/

/*---------------------------------------------------------------
--FUNCTION: 	DetectLRCError
--
--DATE:			Nov 2, 2010
--
--REVISIONS:	(Date and Description)
--
--DESIGNER:		Dean Morin, Marcel Vangrootheest
--
--Programer:	Ian Lee
--
--INTERFACE:	BOOL DetectLRCError(CHAR* pcPacket, DWORD dwLength)
--									pcPacket	- Frame to be processed
--									dwLength	- number of Bytes in frame
--
--
--RETURNS:		BOOL	true if an error is detected
--
--NOTES:
--
--				Performs LRC Error checking on the frame.  

				1. XOR the first byte with the second byte, then the result of 
					that with the third byte, and so on until all bytes except 
					for the final two have been XOR’d. 
				2. If the result of this is not the same as the second to last 
					byte: 
					A. return true 
				3. XOR the second to last byte with ‘FF’ 
				4. If the result of this is not the same as the last byte: 
					A. return true 
				5. return false 

----------------------------------------------------------------*/
BOOL DetectLRCError(CHAR* pcPacket, DWORD dwLength){
	DWORD i;
	
	char sum = 0x0;
	char ff = (char)0xFF;
	for(i = 0;i<dwLength - 2; i++){
		sum = sum ^ pcPacket[i];
	}
	if(sum != pcPacket[i++])
		return TRUE;
	
	sum = sum^ff;
	if(sum!= pcPacket[i++]){
		return TRUE;
	}
	return FALSE;
}
