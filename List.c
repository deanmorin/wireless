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

BOOL AddToByteQueue(PPBYTE_NODE pHead, PPBYTE_NODE pTail, BYTE data) {
    PBYTE_NODE newNode  = NULL;
    
    if ((newNode = (PBYTE_NODE) malloc(sizeof(BYTE_NODE))) == NULL) {
        return FALSE;
    }
    newNode->b = data;

    if (*pHead == NULL) {
        *pHead = newNode;
        *pTail = newNode;
        return TRUE;
    }
    (*pTail)->next = newNode;
    *pTail         = newNode;
    return TRUE;
}


PBYTE RemoveFromByteQueue(PPBYTE_NODE pHead, DWORD dwLength) {
    PBYTE_NODE  p       = NULL;
    PBYTE_NODE  tracer  = NULL;
    UINT        i       = 0;
    PBYTE       removed = NULL;
    
    p       = *pHead;
    removed = (PBYTE) malloc(sizeof(BYTE) * dwLength);

    for (i = 0; i < dwLength; i++) {
        removed[i]  = p->b;
        tracer      = p;
        p           = p->next;
        free(tracer);
    }
    *pHead = p;
    return removed;
}


VOID DeleteByteQueue(PBYTE_NODE pHead) {
    PBYTE_NODE  p   = NULL;
    PBYTE_NODE  q   = NULL;
        
    for (p = pHead; p != NULL; p = q) {
        q = p->next;
        free(p);
    }
}


BOOL AddToFrameQueue(PPFRAME_NODE pHead, PPFRAME_NODE pTail, FRAME data) {
    PFRAME_NODE newNode  = NULL;
    
    if ((newNode = (PFRAME_NODE) malloc(sizeof(FRAME_NODE))) == NULL) {
        return FALSE;
    }
	newNode->f = data;
    if (*pHead == NULL) {

        *pHead = newNode;
        *pTail = newNode;
		
        return TRUE;
    }
    (*pTail)->next = newNode;
    *pTail         = newNode;
    return TRUE;
}


PFRAME RemoveFromFrameQueue(PPFRAME_NODE pHead, DWORD dwLength) {
    PFRAME_NODE p       = NULL;
    PFRAME_NODE tracer  = NULL;
    UINT        i       = 0;
    PFRAME      removed = NULL;
    
    p       = *pHead;
    removed = (PFRAME) malloc(sizeof(FRAME) * dwLength);

    for (i = 0; i < dwLength; i++) {
        removed[i]  = p->f;
        tracer      = p;
        p           = p->next;
        free(tracer);
    }
    *pHead = p;
    return removed;
}
