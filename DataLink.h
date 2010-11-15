#ifndef DATALINK_H
#define DATALINK_H

#include <Windows.h>

#define READ_STATES 4
enum read_states    { STATE_T1, STATE_T3, STATE_IDLE, STATE_R2 };
#define STATE_T2    100
#define STATE_R1    101
#define START_R3    102
#define STATE_R4    103

#define TOR1    500
#define TOR2    500
#define TOR3    500
#define SOH		1
#define EOT		4
#define ENQ		5
#define ACK0	6
#define ACK1	17
#define RVI		18
#define WACK	19

typedef struct Frame {
	BYTE		soh;
	SHORT		length;
	BYTE*		payload;
	BYTE		crc;
} FRAME, PFRAME;


#endif
