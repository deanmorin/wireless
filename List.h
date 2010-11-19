#ifndef LIST_H
#define LIST_H

#include <Windows.h>

typedef struct CHAR_LIST CHAR_LIST;
typedef struct CHAR_LIST {
    CHAR        c;
    CHAR_LIST*  next;
} CHAR_LIST;

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


typedef struct FRAME_LIST FRAME_LIST;
typedef struct FRAME_LIST {
    FRAME        c;
    FRAME_LIST*  next;
} FRAME_LIST;



DWORD AddToBack(CHAR_LIST** p, CHAR* psBuf, DWORD dwLength);
DWORD GetFromList(CHAR_LIST* p, UINT ordinal);
CHAR* RemoveFromFront(CHAR_LIST** p, DWORD dwLength);

#endif
