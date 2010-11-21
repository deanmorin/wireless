#ifndef LIST_H
#define LIST_H

#include <Windows.h>

//pragma pack allow for a structure without padding
#pragma pack(push)
#pragma pack(1)


typedef struct FRAME {
	BYTE		soh;
	BYTE		sequence;
	SHORT		length;
	BYTE*		payload;
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


BOOL AddToByteQueue(PPBYTE_NODE pHead, PPBYTE_NODE pTail, BYTE data);
PBYTE RemoveFromByteQueue(PPBYTE_NODE pHead, DWORD dwLength);
BOOL AddToFrameQueue(PPFRAME_NODE pHead, PPFRAME_NODE pTail, FRAME data);
PFRAME RemoveFromFrameQueue(PPFRAME_NODE pHead, DWORD dwLength);

#endif
