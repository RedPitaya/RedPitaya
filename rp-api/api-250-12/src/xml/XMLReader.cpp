#include "XMLReader.h"

namespace XML
{
	XMLReader::XMLReader(){}

	XMLReader::~XMLReader(){}

	XMLDocument* XMLReader::XMLReadString(char *buffer, int Size){
		return xmlParser.ParseXML(buffer, Size);		
	}

	int XMLReader::GetError(){
		return xmlParser.GetError();
	}

	const std::vector<std::wstring> XMLReader::GetErrorList(){
		return xmlParser.GetErrorList();
	}
}
