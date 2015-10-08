#include "misc.h"
#include "LicenseVerificator.h"

#include <libjson.h>
#include <encoder.h>
#include <cstdio>

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <fstream>
#include <sstream>
#include "../misc.h"

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

#include "../DataManager.h"

using namespace std;

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

std::string GetLicensePath()
{
	std::string path = "/opt/redpitaya/www/apps/lic.lic";
	return path;
}

std::string GetIDFilePath()
{
	std::string path = "/opt/redpitaya/www/apps/idfile.id";
	return path;
}

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

int verify_app_license_impl(const char* app_id)
{
	//getting app_key from license file
	dbg_printf("Liscense verifying... \n");
	std::string lic_file = GetLicensePath();

	JSONNode n(JSON_NODE);
	bool failed = GetJSONObject(lic_file, n);
	if(!failed)
	{
		JSONNode apps(JSON_ARRAY);
		std::string app_key;
		try {
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
			dbg_printf("License verification is failed. File is broken!\n");
			return 1;
		}
		//decoding app_key
		if(app_key.empty())
		{
			dbg_printf("Application is not registered!\n");
			return 1;
		}
		dbg_printf("app_key %s\n", app_key.c_str());
		std::string decoded_key;
		try {
		 	decoded_key = Decode(app_key);
		} catch (std::exception const & e) {
			dbg_printf("License verification is failed. Invalid license!\n");
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
		std::string idfile_name = GetIDFilePath();
		std::string orig_dev_id = GetDevID(idfile_name);

		if(orig_dev_id.empty())
		{
			dbg_printf("License verification is failed. The id file is required!\n");
			return 1;
		}

		if(orig_dev_id != dev_id)
		{
			dbg_printf("Invalid license!\n");
			return 1;
		}

		std::string orig_checksum_id = GetAppChecksum(idfile_name, app_id);

		if(orig_checksum_id.empty())
		{
			dbg_printf("License verification is failed. No such application in id file!\n");
			return 1;
		}

		if(orig_checksum_id != app_checksum)
		{
			dbg_printf("Invalid license!\n");
			return 1;
		}

		//check if application was not change
		std::string apps_folder = GetAppsFolder();
		std::string contr_path = apps_folder+"/"+app_id+"/"+"controllerhf.so";
		orig_checksum_id = GetMD5(contr_path.c_str());

		if(orig_checksum_id.empty())
		{
			dbg_printf("License verification is failed. The application file was not found!\n");
			return 1;
		}

		if(orig_checksum_id != app_checksum)
		{
			dbg_printf("%s %s\n", orig_checksum_id.c_str(), app_checksum.c_str());
			dbg_printf("License verification is failed. The application file was corrupt!\n");
			return 1;
		}
	}
	else
	{
		dbg_printf("License verification is failed. Could not open license file!\n");
		return 1;
	}
	dbg_printf("License verification is successful!\n");
	return 0;
}
