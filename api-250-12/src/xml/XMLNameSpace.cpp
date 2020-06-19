#include "XMLNameSpace.h"
#include "XMLCommon.h"

namespace XML
{
	XMLNameSpace::XMLNameSpace(XMLString &_name, XMLString &_path, char _quote){
		m_name = std::move(_name);
		m_path = std::move(_path);
		m_quote = _quote;
		nodeOwner = NULL;
	}

	XMLNameSpace::~XMLNameSpace(){}

	std::wstring XMLNameSpace::toWString(){
		wchar_t wch(m_quote);
		return m_name.toWString() + L" = " + wch + m_path.toWString() + wch;
	}

	XMLString* XMLNameSpace::getFullName(){
		return &m_name;
	}

	XMLString* XMLNameSpace::getPath(){
		return &m_path;
	}

	std::wstring XMLNameSpace::getName(){
		int i = 0;
		int len = m_name.Lenght();
		for (; i < len; i++){
			if (m_name.getText()[i] == ':'){
				i++;
				break;
			}
		}
		if (i == len){
			return L"";
		}
		else{
			return GetWString(&(m_name.getText()[i]), len - i);
		}
	}
}
