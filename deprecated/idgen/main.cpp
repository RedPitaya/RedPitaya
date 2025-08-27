#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <fstream>
#include <sstream>
#include "licmisc.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <libjson.h>

#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>

using namespace std;

#include <encoder/encoder.h>

//Function returns MAC address
long GetMACAddressLinux(const char* nic, char *mac)
{
    FILE *fp;
    ssize_t read;
    const size_t len = 17;

    fp = fopen(nic, "r");
    if(fp == NULL) {
        return -1;
    }

    read = fread(mac, len, 1, fp);
    if(read != 1) {
        fclose(fp);
        return -2;
    }
    mac[len] = '\0';

    fclose(fp);

    return 0;
}

std::string GetMACAddress()
{
	char result[18];
	sprintf(result, "00:00:00:00:00:00");
	GetMACAddressLinux("/sys/class/net/eth0/address", result);
	std::string mac_address(result);
	return mac_address;
}

/* FPGA housekeeping structure - Bazaar relevant part */
typedef struct hk_fpga_reg_mem_s {
    /* configuration:
     * bit   [3:0] - hw revision
     * bits [31:4] - reserved
     */
    uint32_t rev;
    /* DNA low word */
    uint32_t dna_lo;
    /* DNA high word */
    uint32_t dna_hi;
} hk_fpga_reg_mem_t;

/*----------------------------------------------------------------------------*/
/**
 * @brief Get Xilinx DNA number.
 *
 * Function reads the Xilinx DNA number of Red Pitaya.
 *
 * @param[out]   dna    64-bit unsigned DNA number.
 *
 * @retval   0 Success
 * @retval < 0 Failure
 */

int get_xilinx_dna(unsigned long long *dna)
{
 	void *page_ptr;
    long page_addr, page_size = sysconf(_SC_PAGESIZE);
    const long c_dna_fpga_base_addr = 0x40000000;
    const long c_dna_fpga_base_size = 0x20;
    int fd = -1;

    fd = open("/dev/mem", O_RDONLY | O_SYNC);
    if(fd < 0) {
       // printf("%s: open(/dev/mem) failed: %s\n",
             //  __FUNCTION__, strerror(errno));
        return -1;
    }

    page_addr = c_dna_fpga_base_addr & (~(page_size-1));

    page_ptr = mmap(NULL, c_dna_fpga_base_size, PROT_READ,
                          MAP_SHARED, fd, page_addr);

    if((void *)page_ptr == MAP_FAILED) {
        //printf("%s: mmap() failed: %s\n",
               //__FUNCTION__, strerror(errno));
        close(fd);
        return -1;
    }

    hk_fpga_reg_mem_t *hk =(hk_fpga_reg_mem_t *)page_ptr;
    *dna = hk->dna_hi & 0x00ffffff;

    *dna = *dna << 32 | hk->dna_lo;

    if(munmap(page_ptr, c_dna_fpga_base_size) < 0) {
        //printf("%s: munmap() failed: %s\n", 
            //   __FUNCTION__, strerror(errno));
        close(fd);
        return -1;
    }
	
    close (fd);

    return 0;
}

std::string GetZynqId()
{
	unsigned long long dna;
	get_xilinx_dna(&dna);

    ostringstream oss;
    oss << dna;
    string res = oss.str();

	return res;
}

//for testing
/*
std::string GetMACAddress()
{
	return "00:0a:95:9d:68:16";
}

std::string GetZynqId()
{
	return "D234048A";
}
*/


//Function returns list of installed apps in JSON-formatted string _info
int GetAppInfo(std::string & _info) {
    
	const std::string path = GetAppsFolder();
	DIR *current = opendir(path.c_str());
    if (current == NULL) {
        return -1;
    }

    struct dirent* entry;
	
	JSONNode apps(JSON_ARRAY);

    while (entry = readdir(current))
	{
        if (entry->d_type == DT_DIR)
		{
			std::string app_path = path + "/" + std::string(entry->d_name);
			DIR *app_dir = opendir(app_path.c_str());
			if (app_dir == NULL) {
				return -1;
			}
			//std::cout<<"app"<<std::endl;
			JSONNode app(JSON_NODE);
			
			struct dirent* app_entry;
			while (app_entry = readdir(app_dir))
			{
				//serch for controllerhf.so
				if ( strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..") && (app_entry->d_type == DT_REG) && !(strcmp(app_entry->d_name, "controllerhf.so")))
				{
					//set application dir name as app_id
					app.push_back(JSONNode("app_id", entry->d_name));
					
					//get name of app from info.json
					std::string info_file_path = app_path + "/info/info.json";

					std::string name_str = GetDataFromFile(info_file_path, "name");
					//std::cout<<name_str<<std::endl;
					
					app.push_back(JSONNode("app_name", name_str));
					
					//get md5 checksum of controllerhf.so
					std::string contr_path = app_path + "/controllerhf.so"; 
					
					std::string md5 = GetMD5(contr_path.c_str());
					
					app.push_back(JSONNode("app_checksum", md5));
					apps.push_back(app);	
				}
			}
			
			closedir(app_dir);
        }
    }
	
	//std::cout<<"apps"<< apps.write_formatted()<< std::endl;
	_info = apps.write_formatted();
    closedir(current);

    return 0;
}

void print_help(void)
{
	printf("Options\n idgen -o <ID file name> -i <ID file name>  -v <Licence file> -a <Application id>\n\
				-o Specify output ID file name.\n\
				-i Specify input ID file name.\n\
				-v Specify input license file to verify.\n\
				-a Specify input application id to verify.\n");
}

bool verify_app_license(const char* app_id, const char* lic_file, const char* idfile_name)
{
	//getting app_key from license file

	JSONNode n(JSON_NODE);
	bool failed = GetJSONObject(lic_file, n);
	if(!failed)
	{

		std::string app_key;		
		try {
			JSONNode apps(JSON_ARRAY);
			apps = n.at("registered_apps").as_array();
			
			int size = apps.size();
			
			for(int i=0; i<size; i++)
			{
				JSONNode app(JSON_NODE);
				app = apps.at(i);
				
				std::string id = app.at("app_id").as_string();
				if(id == app_id)
				{

					app_key = app.at("app_key").as_string();
					break;
				}	
			}
		} catch (std::exception const & e) {
			std::cout<<"License verification is failed. License file is broken!"<<std::endl;
			return 1;
		}	
		//decoding app_key
		if(app_key.empty())
		{
			std::cout<<"Application is not registered!"<<std::endl;
			return 1;
		}
		//std::cout<<"app_key "<<app_key<<std::endl;
		std::string decoded_key;
		try {
		 	decoded_key = Decode(app_key);
		} catch (std::exception const & e) {
			std::cout<<"License verification is failed. Invalid license!"<<std::endl;
			return 1;
		}
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

		std::string orig_dev_id = GetDevID(idfile_name);

		if(orig_dev_id.empty())
		{
			std::cout<<"License verification is failed. The id file is required!"<<std::endl;
			return 1;
		}

		if(orig_dev_id != dev_id)
		{
			std::cout<<"Invalid license!"<<std::endl;
			return 1;
		}
		
		std::string orig_checksum_id = GetAppChecksum(idfile_name, app_id);

		if(orig_checksum_id.empty())
		{
			std::cout<<"License verification is failed. No such application in id file!"<<std::endl;
			return 1;
		}
		
		if(orig_checksum_id != app_checksum)
		{
			std::cout<<"Invalid license!"<<std::endl;
			return 1;
		}
		
		//check if application was not change
		std::string apps_folder = GetAppsFolder();
		std::string contr_path = apps_folder+"/"+app_id+"/"+"controllerhf.so";
		orig_checksum_id = GetMD5(contr_path.c_str());
		
		if(orig_checksum_id.empty())
		{
			std::cout<<"License verification is failed. The application file was not found!"<<std::endl;
			return 1;
		}
		
		if(orig_checksum_id != app_checksum)
		{
			std::cout<<"License verification is failed. The application file was corrupt!"<<std::endl;
			return 1;
		}
	}
	else
	{	
		std::cout<<"License verification is failed. Could not open license file!"<<std::endl;
		return 1;
	}
	std::cout<<"License verification is successful!"<<std::endl;
	return 0;
}

int main (int argc, char *argv[]){
	
	int rez=0;
	bool gen = false;
	bool verify = false;
	std::string idfile_name, app_id, lic_file_name;
	while ( (rez = getopt(argc,argv,"o:v:i:a:h")) != -1){
		switch (rez){
			case 'h':
			{
				printf("This is idgen help.\n");
				print_help();
				break;
			};		
			case 'v': 
			{
				//TODO
				//Decode and check
				verify = true;
				lic_file_name = optarg;
				//printf("found argument \"v = %s\".\n",optarg); 
				break;
			};
			case 'i': 
			{
				//TODO
				//Check ID file
				//verify = true;
				idfile_name = optarg;
				//printf("found argument \"i = %s\".\n",optarg);
				break;
			};
			case 'o': 
			{
				//TODO
				//Create ID file
				gen = true;
				idfile_name = optarg;
				//printf("found argument \"o = %s\".\n",optarg);
				break;
			};
			case 'a': 
			{
				//TODO
				//Create ID file
				//verify = true;
				app_id = optarg;
				//printf("found argument \"a = %s\".\n",optarg);
				break;
			};
        };
	};
	
	//Generate Id file including information of installed apps, MAC address and Zynq ID
	if(gen)
	{

		std::string app_info;
		//get list of installed apps
		GetAppInfo(app_info);
		
		JSONNode apps = libjson::parse(app_info);
		apps.set_name("apps");
		//std::cout<<"app_info \n"<<app_info<<std::endl;
		
		//get MAC
		std::string mac = GetMACAddress();
		//get Zynq
		std::string zynq = GetZynqId();
		
		JSONNode n(JSON_NODE);//root
		n.push_back(JSONNode("version", "2.0"));
		n.push_back(JSONNode("mac_address", mac));
		n.push_back(JSONNode("zynq_id", zynq));
		n.push_back(apps);
		
		std::string text = n.write_formatted();
		
		WriteData(idfile_name, text);
	}
	
	if(verify && !idfile_name.empty())
	{
		bool res = verify_app_license(app_id.c_str(), lic_file_name.c_str(), idfile_name.c_str());
		return res;
	}
	
	return 0;
}
