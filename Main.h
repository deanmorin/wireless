/*------------------------------------------------------------------------------
-- FILE:			Main.h                 
-- PROGRAM:		Dean and the Rockets' Wireless Protocol Testing and Evaluation 
-- DATE:        Dec 03, 2010
-- DESIGNER:    Dean Morin, Ian Lee, Daniel Wright, Marcel Vangrootheest
------------------------------------------------------------------------------*/
#ifndef STRICT
#define STRICT
#endif

#ifndef MAIN_H
#define MAIN_H

#include <Windows.h>
#include <stdio.h>
#include "Menu.h"
#include "osi.h"
#include "crc.h"

/*--------------------------------Macros--------------------------------------*/
#define WND_CAPTION     TEXT("Dean and the Rockets' Wireless Protocol Testing \
and Evaluation Facilitator")

/*---------------------------Function Prototypes------------------------------*/
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

#endif
