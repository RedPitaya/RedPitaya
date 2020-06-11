#pragma once
#include <vector>
#include "XMLNode.h"
#include "XMLString.h"

namespace XML
{
	class XMLNameSpace
	{
		char      m_quote;
		XMLString m_name;
		XMLString m_path;
	public:
	
		XMLNode     *nodeOwner;
			
		XMLNameSpace(XMLString &_name, XMLString &_path, char _quote);
		~XMLNameSpace();

		std::wstring toWString();

		XMLString* getFullName();
		XMLString* getPath();
		std::wstring getName();
	};
}

