#include <cstring>
#include <codecvt>
#include <locale>
#include "XMLString.h"
#include "XMLCommon.h"

namespace XML
{
	XMLString::XMLString(){
		text = nullptr;
		lenght = 0;
	}

	XMLString::XMLString(const char * _text, int _lenght){
		text = new char[_lenght];
		strncpy(text,_text,_lenght);
		lenght = _lenght;
	}

    XMLString::XMLString(const char *_text){
        text = new char[strlen(_text)];
        strncpy(text,_text,strlen(_text));
        lenght = strlen(_text);
    }

	XMLString::~XMLString(){
		delete[] text;
		text = nullptr;
	}

	XMLString::XMLString(XMLString && v):text(v.text),lenght(v.lenght){
		delete[] v.text;
		v.lenght = 0;		
	}

	XMLString & XMLString::operator=(XMLString && v){
		delete[] text;
		text = v.text;
		lenght = v.lenght;
		v.text = nullptr;
		v.lenght = 0;
		return *this;
	}

	std::wstring XMLString::toWString(){
		return GetWString(text, lenght);
	}

	const char*  XMLString::getText(){
		return text;
	}

	int         XMLString::Lenght(){
		return lenght;
	}

    std::string XMLString::toString(std::wstring str){
        using convert_type = std::codecvt_utf8<wchar_t>;
        std::wstring_convert<convert_type, wchar_t> converter;
        return converter.to_bytes( str );
	}
}
