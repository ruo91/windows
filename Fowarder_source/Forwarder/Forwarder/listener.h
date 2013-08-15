/*
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *		Copyright (C) 2006  Tommi Rouvali (tommi@rouvali.com)
 */

#include "stdafx.h"

#define RULES_T_FORWARD 1
#define RULES_T_DENY 2
#define RULES_T_EOL 100

DWORD WINAPI InvokeListenerThread(LPVOID lpParameter);

	struct rules_t
	{
		int type;
		TCHAR match[1024];
		TCHAR dest[1024];
		int destPort;
	};

class listener
{
public:
	HANDLE Thread;
	static bool init();
	static void cleanup();
	static void startListener(int xport,rules_t *rules);	
	void run();
	rules_t *rules;

protected:
	void findDestination(SOCKET socket,TCHAR **destination, int *port);
	int port;
};
