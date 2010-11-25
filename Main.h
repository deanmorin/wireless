/*------------------------------------------------------------------------------
-- SOURCE FILE:     Main.c      Contains the WinMain() and WinProc() functions 
--                              for the program.
--                      
-- PROGRAM:     Dean and the Rockets' Wireless Protocol Testing and Evaluation 
--              Facilitator
--
-- FUNCTIONS:
--              int     WINAPI WinMain(HINSTANCE, HINSTANCE, PSTR, int)
--              LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM) 
--
--
-- DATE:        Nov 24, 2010
--
-- REVISIONS:   
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- NOTES:       The main entry point for the program.
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
#include "ErrorDetect.h"
#include "crc.h"
#pragma warning(disable:4002)
#pragma warning(disable:4996)

/*--------------------------------Macros--------------------------------------*/
#define WND_CAPTION     TEXT("Dean and the Rockets' Wireless Protocol Testing \
and Evaluation Facilitator")

/*---------------------------Function Prototypes------------------------------*/
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

#endif
