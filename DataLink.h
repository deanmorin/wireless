#ifndef DATALINK_H
#define DATALINK_H
#include <Windows.h>
#include <math.h>
#include "Main.h"

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
#define ENQ             0x05
#define ACK             "\0x06"

typedef struct STATEINFO_tag {
    INT     iState;
    DWORD   dwTimeout;
    INT     iTOCount;
} STATEINFO, *PSTATEINFO;


VOID    ProcessTimeout(PSTATEINFO psi);
VOID    ProcessWrite(HWND hWnd, PSTATEINFO psi);
VOID    ReadT1(HWND hWnd, PSTATEINFO psi);
VOID    ReadT3(HWND hWnd, PSTATEINFO psi);
VOID    ReadIDLE(HWND hWnd, PSTATEINFO psi);
VOID    ReadR2(HWND hWnd, PSTATEINFO psi);

#endif
