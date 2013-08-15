/*
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *		Copyright (C) 2006  Tommi Rouvali (tommi@rouvali.com)
 */

#include "stdafx.h"

bool listener::init()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD( 2, 2 );

	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) 
	{
		//log something
		return(false);
	}

	if ( LOBYTE( wsaData.wVersion ) != 2 || HIBYTE( wsaData.wVersion ) != 2 ) 
	{
		// Log something

		WSACleanup( );
		return(false); 
	}
	return(true);
}

void listener::cleanup()
{
	WSACleanup( );
}

void listener::startListener(int xport,rules_t *rules)
{
	listener *obj=new listener();
	obj->port=xport;
	obj->rules=rules;
	obj->Thread=CreateThread(NULL,0,InvokeListenerThread,(LPVOID)obj,0,NULL);
}

void listener::run()
{
	TCHAR buf[1000];
	SOCKET ListenSocket;
	ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ListenSocket == INVALID_SOCKET) 
	{
		wsprintf(buf,_T("Error at socket(): %ld\r\n"), WSAGetLastError());
		SvcDebugOut(buf);
		return;
	}

	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr("0.0.0.0");
	service.sin_port = htons(port);

	if (bind( ListenSocket,(SOCKADDR*) &service, sizeof(service)) == SOCKET_ERROR) 
	{
		SvcDebugOut(_T("bind() failed.\r\n"));
		closesocket(ListenSocket);
		return;
	}

	if (listen( ListenSocket, 1 ) == SOCKET_ERROR)
		SvcDebugOut(_T("Error listening on socket.\r\n"));

	SOCKET AcceptSocket;
	wsprintf(buf,_T("Listening port %d with rules:\r\n"),port);
	SvcDebugOut(buf);

	int pos=0;

	while(rules[pos].type!=RULES_T_EOL)
	{
		if(rules[pos].type==RULES_T_FORWARD)
			wsprintf(buf,_T("Forward host matching %s to %s:%d\r\n"),rules[pos].match,rules[pos].dest,rules[pos].destPort);
		else
			wsprintf(buf,_T("Drop host matching %s\r\n"),rules[pos].match);
		SvcDebugOut(buf);
		pos++;
	}

	while(1) 
	{
		AcceptSocket = SOCKET_ERROR;
		while( AcceptSocket == SOCKET_ERROR ) 
		{
			AcceptSocket = accept( ListenSocket, NULL, NULL );
		}

		TCHAR *dest;
		int destPort;
		findDestination(AcceptSocket,&dest,&destPort);
		if(dest==NULL)
		{
			closesocket(AcceptSocket);
		}
		else
		{
			connection *con=new connection();
			con->inSock=AcceptSocket;
			con->destination=dest;
			con->destPort=destPort;
			con->run();
		}
	}
}

void listener::findDestination(SOCKET socket,TCHAR **destination, int *dport)
{
	TCHAR buf[1000];
	int pos=0;

	sockaddr_in inAddr;
	int addrLen=sizeof(inAddr);
	getpeername(socket,(sockaddr *)&inAddr,&addrLen);

	while(rules[pos].type!=RULES_T_EOL)
	{
		TCHAR TSTRinAddr[100];

		wsprintf(TSTRinAddr,_T("%d.%d.%d.%d"),inAddr.sin_addr.S_un.S_un_b.s_b1,inAddr.sin_addr.S_un.S_un_b.s_b2,inAddr.sin_addr.S_un.S_un_b.s_b3,inAddr.sin_addr.S_un.S_un_b.s_b4);

		char *szInaddr=UnicodeToCChar(TSTRinAddr);
		char *szMatch=UnicodeToCChar(rules[pos].match);
		
		boost::RegEx *r=new boost::RegEx(szMatch);		
		bool match=r->Match(szInaddr);
		delete(r);
		delete(szInaddr);
		delete(szMatch);
		SvcDebugOut(_T("Before match\r\n"));
		if(match)
		{
			SvcDebugOut(_T("match=true\r\n"));
			if(rules[pos].type==RULES_T_FORWARD)
			{	
				SvcDebugOut(_T("Forw\r\n"));
				wsprintf(buf,_T("Client connected from %d.%d.%d.%d to port %d. Forwarding it to %s:%d\r\n")
					,inAddr.sin_addr.S_un.S_un_b.s_b1,inAddr.sin_addr.S_un.S_un_b.s_b2,inAddr.sin_addr.S_un.S_un_b.s_b3,inAddr.sin_addr.S_un.S_un_b.s_b4
					,port,rules[pos].dest,rules[pos].destPort);
				SvcDebugOut(buf);
				*destination=rules[pos].dest;
				*dport=rules[pos].destPort;
				return;
			}

			if(rules[pos].type==RULES_T_DENY)
			{	
				wsprintf(buf,_T("Client connected from %d.%d.%d.%d to port %d. Dropping because deny rule:%s\r\n")
					,inAddr.sin_addr.S_un.S_un_b.s_b1,inAddr.sin_addr.S_un.S_un_b.s_b2,inAddr.sin_addr.S_un.S_un_b.s_b3,inAddr.sin_addr.S_un.S_un_b.s_b4
					,port,rules[pos].match);
				SvcDebugOut(buf);

				*destination=NULL;
				*dport=0;
				return;
			}
		}
		pos++;
		SvcDebugOut(_T("pos++\r\n"));
	}

	wsprintf(buf,_T("Client connected from %d.%d.%d.%d to port %d. Dropping because hitting end of rules\r\n")
					,inAddr.sin_addr.S_un.S_un_b.s_b1,inAddr.sin_addr.S_un.S_un_b.s_b2,inAddr.sin_addr.S_un.S_un_b.s_b3,inAddr.sin_addr.S_un.S_un_b.s_b4);
	SvcDebugOut(buf);
	*destination=NULL;
	*dport=0;
}


DWORD WINAPI InvokeListenerThread(LPVOID lpParameter)
{
	listener *obj=(listener *)lpParameter;

	if(obj==NULL)
	{
		SvcDebugOut(_T("Obj is NULL\r\n"));
	}

	obj->run();		

	return(0);
}
