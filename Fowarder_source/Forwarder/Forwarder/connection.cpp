/*
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *		Copyright (C) 2006  Tommi Rouvali (tommi@rouvali.com)
 */

#include "stdafx.h"

void connection::run()
{
	timeToDie=false;

	if(openConnectionToRemote())
	{
		IncomingParam.obj=this;
		IncomingParam.method=0;
		inThread=CreateThread(NULL,0,InvokeConnectionThread,(LPVOID)&IncomingParam,0,NULL);

		OutgoingParam.obj=this;
		OutgoingParam.method=1;
		outThread=CreateThread(NULL,0,InvokeConnectionThread,(LPVOID)&OutgoingParam,0,NULL);
	}
}

bool connection::openConnectionToRemote()
{
  outSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (outSock == INVALID_SOCKET) 
  {
		TCHAR buffer[1000];
		wsprintf(buffer,_T("openConnectionToRemote: Error at socket(): %ld\r\n"),WSAGetLastError());
		SvcDebugOut(buffer);
		return(false);
  }

   char *buf=UnicodeToCChar(destination);

  sockaddr_in clientService; 
  clientService.sin_family = AF_INET;
  clientService.sin_addr.s_addr = inet_addr( buf );
  clientService.sin_port = htons( destPort );

  if ( connect( outSock, (SOCKADDR*) &clientService, sizeof(clientService) ) == SOCKET_ERROR)
  {
	  TCHAR buffer[1000];
	  wsprintf(buffer,_T("Failed to connect. With error %d\r\n"),WSAGetLastError());
	  SvcDebugOut(buffer);
	  wsprintf(buffer,_T("IP address was %d.%d.%d.%d port was %d\r\n"),clientService.sin_addr.S_un.S_un_b.s_b1,clientService.sin_addr.S_un.S_un_b.s_b2,clientService.sin_addr.S_un.S_un_b.s_b3,clientService.sin_addr.S_un.S_un_b.s_b4,destPort);
	  SvcDebugOut(buffer);
	  delete(buf);
	  closesocket(inSock);
      return(false);
  }
  delete(buf);
  return(true);
}

void connection::handleIncoming()
{
	char buffer[BUF_SIZE];

	while(!timeToDie)
	{
		int recvCount;
		recvCount=recv(inSock,buffer,BUF_SIZE,0);

		if(recvCount==0)
		{
			timeToDie=true;
			break;
		}

		if(recvCount<0 && timeToDie)
			break;

		if(recvCount<0 && recvCount!=WSAEMSGSIZE)
		{
			TCHAR buffer[1000];
			wsprintf(buffer,_T("In Receive failed with error %d\r\n"),WSAGetLastError());
			SvcDebugOut(buffer);
			timeToDie=true;
			break;
		}

		if(recvCount>0)
		{
			send(outSock,buffer,recvCount,0);
		}
	}
	timeToDie=true;
	closesocket(inSock);
	closesocket(outSock);
}

void connection::handleOutgoing()
{
	char buffer[BUF_SIZE];

	while(!timeToDie)
	{
		int recvCount;
		recvCount=recv(outSock,buffer,BUF_SIZE,0);

		if(recvCount==0)
		{
			timeToDie=true;
			break;
		}

		if(recvCount<0 && timeToDie)
			break;

		if(recvCount<0 && recvCount!=WSAEMSGSIZE)
		{
			TCHAR buffer[1000];
			wsprintf(buffer,_T("Out Receive failed with error %d\r\n"),WSAGetLastError());
			SvcDebugOut(buffer);
			timeToDie=true;
			break;
		}

		if(recvCount>0)
		{
			send(inSock,buffer,recvCount,0);
		}
	}
	timeToDie=true;
	closesocket(outSock);
	closesocket(inSock);
}

DWORD WINAPI InvokeConnectionThread(LPVOID lpParameter)
{
	connection *obj=((connection::invokeParam_t *)lpParameter)->obj;

	if(obj==NULL)
	{
		SvcDebugOut(_T("Obj is NULL\r\n"));
	}

	if(((connection::invokeParam_t *)lpParameter)->method==1)
		obj->handleIncoming();
	else
		obj->handleOutgoing();

	return(0);
}
