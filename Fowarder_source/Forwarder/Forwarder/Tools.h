/*
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *		Copyright (C) 2006  Tommi Rouvali (tommi@rouvali.com)
 */


#include "stdafx.h"

extern HANDLE logMutex;
extern LPTSTR logFile;

VOID SvcDebugOut(LPTSTR String); 
char *UnicodeToCChar(LPTSTR str);
LPTSTR ReadRegStr(LPTSTR key);