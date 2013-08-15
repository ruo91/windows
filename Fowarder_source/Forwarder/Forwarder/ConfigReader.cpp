/*
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *		Copyright (C) 2006  Tommi Rouvali (tommi@rouvali.com)
 */

#include "stdafx.h"
#import "msxml4.dll"
#include <tchar.h>
#include <msxml2.h>
#include <exception>
#include "shlwapi.h"
#include "boost/regex.h"

using namespace MSXML2;

inline void EVAL_HR( HRESULT _hr ) 
   { if FAILED(_hr) throw(_hr); }


rules_t *readRules(MSXML2::IXMLDOMNode *node)
{
	rules_t rules[100];

	int i=0,pos=0;
	VARIANT_BOOL hasChild;
	hasChild=node->hasChildNodes();

	if(hasChild)
	{
		MSXML2::IXMLDOMNodeListPtr nodes=node->GetchildNodes();

		for(int i=0;i<nodes->length;i++)
		{
			MSXML2::IXMLDOMNodePtr n=nodes->Getitem(i);
			if(n->nodeType==MSXML2::NODE_ELEMENT)
			{
				if(strcmp(n->nodeName,"forward")==0)
				{
					MSXML2::IXMLDOMNamedNodeMap *attributeMap;
					if(S_OK==n->get_attributes(&attributeMap))
					{
						BOOL matchOk=FALSE;
						BOOL destinationOk=FALSE;
						BOOL destinationPortOk=FALSE;

						for(int i=0;i<attributeMap->length;i++)
						{
							MSXML2::IXMLDOMNodePtr attr=attributeMap->Getitem(i);
							BSTR text;
							attr->get_text(&text);

							if(StrCmp(attr->nodeName,_T("match"))==0)
							{
								StrCpy(rules[pos].match,text);
								matchOk=TRUE;
							}

							if(StrCmp(attr->nodeName,_T("destination"))==0)
							{
								StrCpy(rules[pos].dest,text);
								destinationOk=TRUE;
							}

							if(StrCmp(attr->nodeName,_T("destinationPort"))==0)
							{
								rules[pos].destPort=StrToInt(text);
								destinationPortOk=TRUE;
							}
						}
						if(matchOk && destinationOk && destinationPortOk)
						{
							rules[pos].type=RULES_T_FORWARD;
							pos++;
						}
					}
				}

				if(strcmp(n->nodeName,"deny")==0)
				{
					MSXML2::IXMLDOMNamedNodeMap *attributeMap;
					if(S_OK==n->get_attributes(&attributeMap))
					{
						BOOL matchOk=FALSE;

						for(int i=0;i<attributeMap->length;i++)
						{
							MSXML2::IXMLDOMNodePtr attr=attributeMap->Getitem(i);
							BSTR text;
							attr->get_text(&text);

							if(StrCmp(attr->nodeName,_T("match"))==0)
							{
								StrCpy(rules[pos].match,text);
								matchOk=TRUE;
							}
						}
						if(matchOk)
						{
							rules[pos].type=RULES_T_DENY;
							pos++;
						}
					}
				}
			}
		}
	}
	if(pos==0)
		return(NULL);

	rules[pos].type=RULES_T_EOL;
	pos++;
	rules_t *retval=new rules_t[pos];
	memcpy(retval,rules,sizeof(rules_t)*pos);
	return(retval);
}


void ReadConfig()
{
	LPTSTR path=ReadRegStr(_T("ConfigFile"));
	try
	{
		EVAL_HR(CoInitialize(NULL));
		IXMLDOMDocument2Ptr pXMLDoc = NULL;	
		EVAL_HR(pXMLDoc.CreateInstance("Msxml2.DOMDocument.4.0"));

		HRESULT hr=pXMLDoc->load(path);
		if(hr==0)
		{
			SvcDebugOut(_T("Error opening config file"));
			return;
		}
		delete(path);

		MSXML2::IXMLDOMElementPtr root=pXMLDoc->GetdocumentElement();
		MSXML2::IXMLDOMNodeListPtr nodes=root->GetchildNodes();

		for(int i=0;i<nodes->length;i++)
		{
			MSXML2::IXMLDOMNodePtr n=nodes->Getitem(i);
			if(n->nodeType==MSXML2::NODE_ELEMENT)
			{
				if(strcmp(n->nodeName,"listen")==0)
				{
					MSXML2::IXMLDOMNamedNodeMap *attributeMap;
					if(S_OK==n->get_attributes(&attributeMap))
					{
						int port=0;
						BOOL portOk=FALSE;

						for(int i=0;i<attributeMap->length;i++)
						{
							MSXML2::IXMLDOMNodePtr attr=attributeMap->Getitem(i);
							BSTR text;
							attr->get_text(&text);

							if(StrCmp(attr->nodeName,_T("port"))==0)
							{
								port=StrToInt(text);
								portOk=TRUE;
							}
						}
						rules_t *rules=readRules(n);
						if(portOk && rules!=NULL)
						{							
							listener::startListener(port,rules);
						}
						else
						{
							TCHAR buffer[1000];
							wsprintf(buffer,_T("Not listening port %d. Something wrong with config (missing rules?)\r\n"),port);
							SvcDebugOut(buffer);
						}
					}
				}
			}
		}
	
	}
	catch (...)
	{
		SvcDebugOut(_T("Something bad hapend while loading config."));
	}

//	CoUninitialize();


}

