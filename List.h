/*------------------------------------------------------------------------------
-- FILE:			List.h                 
-- PROGRAM:		Dean and the Rockets' Wireless Protocol Testing and Evaluation 
-- DATE:        Dec 03, 2010
-- DESIGNER:    Dean Morin, Ian Lee, Daniel Wright, Marcel Vangrootheest
------------------------------------------------------------------------------*/
#ifndef LIST_H
#define LIST_H

#include <Windows.h>

/*--------------------------------Macros--------------------------------------*/
#define MAX_PAYLOAD_SIZE	1019
#define FRAME_SIZE			1024

/*-------------------------------Structures-----------------------------------*/
//pragma pack allow for a structure without padding
#pragma pack(push)
#pragma pack(1)
typedef struct FRAME {
    BYTE		soh;
    BYTE		sequence;
    SHORT		length;
    BYTE		payload[MAX_PAYLOAD_SIZE];
    BYTE		crc;
} FRAME, *PFRAME;
#pragma pack(pop)

typedef struct BYTE_NODE BYTE_NODE;
typedef struct BYTE_NODE {
    BYTE        b;
    BYTE_NODE*  next;
} BYTE_NODE, *PBYTE_NODE, **PPBYTE_NODE;

typedef struct FRAME_NODE FRAME_NODE;
typedef struct FRAME_NODE {
    FRAME       f;
    FRAME_NODE* next;
} FRAME_NODE, *PFRAME_NODE, **PPFRAME_NODE;


/*---------------------------Function Prototypes------------------------------*/
BOOL    AddToByteQueue(PPBYTE_NODE pHead, PPBYTE_NODE pTail, BYTE data);
BOOL    AddToFrameQueue(PPFRAME_NODE pHead, PPFRAME_NODE pTail, FRAME data);
VOID    DeleteByteQueue(PBYTE_NODE pHead);
VOID	DeleteFrameQueue(PFRAME_NODE pHead);
PBYTE   RemoveFromByteQueue(PPBYTE_NODE pHead, DWORD dwLength);
PFRAME  RemoveFromFrameQueue(PPFRAME_NODE pHead, DWORD dwLength);

#endif
