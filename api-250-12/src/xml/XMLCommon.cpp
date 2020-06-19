#include <stdexcept>
#include <locale>
#include <codecvt>

#include "XMLCommon.h"

namespace XML
{
	std::wstring GetWString(const char* buff, int size){
		std::string str(buff, size);
		try
		{
			std::wstring wstr(str.begin(), str.end());
			return wstr;
			//using convert_type = std::codecvt_utf8<wchar_t>;
        	//std::wstring_convert<convert_type, wchar_t> converter;
        	//return converter.to_bytes( str );
			//std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
			//return converter.from_bytes(str);
		}
		catch (...)
		{
			
			size_t length = str.length();
			std::wstring result;
			result.reserve(length);
			for (size_t i = 0; i < length; i++){
				result.push_back(str[i] & 0xFF);
			}
			return result;
		}
	}
}