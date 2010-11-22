#ifndef STRICT
#define STRICT
#endif

#ifndef MAIN_H
#define MAIN_H

#include <Windows.h>
#include <stdio.h>
#include "Menu.h"
#include "osi.h"
#include "ErrorDetect.h"
#include "crc.h"
#pragma warning(disable:4002)
#pragma warning(disable:4996)

/*---------------------------Function Prototypes------------------------------*/
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

#endif
