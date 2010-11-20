/*------------------------------------------------------------------------------
-- SOURCE FILE:     List.c - Contains some basic functionality for a 
--                           linked-list.
--
-- PROGRAM:     RFID Reader - Enterprise Edition
--
-- FUNCTIONS:
--              DWORD   AddToBack(CHAR_LIST**, CHAR*, DWORD);
--              DWORD   GetFromList(CHAR_LIST*, UINT);
--              CHAR*   RemoveFromFront(CHAR_LIST**, DWORD); 
--
--
-- DATE:        Nov 06, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- NOTES:
-- These linked list functions are designed to process incoming characters on a 
-- buffer. The functions take or return entire strings of characters, and handle
-- the conversion to a linked list internally. More specifically, the functions
-- will treat the lists as queues.
------------------------------------------------------------------------------*/

#include "List.h"

BOOL AddByteToBack(PBYTE_NODE pHead, PBYTE_NODE pTail, BYTE data) {
    PBYTE_NODE p        = NULL;
    PBYTE_NODE newNode  = NULL;
    
    if ((newNode = (PBYTE_NODE) malloc(sizeof(BYTE_NODE))) == NULL) {
        return FALSE;
    }
    if (pHead == NULL) {
        pHead = newNode;
        pTail = newNode;
        return TRUE;
    }
    pTail->next = newNode;
    pTail       = newNode;
    return TRUE;
}


/*------------------------------------------------------------------------------
-- FUNCTION:    AddToBack
--
-- DATE:        Nov 06, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   DWORD AddToBack(CHAR_LIST** pHead, CHAR* psBuf, DWORD dwLength)
--                          pHead       - the first node in the list
--                          psBuf       - the characters to be added
--                          dwLength    - the length of psBuf
--
-- RETURNS:     The length of the linked-list.
--
-- NOTES:
--              Adds the contents of psBuf to the end of the list.
------------------------------------------------------------------------------*/

DWORD AddToBack(CHAR_LIST** pHead, CHAR* psBuf, DWORD dwLength) {
    CHAR_LIST*  newNode = NULL;
    CHAR_LIST*  p       = NULL;
    DWORD       dwCount = 0;
    DWORD       i       = 0;

    p = *pHead;

    if (p == NULL) {
        newNode         = (CHAR_LIST*) malloc(sizeof(CHAR_LIST));
        newNode->c      = psBuf[i++];
        newNode->next   = NULL;
        p               = newNode;
        *pHead			= newNode;
    } else {

        while (p->next != NULL) {
            p = p->next;
            dwCount++;
        }
    }
	dwCount++;
        
    for (i; i < dwLength; i++) {
        newNode         = (CHAR_LIST*) malloc(sizeof(CHAR_LIST));
        newNode->c      = psBuf[i];
        newNode->next   = NULL;
        p->next         = newNode;
        p               = p->next;
        dwCount++;
    }

    return dwCount;
}

/*------------------------------------------------------------------------------
-- FUNCTION:    GetFromList
--
-- DATE:        Nov 06, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   DWORD GetFromList(CHAR_LIST* p, UINT ordinal)
--                          p           - the first node in the list
--                          uiOrdinal   - the node to get the value from
--
-- RETURNS:     The value in the node specified by uiOrdinal.
--
-- NOTES:
--              Returns the value in the node specified by uiOrdinal (if
--              uiOrdinal = 2 for example, then it returns the value in the
--              2nd node in the list).
------------------------------------------------------------------------------*/
DWORD GetFromList(CHAR_LIST* p, UINT uiOrdinal) {
    UINT i = 0;

    for (i = 1; i < uiOrdinal; i++) {
        p = p->next;
    }
    return p->c;
}

/*------------------------------------------------------------------------------
-- FUNCTION:    AddToBack
--
-- DATE:        Nov 06, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   CHAR* RemoveFromFront(CHAR_LIST** pHead, DWORD dwLength)
--                          pHead       - the first node in the list
--                          dwLength    - the number of nodes to remove
--
-- RETURNS:     The contents of the first n nodes in the list, 
--              where n = dwLength
--
-- NOTES:
--              Retrieves and removes a specified amount of nodes in the form
--              of a character array.
------------------------------------------------------------------------------*/
CHAR* RemoveFromFront(CHAR_LIST** pHead, DWORD dwLength) {
    CHAR_LIST*  p       = NULL;
    CHAR_LIST*  tracer  = NULL;
    UINT        i       = 0;
    CHAR*       removed = NULL;
    
    p       = *pHead;
    removed = (CHAR*) malloc(sizeof(CHAR) * dwLength);

    for (i = 0; i < dwLength; i++) {
        removed[i]  = p->c;
        tracer      = p;
        p           = p->next;
        free(tracer);
    }
    *pHead = p;
    return removed;
}





DWORD FAddToBack(FRAME_LIST** pHead, FRAME psBuf) {
    FRAME_LIST*  newNode = NULL;
    FRAME_LIST*  p       = NULL;
    DWORD       dwCount = 0;
    

    p = *pHead;

    if (p == NULL) {
        newNode         = (FRAME_LIST*) malloc(sizeof(FRAME_LIST));
        newNode->c      = psBuf;
        newNode->next   = NULL;
        p               = newNode;
        *pHead			= newNode;
    } else {

        while (p->next != NULL) {
            p = p->next;
            dwCount++;
        }
    }
	dwCount++;
     

    return dwCount;
}


FRAME FGetFromList(FRAME_LIST* p, UINT uiOrdinal) {
    UINT i = 0;

    for (i = 1; i < uiOrdinal; i++) {
        p = p->next;
    }
    return p->c;
}

FRAME* FRemoveFromFront(FRAME_LIST** pHead, DWORD dwLength) {
    FRAME_LIST*  p       = NULL;
    FRAME_LIST*  tracer  = NULL;
    UINT        i       = 0;
    FRAME*       removed = NULL;
    
    p       = *pHead;
    removed = (FRAME*) malloc(sizeof(FRAME) * dwLength);

    for (i = 0; i < dwLength; i++) {
        removed[i]  = p->c;
        tracer      = p;
        p           = p->next;
        free(tracer);
    }
    *pHead = p;
    return removed;
}
