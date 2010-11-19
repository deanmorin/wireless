#ifndef DATALINK_H
#define DATALINK_H

#include <Windows.h>
#include <math.h>
#include "crc.h"
#include "WndExtra.h"

#define FULL_BUFFER 100
#define READ_STATES 4
enum read_states    { STATE_T1, STATE_T3, STATE_IDLE, STATE_R2 };
#define STATE_T2    100
#define STATE_R1    101
#define START_R3    102
#define STATE_R4    103

#define TOR1    500
#define TOR2    500
#define TOR3    500



#define CTRL_CHAR_SIZE  1
#define FRAME_SIZE      1024
#define SOH             0x01
#define EOT             0x04
#define ENQ             0x05
#define ACK             0x06
#define RVI             0x13

typedef struct STATEINFO_tag {
    INT     iState;
    DWORD   dwTimeout;
    INT     itoCount;
} STATEINFO, *PSTATEINFO;


VOID    ProcessTimeout(PSTATEINFO psi);
VOID    ProcessWrite(HWND hWnd, PSTATEINFO psi);

VOID    ProcessRead(HWND hWnd, PSTATEINFO psi, BYTE* pReadBuf, DWORD dwLength);
VOID    ReadT1(HWND hWnd, PSTATEINFO psi, BYTE* pReadBuf, DWORD dwLength);
VOID    ReadT3(HWND hWnd, PSTATEINFO psi, BYTE* pReadBuf, DWORD dwLength);
VOID    ReadIDLE(HWND hWnd, PSTATEINFO psi, BYTE* pReadBuf, DWORD dwLength);
VOID    ReadR2(HWND hWnd, PSTATEINFO psi, BYTE* pReadBuf, DWORD dwLength);
VOID 	OpenFileReceive(HWND hWnd);
VOID 	OpenFileTransmit(HWND hWnd);
VOID	CloseFileReceive(HWND hWnd);
VOID	CloseFileTransmit(HWND hWnd);
VOID	WriteToFile(HWND hWnd, PFRAME frame);
VOID	ReadFromFile(HWND hWnd);


#endif
