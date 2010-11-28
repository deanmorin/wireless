#ifndef DATALINK_H
#define DATALINK_H

#include <Windows.h>
#include <math.h>
#include "crc.h"
#include "Debug.h"
#include "WndExtra.h"

/*--------------------------------Macros--------------------------------------*/
#define FULL_BUFFER     32
#define MAX_FAILED_ENQS 100
#define READ_STATES     4
#define PORT_IO_EVENTS  2

// protocol states
enum read_states    { STATE_T1, STATE_T3, STATE_IDLE, STATE_R2 };
#define STATE_T2    100
#define STATE_R1    101
#define STATE_R3    102
#define STATE_R4    103

// timeout values
#define TOR0_BASE               1001
#define TOR0_RANGE              500
#define TOR0					TOR0_BASE + rand() % TOR0_RANGE
#define TOR1					1000
#define TOR2                    1000
#define TOR3                    1000
#define MAX_TIMEOUTS            3
#define TOR2_INCREASE_FACTOR    2
#define TOR3_INCREASE_FACTOR    2

// frame components
#define CTRL_FRAME_SIZE     1
#define CTRL_CHAR_INDEX     0
#define UNFINISHED_FRAME	0
#define INVALID_FRAME       99

// control characters
#define SOH 0x01
#define EOT 0x04
#define ENQ 0x05
#define ACK 0x06
#define RVI 0x13

// messages and message parameters
#define SENT    100
#define REC     101
#define STAT_STATE			299
#define STAT_FRAME			300
#define STAT_FRAMEACKD		301
#define STAT_FILE			302
#define WM_STAT				WM_USER+1
#define WM_FILLFTPBUF		WM_USER+2
#define WM_EMPTYPTFBUF		WM_USER+3


/*---------------------------Function Prototypes------------------------------*/
VOID    ProcessTimeout(HWND hWnd, PSTATEINFO psi);
VOID    ProcessWrite(HWND hWnd, BYTE* pFrame, DWORD dwLength);
UINT    ProcessRead(HWND hWnd, PSTATEINFO psi, BYTE* pReadBuf, DWORD dwLength);
UINT    ReadIDLE(HWND hWnd, PSTATEINFO psi, BYTE* pReadBuf, DWORD dwLength);
UINT    ReadR2(HWND hWnd, PSTATEINFO psi, BYTE* pReadBuf, DWORD dwLength);
UINT    ReadT1(HWND hWnd, PSTATEINFO psi, BYTE* pReadBuf, DWORD dwLength);
UINT    ReadT3(HWND hWnd, PSTATEINFO psi, BYTE* pReadBuf, DWORD dwLength);
VOID    SendFrame(HWND hWnd, PSTATEINFO psi);

#endif
