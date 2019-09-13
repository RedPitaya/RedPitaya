#pragma once
#include <locale>
#include "XMLString.h"

namespace XML
{
	class XMLAttribute
	{
		XMLString m_name;
		XMLString m_value;
			 char m_quote;
		
		XMLAttribute(const XMLAttribute &v) = delete;
		XMLAttribute& operator = (XMLAttribute &v1) = delete;
		XMLAttribute() = delete;
	public:
		
		// Copy pointer inside and delete in destructor
		XMLAttribute(XMLString &_name, XMLString &_value, char _quote);
		
		~XMLAttribute();

		std::wstring toWString();
		std::wstring Name();
        XMLString&   NameXS();
		std::wstring Value();
        std::string  ValueString();
	};

}
