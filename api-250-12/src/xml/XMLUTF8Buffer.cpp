#include "XMLUTF8Buffer.h"
#include <stdexcept>
#include <locale>
#include <codecvt>

namespace XML
{

	UTF8Buffer::UTF8Buffer(char *buffer, int BufSize) :Buffer(buffer,BufSize){
		bytePosition = 0;
	}

	UTF8Buffer::~UTF8Buffer(){}

	bool UTF8Buffer::ReadBOM(){
		if (bytePosition == 0){
			if ((((unsigned char)buffer[bytePosition] == 0xEF) && ((unsigned char)buffer[bytePosition + 1] == 0xBB) && ((unsigned char)buffer[bytePosition + 2] == 0xBF))){
				bytePosition += 3;
				return true;
			}
		}
		return false;
	}

	// UTF8Char UTF8Buffer::NextRead(){
	// 	UTF8Char charu8;
	// 	while (!IsEnd()){
	// 		if ((buffer[bytePosition] & 0xF0) == 0xF0){
	// 			charu8.B1 = buffer[bytePosition++];
	// 			charu8.B2 = buffer[bytePosition++];
	// 			charu8.B3 = buffer[bytePosition++];
	// 			charu8.B4 = buffer[bytePosition++];
	// 			charu8.Lenght = 4;
	// 			return charu8;
	// 		}

	// 		if ((buffer[bytePosition] & 0xE0) == 0xE0){
	// 			charu8.B1 = buffer[bytePosition++];
	// 			charu8.B2 = buffer[bytePosition++];
	// 			charu8.B3 = buffer[bytePosition++];
	// 			charu8.Lenght = 3;
	// 			return charu8;
	// 		}

	// 		if ((buffer[bytePosition] & 0xC0) == 0xC0){
	// 			charu8.B1 = buffer[bytePosition++];
	// 			charu8.B2 = buffer[bytePosition++];
	// 			charu8.Lenght = 2;
	// 			return charu8;
	// 		}

	// 		if ((buffer[bytePosition] & 0x80) == false){
	// 			charu8.B1 = buffer[bytePosition++];
	// 			charu8.Lenght = 1;
	// 			return charu8;
	// 		}
	// 		bytePosition++;
	// 	}
	// 	return charu8;
	// }

	char UTF8Buffer::ReadANSI(){
		if (!IsEnd()){
			if ((buffer[bytePosition] & 0x80) == false){
				return buffer[bytePosition];
			}
		}
		return 0;
	}

	char UTF8Buffer::ReadNextANSI(){
		if (!IsEnd()){
			if ((buffer[bytePosition] & 0x80) == false){
				return buffer[bytePosition++];
			}
		}
		return 0;
	}

	char*  UTF8Buffer::ReadANSI(int Count){
		char *buf = new char[Count];
		memset(buf, 0, Count);
		for (int i = 0; i < Count; i++){
			if ((bytePosition + i) < lenght){
				if ((buffer[bytePosition + i] & 0x80) == false){
					buf[i] = buffer[bytePosition + i];
				}
				else
					break;
			}
			else
				break;
		}
		return buf;
	}

	char* UTF8Buffer::ReadNextANSI(int Count){
		char *buf = new char[Count];
		memset(buf, 0, Count);
		for (int i = 0; i < Count; i++){
			if (bytePosition < lenght){
				if ((buffer[bytePosition] & 0x80) == false){
					buf[i] = buffer[bytePosition];
				}
				else
					break;
			}
			else
				break;
			bytePosition++;
		}
		return buf;
	}

	bool UTF8Buffer::CheckBuffer(){
		if (lenght < 3) throw new std::runtime_error("UTF8Buffer: CheckUTF8Buffer out of range");
		return (((unsigned char)buffer[0] == 0xEF) && ((unsigned char)buffer[1] == 0xBB) && ((unsigned char)buffer[2] == 0xBF));
	}	
}
