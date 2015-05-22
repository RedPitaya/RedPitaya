#include "misc.h"
#include "LicenseVerificator.h"

#include <libjson.h>
#include <encoder.h>
#include <cstdio>
#include "../DataManager.h"

std::string GetLicensePath()
{
	std::string path = "/var/redpitaya/lic.lic";
	return path;
}

std::string GetIDFilePath()
{
	std::string path = "/var/redpitaya/idfile.id";
	return path;
}

int verify_app_license(const char* app_id)
{
	dbg_printf("verify_app_license()\n");

	//getting app_key from license file
	std::cout<<"Liscense verifying... "<<std::endl;
	std::string lic_file = GetLicensePath();
	
	
	JSONNode n(JSON_NODE);
	bool failed = GetJSONObject(lic_file, n);
	if(!failed)
	{
		dbg_printf("1\n");
		JSONNode apps(JSON_ARRAY);
		apps = n.at("registered_apps").as_array();
		
		int size = apps.size();
		
		std::string app_key;
		
		for(int i=0; i<size; i++)
		{
			JSONNode app(JSON_NODE);
			app = apps.at(i);
			
			std::string id = app.at("app_id").as_string();
			if(id == app_id)
			{
				dbg_printf("2\n");
				app_key = app.at("app_key").as_string();
				break;
			}	
		}
		
		//decoding app_key
		if(app_key.empty())
		{
			dbg_printf("3\n");
			std::cout<<"Application is not registered!"<<std::endl;
			return 1;
		}
		//std::cout<<"app_key "<<app_key<<std::endl;
		std::string decoded_key = Decode(app_key);
		
		//getting app_id, devid, checksum
		int term_pos = decoded_key.find(";");
		std::string dev_id = decoded_key.substr(0, term_pos);
		decoded_key = decoded_key.substr(term_pos+1);
		term_pos = decoded_key.find(";");
		
		std::string id = decoded_key.substr(0, term_pos);
		
		std::string app_checksum = decoded_key.substr(term_pos+1);
		term_pos = app_checksum.find(";");
		app_checksum = app_checksum.substr(0, term_pos);
		
		//verifying data with data from id file
		std::string idfile_name = GetIDFilePath();
		std::string orig_dev_id = GetDevID(idfile_name);

		if(orig_dev_id.empty())
		{
			dbg_printf("4\n");
			std::cout<<"License verification is failed. The id file is required!"<<std::endl;
			return 1;
		}

		if(orig_dev_id != dev_id)
		{
			dbg_printf("5\n");
			std::cout<<"Invalid license!"<<std::endl;
			return 1;
		}
		
		std::string orig_checksum_id = GetAppChecksum(idfile_name, app_id);

		if(orig_checksum_id.empty())
		{
			dbg_printf("6\n");
			std::cout<<"License verification is failed. No such application in id file!"<<std::endl;
			return 1;
		}
		
		if(orig_checksum_id != app_checksum)
		{
			dbg_printf("6\n");
			std::cout<<"Invalid license!"<<std::endl;
			return 1;
		}
		
		//check if application was not change
		std::string apps_folder = GetAppsFolder();
		std::string contr_path = apps_folder+"/"+app_id+"/"+"controller.so";
		orig_checksum_id = GetMD5(contr_path.c_str());
		
		if(orig_checksum_id.empty())
		{
			dbg_printf("7\n");
			std::cout<<"License verification is failed. The application file was not found!"<<std::endl;
			return 1;
		}
		
		if(orig_checksum_id != app_checksum)
		{
			dbg_printf("%s %s\n", orig_checksum_id.c_str(), app_checksum.c_str());
			std::cout<<"License verification is failed. The application file was corrupt!"<<std::endl;
			return 1;
		}
	}
	else
	{	
		dbg_printf("9\n");
		std::cout<<"License verification is failed. Could not open license file!"<<std::endl;
		return 1;
	}
	dbg_printf("10\n");
	std::cout<<"License verification is successful!"<<std::endl;
	return 0;
}
