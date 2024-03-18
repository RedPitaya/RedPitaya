/**
 * $Id: $
 *
 * @brief Red Pitaya library arb api
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <cstring>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <unistd.h>
#include <vector>
#include <map>
#include <cmath>
#include "rp_arb.h"
#include "rp_hw-profiles.h"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define NAME_LEN 20

std::string FILE_PATH = {"/home/redpitaya/arb_files"};

namespace fs = std::filesystem;

struct info_t
{
    std::string name;
    uint32_t color;
};

std::vector<std::pair<std::string,info_t>> g_files;

struct binFile_t
{
    // TODO redo to header
    int32_t version = 1;
	char sigName[NAME_LEN];
    uint32_t color;
 	uint32_t size;
	float values[DAC_BUFFER_SIZE];
};

std::string trim(const std::string & source) {
    std::string s(source);
    s.erase(0,s.find_first_not_of(" \n\r\t"));
    s.erase(s.find_last_not_of(" \n\r\t")+1);
    return s;
}

bool checkFreeName(std::string _name){
	for (const auto & entry : fs::directory_iterator(FILE_PATH)){
		std::ifstream file(entry.path());
		if (file.is_open()){
          	file.seekg (sizeof(int32_t), file.beg);
			char name[NAME_LEN];
			file.read(name,NAME_LEN);
			if (_name == std::string(name)){
				return false;
			}
		}
	}
	return true;
}

auto getDACChannels() -> uint8_t{
    uint8_t c = 0;

    if (rp_HPGetFastDACChannelsCount(&c) != RP_HP_OK){
        ERROR("Can't get fast DAC channels count\n");
    }
    return c;
}

bool getGain(float *f){
	auto count = getDACChannels();
	float z = -1;
	for(int i = 0; i < count ;i++){
		float g;
		if (rp_HPGetFastDACGain(i, &g) != RP_HP_OK) return false;
		if(z == -1){
			z = g;
		}else if (z != g){
			return false;
		}
	}
	*f = z;
	return true;
}

int rp_ARBInit(){
	fs::create_directories(FILE_PATH);
	fs::permissions(FILE_PATH, std::filesystem::perms::owner_all | std::filesystem::perms::group_all, std::filesystem::perm_options::add);
	return rp_ARBLoadFiles();
}

int rp_ARBGenFile(std::string _filename){
	if (_filename == ""){
		return RP_ARB_FILE_ERR;
	}

	binFile_t s;
	memset(s.sigName,0,sizeof(s.sigName));
	memset(s.values,0,sizeof(s.values));
	int size = 0;
    s.version = 1;
    s.color = 0xFFC2BC14;
	auto filePath = trim(FILE_PATH + "/" + _filename);
	std::ifstream file(filePath);
    std::string str;
	if (!file.is_open()){
		return RP_ARB_FILE_ERR;
	}

    while (std::getline(file, str)){
		try {
			if (size >= DAC_BUFFER_SIZE){
				fs::remove(filePath);
				return RP_ARB_FILE_TO_LONG;
			}
			str = trim(str);
			if (str != ""){
				float v = std::stof(str);
				s.values[size] = v;
				size++;
			}
		}
		catch (const std::exception&) {
			fs::remove(filePath);
			return RP_ARB_FILE_PARSE_ERR;
		}
    }
	fs::remove(filePath);
	s.size = size;
	int nameIndex = 1;
	std::string sigName = "ARB";
	while(!checkFreeName(sigName + "_" + std::to_string(nameIndex))){
		nameIndex++;
	}
	std::string fn = sigName + "_" + std::to_string(nameIndex);
	strncpy (s.sigName,fn.c_str(),10);
	std::ofstream fileOut(filePath + ".bin" , std::ios::binary | std::ios::trunc);
	fileOut.write (( char *)&s,sizeof(s));
	fileOut.flush();
	fileOut.close();
	return RP_ARB_FILE_OK;
}

int rp_ARBLoadFiles(){
	g_files.clear();
	for (const auto & entry : fs::directory_iterator(FILE_PATH)){
        // fprintf(stderr,"File %s\n",entry.path().filename().c_str());
        TRACE_SHORT("Load file %s",entry.path().filename().c_str())
		std::ifstream file(entry.path());
		if (file.is_open()){
            file.seekg (sizeof(int32_t), file.beg);
			char name[NAME_LEN];
			file.read(name,NAME_LEN);
            uint32_t color;
            file.read(reinterpret_cast<char*>(&color),sizeof(uint32_t));
			g_files.push_back({entry.path().filename(),{.name = std::string(name),.color = color}});
		}
	}
	return RP_ARB_FILE_OK;
}

int rp_ARBGetCount(uint32_t *_count){
	*_count = g_files.size();
	return RP_ARB_FILE_OK;
}

int rp_ARBGetName(uint32_t _index,std::string *_name){
	*_name = "";
	if (_index >= g_files.size()){
		return RP_ARB_WRONG_INDEX;
	}
	*_name = g_files[_index].second.name;
	return RP_ARB_FILE_OK;
}

int rp_ARBGetFileName(uint32_t _index,std::string *_fileName){
	*_fileName = "";
	if (_index >= g_files.size()){
		return RP_ARB_WRONG_INDEX;
	}
	*_fileName = g_files[_index].first;
	return RP_ARB_FILE_OK;
}

int rp_ARBGetSignal(uint32_t _index, float *_data, uint32_t *_size){
	*_size = 0;
	if (_index >= g_files.size()){
		return RP_ARB_WRONG_INDEX;
	}
	auto filePath = trim(FILE_PATH + "/" + g_files[_index].first);
	std::ifstream file(filePath);
	if (!file.is_open()){
		return RP_ARB_FILE_ERR;
	}
	binFile_t s;
	file.read ((char*)&s,sizeof(s));

	*_size = s.size;
	if (s.size > DAC_BUFFER_SIZE){
		s.size = 0;
		return RP_ARB_FILE_ERR;
	}
    if ((int)s.size > DAC_BUFFER_SIZE){
		s.size = 0;
		return RP_ARB_FILE_ERR;
	}
	for(uint32_t i = 0 ; i < s.size; i++){
		_data[i] = s.values[i];
	}
	return RP_ARB_FILE_OK;
}

int rp_ARBGetSignalNP(uint32_t _index, float *_np_data, int _in_size, uint32_t *_size){
    *_size = _in_size;
    return rp_ARBGetSignal(_index,_np_data,_size);
}


int rp_ARBGetSignalByName(std::string _sigName, float *_data, uint32_t *_size){
	*_size = 0;
	for(uint32_t i = 0; i < g_files.size(); i++){
		if (g_files[i].second.name == _sigName){
			return rp_ARBGetSignal(i,_data,_size);
		}
	}
	return RP_ARB_FILE_ERR;
}

int rp_ARBGetSignalByNameNP(std::string _sigName, float *_np_data, int _in_size, uint32_t *_size){
    *_size = _in_size;
    return rp_ARBGetSignalByName(_sigName,_np_data,_size);
}


int rp_ARBSetColor(uint32_t _index,uint32_t color){
    if (_index >= g_files.size()){
		return RP_ARB_WRONG_INDEX;
	}
	auto fname = g_files[_index].first;
	std::fstream file;
	file.open(FILE_PATH + "/" + fname,std::fstream::in | std::fstream::out);
	if (!file.is_open()){
		return RP_ARB_FILE_ERR;
	}

	file.seekp (NAME_LEN + sizeof(int32_t), file.beg);
	file.write(reinterpret_cast<char*>(&color),sizeof(uint32_t));
	file.close();
	return RP_ARB_FILE_OK;
}


int rp_ARBGetColor(uint32_t _index,uint32_t *color){
    *color = 0xFFC2BC14;
	if (_index >= g_files.size()){
		return RP_ARB_WRONG_INDEX;
	}
	*color = g_files[_index].second.color;
	return RP_ARB_FILE_OK;
}

int rp_ARBRenameFile(uint32_t _index,std::string _new_name){
	if (_index >= g_files.size()){
		return RP_ARB_WRONG_INDEX;
	}
	auto fname = g_files[_index].first;
	char name[NAME_LEN];
	std::fstream file;
	file.open(FILE_PATH + "/" + fname,std::fstream::in | std::fstream::out);
	if (file.is_open()){
		file.seekg (sizeof(int32_t), file.beg);
		file.read(name,NAME_LEN);
	}else{
		return RP_ARB_FILE_ERR;
	}

	if (std::string(name) == _new_name){
		return RP_ARB_FILE_SOME_NAME;
	}

	if (!checkFreeName(_new_name)){
		return RP_ARB_FILE_CANT_RENAME;
	}
	file.seekp (sizeof(int32_t), file.beg);
	file.write(_new_name.c_str(), MIN(_new_name.length(),19));
	file.write("\0", 1);
	file.close();
	return RP_ARB_FILE_OK;
}

int rp_ARBLoadToFPGA(rp_channel_t _channel, std::string _sigName){
	for(uint32_t i = 0; i < g_files.size(); i++){
		if (g_files[i].second.name == _sigName){
			float data[DAC_BUFFER_SIZE];
            uint32_t size;
			if (rp_ARBGetSignal(i,data,&size)){
				return rp_GenArbWaveform((rp_channel_t)_channel,data,size);
			}
		}
	}
	return RP_ARB_ERROR_LOAD;
}

int rp_ARBIsValid(std::string _sigName,bool *_valid){

	float max_gain;
	if (!getGain(&max_gain)) return RP_ARB_FILE_ERR;

	*_valid = false;
	for(uint32_t i = 0; i < g_files.size(); i++){
		if (g_files[i].second.name == _sigName){
			float data[DAC_BUFFER_SIZE];
			uint32_t size;
			if (rp_ARBGetSignal(i,data,&size) == RP_ARB_FILE_OK){
				for (uint32_t z = 0 ; z < size ; z++){
					if (max_gain < fabs(data[z])){
						*_valid = false;
						return RP_ARB_FILE_OK;
					}
				}
				*_valid = true;
				return RP_ARB_FILE_OK;
			}
		}
	}
	return RP_ARB_FILE_ERR;
}


