#pragma once
#include <vector>

namespace XML
{
	template<typename T>
	void DestoryVector(T *list){
		for (int i = 0; i < (int)list->size(); i++){
			delete list->at(i);
		}
		delete list;
	}

	std::wstring GetWString(const char* buff, int size);
}