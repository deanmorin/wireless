/*------------------------------------------------------------------------------
-- SOURCE FILE:     List.c      Contains implementations of linked lists.
--                      
-- PROGRAM:     Dean and the Rockets' Wireless Protocol Testing and Evaluation 
--              Facilitator
--
-- FUNCTIONS:
--              BOOL    AddToByteQueue(PPBYTE_NODE, PPBYTE_NODE, BYTE);
--              BOOL    AddToFrameQueue(PPFRAME_NODE, PPFRAME_NODE, FRAME);
--              VOID    DeleteByteQueue(PBYTE_NODE);
--              PBYTE   RemoveFromByteQueue(PPBYTE_NODE, DWORD);
--              PFRAME  RemoveFromFrameQueue(PPFRAME_NODE, DWORD);
--
--
-- DATE:        Nov 24, 2010
--
-- REVISIONS:   
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- NOTES:       Implements linked lists for bytes and data frames.
------------------------------------------------------------------------------*/
#include "List.h"

/*------------------------------------------------------------------------------
-- FUNCTION:    AddToByteQueue
--
-- DATE:        Nov 24, 2010
--
-- REVISIONS:
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   AddToByteQueue(PPBYTE_NODE pHead, PPBYTE_NODE pTail,
--                             BYTE data)
--                      pHead   - the first node in the queue
--                      pTail   - the last  node in the list
--                      data    - the BYTE to be added to the list
--
-- RETURNS:     FALSE if memory allocation for the new node failed.
--
-- NOTES:       Adds the new BYTE to the back of the list.
------------------------------------------------------------------------------*/
BOOL AddToByteQueue(PPBYTE_NODE pHead, PPBYTE_NODE pTail, BYTE data) {
    PBYTE_NODE newNode  = NULL;
    
    if ((newNode = (PBYTE_NODE) malloc(sizeof(BYTE_NODE))) == NULL) {
        return FALSE;
    }
    newNode->b      = data;
    newNode->next   = NULL;

    if (*pHead == NULL) {
        *pHead = newNode;
        *pTail = newNode;
        return TRUE;
    }
    (*pTail)->next = newNode;
    *pTail         = newNode;
    return TRUE;
}

/*------------------------------------------------------------------------------
-- FUNCTION:    RemoveFromByteQueue
--
-- DATE:        Nov 24, 2010
--
-- REVISIONS:   
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   RemoveFromByteQueue(PPBYTE_NODE pHead, DWORD dwLength)
--                      pHead    - the first node in the queue
--                      dwLength - the number of nodes to remove
--
-- RETURNS:     A pointer to the first node that was removed.
--
-- NOTES:       Removes dwLength nodes from the queue. The function requires 
--              that dwLength be no longer than the list.
------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------
-- FUNCTION:    DeleteByteQueue
--
-- DATE:        Nov 24, 2010
--
-- REVISIONS:
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   DeleteByteQueue(PBYTE_NODE pHead)
--                      pHead   - the first node in the queue
--
-- RETURNS:     VOID.
--
-- NOTES:       Deletes the queue.
------------------------------------------------------------------------------*/
VOID DeleteByteQueue(PBYTE_NODE pHead) {
    PBYTE_NODE  p   = NULL;
    PBYTE_NODE  q   = NULL;
        
    for (p = pHead; p != NULL; p = q) {
        q = p->next;
        free(p);
    }
}

/*------------------------------------------------------------------------------
-- FUNCTION:    AddToFrameQueue
--
-- DATE:        Nov 24, 2010
--
-- REVISIONS:
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   AddToFrameQueue(PPFRAME_NODE pHead, PPFRAME_NODE pTail,
--                              FRAME data)
--                      pHead   - the first node in the queue
--                      pTail   - the last  node in the list
--                      data    - the FRAME to be added to the list
--
-- RETURNS:     FALSE if memory allocation for the new node failed.
--
-- NOTES:       Adds the new FRAME to the back of the list.
------------------------------------------------------------------------------*/
BOOL AddToFrameQueue(PPFRAME_NODE pHead, PPFRAME_NODE pTail, FRAME data) {
    PFRAME_NODE newNode = NULL;

    if ((newNode = (PFRAME_NODE) malloc(sizeof(FRAME_NODE))) == NULL) {
        return FALSE;
    }
	newNode->f      = data;
    newNode->next   = NULL;

    if (*pHead == NULL) {
        *pHead = newNode;
        *pTail = newNode;	
        return TRUE;
    }
    (*pTail)->next = newNode;
    *pTail         = newNode;

    return TRUE;
}

/*------------------------------------------------------------------------------
-- FUNCTION:    RemoveFromFrameQueue
--
-- DATE:        Nov 24, 2010
--
-- REVISIONS:   
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   RemoveFromFrameQueue(PPBYTE_NODE pHead, DWORD dwLength)
--                      pHead    - the first node in the queue
--                      dwLength - the number of nodes to remove
--
-- RETURNS:     A pointer to the first node that was removed.
--
-- NOTES:       Removes dwLength nodes from the queue. The function requires 
--              that dwLength be no longer than the list.
------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------
-- FUNCTION:    DeleteFrameQueue
--
-- DATE:        Nov 24, 2010
--
-- REVISIONS:
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Daniel Wright
--
-- INTERFACE:   DeleteFrameQueue(PFRAME_NODE pHead)
--                      pHead   - the first node in the queue
--
-- RETURNS:     VOID.
--
-- NOTES:       Deletes the queue.
------------------------------------------------------------------------------*/
VOID DeleteFrameQueue(PFRAME_NODE pHead) {
    PFRAME_NODE  p   = NULL;
    PFRAME_NODE  q   = NULL;
        
    for (p = pHead; p != NULL; p = q) {
        q = p->next;
        free(p);
    }
}
