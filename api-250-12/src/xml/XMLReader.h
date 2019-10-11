#pragma once
#include <iostream>
#include "XMLParser.h"

namespace XML
{
	class XMLReader
	{
		XMLParser xmlParser;
	public:
		XMLReader();
		~XMLReader();
		XMLDocument* XMLReadString(char *buffer, int Size);
		int GetError();
		const std::vector<std::wstring> GetErrorList();
	};
}

