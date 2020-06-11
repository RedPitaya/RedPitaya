#pragma once
#include <vector>
#include "XMLUTF8Buffer.h"
#include "XMLNode.h"
#include "XMLNameSpace.h"
#include "XMLDocument.h"
#include "XMLAttribute.h"
#include "XMLNode.h"
#include "XMLString.h"

namespace XML
{
	class XMLParser
	{
		NodeVector *nodeoflist;
		std::vector<std::wstring> nodes_stek;
		NameSpaceVector *namespacevector;

		Buffer *buffer;
		XMLProlog*   ParseProlog();
		AttibuteVector* ParseAttribute(char *_buffer, int BufferLenght);
		XMLString* ParseAttributName(Buffer &buffer);
		XMLString* ParseAttributValue(Buffer &buffer, char &Quote);
		char* Trim(char *Buffer, int BufferLenght, int &NewBufferLenght);

		int Error;
		std::vector<std::wstring> ErrorList;
		void CheckProlog(XMLProlog *prolog);
		XMLNode*     ParseXMLNode(XMLNode* _parentNode);
		XMLNode*     ParseXMLNodeName(XMLNode* _parentNode);
		XMLNode*     ParseXMLRootNode();
		std::wstring ParseXMLEndNode();
		bool         ParseXMLComment();
		std::wstring ParseXMLReadCDATA();
		std::wstring ParseXMLInnerText();
		void		 AddException(std::exception &_ex, int _errorCode);
		bool		 CheckPrefixForXML(XMLString &_text);
		bool         CheckPrefixForXMLNS(XMLString &_text);
		bool		 CheckFormatWithXMLNS(XMLString &_text);
	public:
		XMLParser();
		~XMLParser();
		XML::XMLDocument* ParseXML(char *_buffer, int BufferLenght);
		int GetError();
		std::vector<std::wstring>& GetErrorList();
	};
}

