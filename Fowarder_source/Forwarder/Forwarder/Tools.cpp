/*
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *		Copyright (C) 2006  Tommi Rouvali (tommi@rouvali.com)
 */

#include "stdafx.h"
#include "tools.h"
#include <windows.h>

VOID SvcDebugOut(LPTSTR String) 
{ 
   char *buffer=UnicodeToCChar(String);

	OutputDebugStringA(buffer); 
	DWORD l;
	WaitForSingleObject(logMutex,INFINITE);
	HANDLE f=CreateFile(logFile,FILE_WRITE_DATA,FILE_SHARE_WRITE|FILE_SHARE_READ,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	SetFilePointer(f,0,0,FILE_END);
	WriteFile((HANDLE)f,buffer,(DWORD)strlen(buffer),&l,NULL);
	CloseHandle((HANDLE)f);
	ReleaseMutex(logMutex);
	delete(buffer);
}

char *UnicodeToCChar(LPTSTR str)
{
	char *buffer;
	buffer=new char[lstrlen(str)+1];

#ifdef UNICODE
	WideCharToMultiByte(CP_OEMCP,WC_COMPOSITECHECK,str,-1,buffer,lstrlen(str)+1,NULL,NULL);
#else
    StrCpy(buffer,str);
#endif
	return(buffer);
}

LPTSTR ReadRegStr(LPTSTR key)
{
		HKEY regHandle;
		WCHAR txt[1024];
		DWORD datalen=1024;
		DWORD type=REG_SZ;

		if(ERROR_SUCCESS!=RegOpenKey(HKEY_LOCAL_MACHINE,L"SOFTWARE\\TommiRouvali\\Forwarder",&regHandle))
		{
			SvcDebugOut(_T("Cannot open registry key HKEY_LOCAL_MACHINE\\SOFTWARE\\TommiRouvali\\Forwarder."));
			return(_T(""));
		}

		LONG error=RegQueryValueEx(regHandle,key,NULL,&type,(LPBYTE)txt,&datalen);
		if(ERROR_SUCCESS!=error)
		{
			SvcDebugOut(_T("Cannot read registry value.scm"));
			TCHAR errorstr[1000];
			wsprintf(errorstr,_T("%d"),error);
			SvcDebugOut(errorstr);
			return(_T(""));
		}
		LPTSTR retval=new TCHAR[lstrlen(txt)+2];
		lstrcpy(retval,txt);
		RegCloseKey(regHandle);
		return(retval);
		
}