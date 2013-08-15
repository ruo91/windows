/*
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *		Copyright (C) 2006  Tommi Rouvali (tommi@rouvali.com)
 */

#include "stdafx.h"
#include <windows.h>
#include "shlwapi.h"

SERVICE_STATUS          ForwarderServiceStatus; 
SERVICE_STATUS_HANDLE   ForwarderServiceStatusHandle; 

HANDLE logMutex;
LPTSTR logFile;

VOID WINAPI ForwarderServiceCtrlHandler(DWORD fdwControl);
DWORD ForwarderServiceInitialization(DWORD   argc, LPTSTR  *argv, DWORD *specificError);

void WINAPI ForwarderServiceStart (DWORD argc, LPTSTR *argv) 
{ 
    DWORD status; 
    DWORD specificError; 

	logMutex=CreateMutex(NULL,FALSE,NULL);
	logFile=ReadRegStr(_T("logFile"));

	SvcDebugOut(_T("\r\nForwarder Service Started\r\n"));

    ForwarderServiceStatus.dwServiceType        = SERVICE_WIN32; 
    ForwarderServiceStatus.dwCurrentState       = SERVICE_START_PENDING; 
    ForwarderServiceStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP | 
        SERVICE_ACCEPT_PAUSE_CONTINUE; 
    ForwarderServiceStatus.dwWin32ExitCode      = 0; 
    ForwarderServiceStatus.dwServiceSpecificExitCode = 0; 
    ForwarderServiceStatus.dwCheckPoint         = 0; 
    ForwarderServiceStatus.dwWaitHint           = 0; 

    ForwarderServiceStatusHandle = RegisterServiceCtrlHandler( 
        _T("fwdsrv"), 
        ForwarderServiceCtrlHandler); 
 
    if (ForwarderServiceStatusHandle == (SERVICE_STATUS_HANDLE)0) 
    { 
        SvcDebugOut(_T(" [fwdsrv] RegisterServiceCtrlHandler failed\r\n")); 
        return; 
    } 
 
    // Initialization code goes here. 
    status = ForwarderServiceInitialization(argc,argv, &specificError); 
 
    // Handle error condition 

    if (status != NO_ERROR) 
    { 
        ForwarderServiceStatus.dwCurrentState       = SERVICE_STOPPED; 
        ForwarderServiceStatus.dwCheckPoint         = 0; 
        ForwarderServiceStatus.dwWaitHint           = 0; 
        ForwarderServiceStatus.dwWin32ExitCode      = status; 
        ForwarderServiceStatus.dwServiceSpecificExitCode = specificError; 
 
        SetServiceStatus (ForwarderServiceStatusHandle, &ForwarderServiceStatus); 
        return; 
    } 
 
    // Initialization complete - report running status. 
    ForwarderServiceStatus.dwCurrentState       = SERVICE_RUNNING; 
    ForwarderServiceStatus.dwCheckPoint         = 0; 
    ForwarderServiceStatus.dwWaitHint           = 0; 
 
    if (!SetServiceStatus (ForwarderServiceStatusHandle, &ForwarderServiceStatus)) 
    { 
        status = GetLastError(); 
        SvcDebugOut(_T(" [fwdsrv] SetServiceStatus error\r\n")); 
    } 

	listener::init();

	ReadConfig();

    return; 
} 
 
// Stub initialization function. 
DWORD ForwarderServiceInitialization(DWORD   argc, LPTSTR  *argv, DWORD *specificError) 
{ 
    argv; 
    argc; 
    specificError; 

    return(0); 
}

VOID WINAPI ForwarderServiceCtrlHandler(DWORD fdwControl)
{
	if(fdwControl==SERVICE_CONTROL_STOP)
	{
        ForwarderServiceStatus.dwCurrentState       = SERVICE_STOPPED; 
        ForwarderServiceStatus.dwCheckPoint         = 0; 
        ForwarderServiceStatus.dwWaitHint           = 0; 
        ForwarderServiceStatus.dwWin32ExitCode      = 0; 
        ForwarderServiceStatus.dwServiceSpecificExitCode = 0; 
 
        SetServiceStatus (ForwarderServiceStatusHandle, &ForwarderServiceStatus); 
	}
}

void main( ) 
{ 
   SERVICE_TABLE_ENTRY   DispatchTable[] = 
   { 
      { _T("ForwarderService"), ForwarderServiceStart      }, 
      { NULL,              NULL          } 
   }; 
   if (!StartServiceCtrlDispatcher( DispatchTable)) 
   { 
//      SvcDebugOut(_T(" [MY_SERVICE] StartServiceCtrlDispatcher\r\n")); 
   } 
} 
 