/*
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *		Copyright (C) 2006  Tommi Rouvali (tommi@rouvali.com)
 */


#include "stdafx.h"

#define BUF_SIZE 1024

DWORD WINAPI InvokeConnectionThread(LPVOID lpParameter);

class connection
{
public:
	TCHAR *destination;
	unsigned int destPort;
	SOCKET inSock;
	SOCKET outSock;
	HANDLE inThread;
	HANDLE outThread;

	void run();

	struct invokeParam_t
	{
		connection *obj;
		int method;
	};
	void handleIncoming();
	void handleOutgoing();

protected:
	bool timeToDie;
	bool openConnectionToRemote();
	invokeParam_t IncomingParam;
	invokeParam_t OutgoingParam;
};

