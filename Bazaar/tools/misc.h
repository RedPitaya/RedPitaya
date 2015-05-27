#include <libjson.h>
#include <fstream>
#include <sstream>

#include "md5.h"
using CryptoPP::MD5;
#include "hex.h"
using CryptoPP::HashFilter;
#include "files.h"
using CryptoPP::FileSource;
#include "filters.h"
using CryptoPP::StringSink;

#ifdef IS_DEV_BRANCH
const char* apps_path="/opt/www/apps";
#else
const char* apps_path="/opt/redpitaya/www/apps";
#endif

//Function gets json object from file
inline int GetJSONObject(const std::string _file_name, JSONNode & _json_node)
{
	std::ifstream file(_file_name.c_str(), std::ifstream::in);
	if (file.is_open()) //if exist
	{
		std::ostringstream tmp;
		tmp<<file.rdbuf();
		std::string s = tmp.str();
		_json_node = libjson::parse(s);
		file.close();
		return 0;
	}
	printf("No such file: %s\n", _file_name.c_str());
	file.close();
	return -1;
		
}

//Function reads specific data '_data_id'  from file
inline std::string GetDataFromFile(const std::string _file_name, const std::string _data_id)
{
	JSONNode n(JSON_NODE);
	bool failed = GetJSONObject(_file_name, n);
	
	if(!failed)
		return n.at(_data_id.c_str()).as_string();
	
	return "";
}

//Function reads Zynq ID from ID file
inline std::string GetDevID(const std::string _file_name)
{
	std::string devid_str = GetDataFromFile(_file_name, "zynq_id");
	return devid_str;
}

inline void WriteData(const std::string _file_name, const std::string _data)
{
	FILE * pFile = fopen(_file_name.c_str(), "w");
	if (pFile!=NULL)
	{	
		fputs(_data.c_str(), pFile);	
		fclose(pFile);
	}
}

inline std::string GetAppChecksum(const std::string _file_name, const std::string _app_id)
{
	JSONNode n(JSON_NODE);
	bool failed = GetJSONObject(_file_name, n);
	if(!failed)
	{
		JSONNode apps(JSON_ARRAY);
		apps = n.at("apps").as_array();
		int size = apps.size();
		for(int i=0; i<size; i++)
		{
			JSONNode app(JSON_NODE);
			app = apps.at(i);
			std::string app_id = app.at("app_id").as_string();
			if(app_id == _app_id)
			{
				std::string app_checksum = app.at("app_checksum").as_string();
				std::string res = app_checksum;
				return res;
			}	
		}
	}
	return "";
}

inline std::string GetAppsFolder()
{
	std::string apps_folder = apps_path;
	//std::string apps_folder = "opt/www/apps"; //for testing
	return 	apps_folder;
}

//Function returns MD5 checksum of file
inline std::string GetMD5(const char * _file)
{
	FILE * pFile = fopen(_file, "r");
	if(pFile == NULL)
	{
		return "";
	}
	fclose(pFile);

    std::string result; 
    CryptoPP::MD5 hash; 
    CryptoPP::FileSource(_file, true, new CryptoPP::HashFilter(hash, new CryptoPP::HexEncoder(new CryptoPP::StringSink(result), false))); 

    //std::cout << result << std::endl; 
    return result; 
}
