/*
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *		Copyright (C) 2006  Tommi Rouvali (tommi@rouvali.com)
 */

#pragma once


#define WIN32_LEAN_AND_MEAN		
#include <stdio.h>
#include <tchar.h>
#include "windows.h"
#include "winsock2.h"
#include "listener.h"
#include "connection.h"
#include "tools.h"
#include "boost/regex.h"

void ReadConfig();

