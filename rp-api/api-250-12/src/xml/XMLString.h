#pragma once
#include <iostream>
#include <cstring>
#include <fstream>

namespace XML
{
	class XMLString
	{
		char *text;
		int   lenght;
		XMLString(const XMLString &v) = delete;
		XMLString& operator = (XMLString &v) = delete;
	public:
		XMLString();
		XMLString(const char *_text, int _lenght);
        XMLString(const char *_text);
		~XMLString();
        XMLString(XMLString &&v);
		XMLString& operator=(XMLString&& v);
        bool operator==(const XMLString& lhs){ return strncmp(lhs.text,this->text,this->lenght) == 0; }

		const char* getText();
		int         Lenght();
		std::wstring toWString();
		static std::string toString(std::wstring str);
	};
}

