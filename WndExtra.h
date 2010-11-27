#ifndef WNDEXTRA_H
#define WNDEXTRA_H

#include <Windows.h>
#include "List.h"
/*--------------------------------Macros--------------------------------------*/
#define ASCII_DIGIT_OFFSET  48      // the ascii value for '0'
#define PADDING             10      // the distance between the edge of the
                                    // client area, and any text
#define NO_OF_PORTS         9       // the number of ports available from the
                                    // "Select Ports" dropdown
#define CHARS_PER_LINE      80      // characters per line
#define LINES_PER_SCRN      24      // lines per screen
#define TIMER				3003	// the id for timer

#define DISPLAY_ERROR(x)    MessageBox(NULL, TEXT(x), TEXT(""), MB_OK)
#define X                   pwd->displayBuf.cxCursor
#define Y                   pwd->displayBuf.cyCursor
#define X_POS               pwd->displayBuf.cxCursor * pwd->displayBuf.cxChar \
                                                       + PADDING
#define Y_POS               pwd->displayBuf.cyCursor * pwd->displayBuf.cyChar \
                                                       + PADDING
#define CHAR_WIDTH          pwd->displayBuf.cxChar
#define CHAR_HEIGHT         pwd->displayBuf.cyChar
#define CHARACTER(x, y)     pwd->displayBuf.rows[y]->columns[x]
#define SET_BUFFER(c, x, y) pwd->displayBuf.rows[y]->columns[x].character = c;
#define ROW(y)              pwd->displayBuf.rows[y]
#define CUR_FG_COLOR        pwd->displayBuf.fgColor
#define CUR_BG_COLOR        pwd->displayBuf.bgColor
#define CUR_STYLE           pwd->displayBuf.style
#define BRIGHTNESS			pwd->displayBuf.brightness
#define ESC_VAL(x)          pwd->dwEscSeqValues[x]
#define WINDOW_TOP          pwd->cyWindowTop
#define WINDOW_BOTTOM       pwd->cyWindowBottom

#define DL_STATE            pwd->statsInfo.dlState
#define NUM_FILES           pwd->statsInfo.numFiles
#define UP_FRAMES           pwd->statsInfo.upFrames
#define DOWN_FRAMES         pwd->statsInfo.downFrames
#define UP_FRAMES_ACKD      pwd->statsInfo.upFramesACKd
#define DOWN_FRAMES_ACKD    pwd->statsInfo.downFramesACKd
#define SENT_ACK            pwd->statsInfo.sentACK
#define REC_ACK             pwd->statsInfo.recACK
#define SENT_EOT            pwd->statsInfo.sentEOT
#define REC_EOT             pwd->statsInfo.recEOT
#define SENT_RVI            pwd->statsInfo.sentRVI
#define REC_RVI             pwd->statsInfo.recRVI


/*-------------------------------Structures-----------------------------------*/
typedef struct STATEINFO_tag {
    INT     iState;
    DWORD   dwTimeout;
    INT     itoCount;
    INT     iFailedENQCount;
} STATEINFO, *PSTATEINFO;

typedef struct charInfo {
    CHAR    character;
    BYTE    fgColor;
    BYTE    bgColor;
    BYTE    style;
} CHARINFO;

typedef struct statsInfo {
    INT dlState;
	INT	numFiles;
	INT upFrames;
	INT downFrames;
    INT upFramesACKd;
    INT downFramesACKd;
	INT sentACK;
	INT recACK;
	INT sentEOT;
	INT recEOT;
	INT sentRVI;
	INT recRVI;
} STATSINFO;

typedef struct line {
    CHARINFO    columns[CHARS_PER_LINE];
} LINE, *PLINE;

typedef struct displayBuf {
    PLINE   rows[LINES_PER_SCRN];
    UINT    cxChar;
    UINT    cyChar;
    INT     cxCursor;
    INT     cyCursor;
    HFONT	hFont;
	BYTE    fgColor;
    BYTE    bgColor;
    BYTE    style;
	BYTE	brightness;
} DISPLAYBUF;

typedef struct wndData {
	HWND			hDlgStats;
	HWND			hDlgDebug;
    HANDLE          hPort;
	HANDLE			hFileReceive;
	HANDLE			hFileTransmit;
	LPTSTR          lpszCommName;
    COMMCONFIG      cc;
	BOOL			bMoreData;
    BOOL            bConnected;
    HANDLE          hThread;
	HANDLE			hFileThread;
    DWORD           dwThreadid;
    COMMTIMEOUTS    defaultTimeOuts;
    INT             cyWindowTop;
    INT             cyWindowBottom;
	BOOL			relOrigin;
	STATSINFO		statsInfo;
    INT             FTPQueueSize;
    INT             PTFQueueSize;
	PFRAME_NODE		FTPBuffHead;
	PFRAME_NODE		PTFBuffHead;
	PFRAME_NODE		FTPBuffTail;
	PFRAME_NODE		PTFBuffTail;
	INT				NumOfReads;
	BYTE			TxSequenceNumber;
	BYTE			RxSequenceNumber;
    PBYTE_NODE      pReadBufHead;
    PBYTE_NODE      pReadBufTail;
    BOOL            bDebug;
    DISPLAYBUF      displayBuf;
    BOOL            wordWrap;
} WNDDATA, *PWNDDATA;

#endif
