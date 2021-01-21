#include <XMLDocument.h>
#include <XMLReader.h>
#include <XMLNode.h>
#include <iostream>
#include <fstream>
#include <clocale>
#include <stdarg.h>
#include "rp-spi.h"
#include "spi/spi.h"
#include <unistd.h>

#include <linux/i2c-dev.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>

using namespace std;
using namespace XML;

namespace rp_spi_fpga{

static XML::XMLString bus_node_name("bus_name");
static XML::XMLString bus_node_fpga_base_name("fpga_base");
static XML::XMLString bus_node_dev_addr_name("device_on_bus");
static XML::XMLString bus_node_header_name("header");
static XML::XMLString bus_node_reg_set_name("reg_set");
static XML::XMLString attr_address_string("address");
static XML::XMLString attr_value_string("value");
static XML::XMLString attr_write_string("write");
static XML::XMLString attr_default_string("default");
static XML::XMLString attr_decription_string("decription");
static XML::XMLString attr_length_string("length");

bool g_enable_verbous = false;
#define MSG(args...) if (g_enable_verbous) fprintf(stdout,args);
#define MSG_A(args...) fprintf(stdout,args);

void rp_spi_enable_verbous(){
    g_enable_verbous = true;
}

void rp_spi_disable_verbous(){
    g_enable_verbous = false;
}

XMLDocument* readFile(const char *configuration_file){
    std::setlocale(LC_ALL, "en_US.UTF-8");
    int length;
	char * buffer;
	ifstream is;
	is.open(configuration_file, ios::binary);
	is.seekg(0, ios::end);
	length = is.tellg();
	is.seekg(0, ios::beg);
	buffer = new char[length];
	is.read(buffer, length);
	is.close();

	XMLReader *reader = new XMLReader();
	XMLDocument *doc = reader->XMLReadString(buffer, length);

	for (auto str : reader->GetErrorList()){
        if (g_enable_verbous) wcout << L"Error string = " << str << std::endl;
        delete doc;
        doc = nullptr;
    }

	delete reader;
	delete[] buffer;
	return doc;
}

int readAttributeValue(XML::XMLNode *node,XML::XMLString &name,int &value){
    XMLAttribute *attr = nullptr;
    attr = node->GetAttributesByName(name);
    if (attr == nullptr) {
        MSG_A("[rp_spi] Missing attribute %s in register\n",name.getText());
        return  -1;
    }
    sscanf(attr->ValueString().c_str(), "%x", &value);
    return 0;
}

int prepareHeader(bool read_flag,char data_size,unsigned short address,char *buffer, char header_length){
    if (header_length == 1){
        if (address > 0x1F) return -1;
        buffer[0] = (char)address;
        if (read_flag) buffer[0] = 0x80 | buffer[0];
        if (data_size == 2) buffer[0] = 0x20 | buffer[0];
        if (data_size == 3) buffer[0] = 0x40 | buffer[0];
        if (data_size >= 4) buffer[0] = 0x60 | buffer[0];
        return 0;
    }

    if (header_length == 2){
        if (address > 0x1FFF) return -1;
        ((short*)buffer)[0] = address;
        if (read_flag) buffer[0] = 0x80 | buffer[0];
        if (data_size == 2) buffer[0] = 0x20 | buffer[0];
        if (data_size == 3) buffer[0] = 0x40 | buffer[0];
        if (data_size >= 4) buffer[0] = 0x60 | buffer[0];
        return 0;
    }

    return -1;
}

int rp_write_to_spi(const char* spi_dev_path,char *buffer_header,int header_length, unsigned char spi_val_to_write){
    return write_to_spi(spi_dev_path , buffer_header, header_length, spi_val_to_write);
}

int rp_read_from_spi(const char* spi_dev_path,char *buffer_header,int header_length, char &value){
    return read_from_spi(spi_dev_path,buffer_header,header_length,value);
}

 int rp_write_to_spi_fpga(const char* spi_dev_path,unsigned int fpga_address,unsigned short dev_address,int reg_addr, unsigned char spi_val_to_write){
    return write_to_fpga_spi(spi_dev_path,fpga_address,dev_address, reg_addr , spi_val_to_write);
 }

int rp_read_from_spi_fpga(const char* spi_dev_path,unsigned int fpga_address,unsigned short dev_address,int reg_addr, char &value){
    return read_from_fpga_spi(spi_dev_path,fpga_address,dev_address, reg_addr , value);
}


int read_header_length(XML::XMLDocument *doc,int &header_length, string &bus_name){
    XMLNode *bus_node = doc->FindFirstNodeByName(bus_node_name);
    if (bus_node == nullptr){
        MSG_A("[rp_spi] Missing node bus_name in configuration file\n");
        return  -1;
    }

    XMLNode *bus_node_header = doc->FindFirstNodeByName(bus_node_header_name);
    if (bus_node_header == nullptr){
        MSG_A("[rp_spi] Missing node header in configuration file\n");
        return  -1;
    }else{
        XMLAttribute *attr = bus_node_header->GetAttributesByName(attr_length_string);
        if (attr == nullptr) {
            MSG_A("[rp_spi] Missing attribute length in header\n");
            return  -1;
        }
        sscanf(attr->ValueString().c_str(), "%x", &header_length);
    }

    bus_name =  XMLString::toString(bus_node->GetInnerText());

    return 0;
}

int rp_spi_load(const char *configuration_file){
    int header_length = 0;
    string bus_name = "";

    XMLDocument *doc = readFile(configuration_file);
    if (doc == nullptr) return -1;

    if (read_header_length(doc,header_length,bus_name)!= 0) return  -1;   

    XMLNode *bus_reg_set = doc->FindFirstNodeByName(bus_node_reg_set_name);
    if (bus_reg_set == nullptr){
        MSG_A("[rp_spi] Missing node reg_set in configuration file\n");
        return  -1;
    }

   

    XMLAttribute *attr = nullptr;
    auto reg_set_nodes = bus_reg_set->GetChildNodes();
    for (auto reg: *reg_set_nodes){
        int  reg_addr = 0 ;
        int  reg_value = 0;
        std::string reg_write_mode;
        int reg_default = 0;
        std::string reg_description ;

        if (readAttributeValue(reg,attr_address_string,reg_addr)!=0) return -1;
    
        if (readAttributeValue(reg,attr_value_string,reg_value)!=0) return -1;

        if (readAttributeValue(reg,attr_default_string,reg_default)!=0) return -1;   

        attr = reg->GetAttributesByName(attr_write_string);
        if (attr == nullptr) {
            MSG_A("[rp_spi] Missing attribute write in register\n");
            return  -1;
        }
        reg_write_mode = attr->ValueString();

        attr = reg->GetAttributesByName(attr_decription_string);
        if (attr == nullptr) {
            MSG_A("[rp_spi] Missing attribute write in register\n");
            return  -1;
        }
        reg_description = attr->ValueString();


        if ((reg_write_mode == "value") || (reg_write_mode == "default")) {
            /* Write data to spi */
            char data = 0;
            char header[2] = {0, 0};
            if (reg_write_mode == "value") {
                data = (char)reg_value;
            }

            if (reg_write_mode == "default") {
                data = (char)reg_default;
            }

            if (prepareHeader(false,1 ,reg_addr, header, header_length) != 0) {
                MSG_A("[rp_spi] ERROR prepare header for %s\n",reg_description.c_str());
            }

            if (rp_write_to_spi(bus_name.c_str(),header, header_length , data) != 0) {
                /* Error process */
                MSG_A("[rp_spi] ERROR write value of %s to spi\n",reg_description.c_str());
            }
            MSG("[rp_spi] Success write %s value 0x%.2X by address 0x%.2X \n",reg_description.c_str(),data,reg_addr);
        }else{
            MSG("[rp_spi] Skip write %s to i2c\n",reg_description.c_str());
        }
    }

    delete doc;
    return 0;
}

int rp_spi_print(const char *configuration_file){
    int header_length = 0;
    string bus_name = "";

    XMLDocument *doc = readFile(configuration_file);
    if (doc == nullptr) return -1;

    if (read_header_length(doc,header_length,bus_name)!= 0) return  -1; 

    XMLNode *bus_reg_set = doc->FindFirstNodeByName(bus_node_reg_set_name);
    if (bus_reg_set == nullptr){
        MSG_A("[rp_spi] Missing node reg_set in configuration file\n");
        return  -1;
    }

    XMLAttribute *attr = nullptr;
    auto reg_set_nodes = bus_reg_set->GetChildNodes();
    for (auto reg: *reg_set_nodes){
        int  reg_addr = 0 ;
        int  reg_value = 0;
        std::string reg_write_mode;
        int reg_default = 0;
        std::string reg_description ;
        char data;
        char header[2] = {0, 0};

        if (readAttributeValue(reg,attr_address_string,reg_addr)!=0) return -1;
    
        if (readAttributeValue(reg,attr_value_string,reg_value)!=0) return -1;

        if (readAttributeValue(reg,attr_default_string,reg_default)!=0) return -1;  

        attr = reg->GetAttributesByName(attr_write_string);
        if (attr == nullptr) {
            MSG_A("[rp_spi] Missing attribute write in register\n");
            return  -1;
        }
        reg_write_mode = attr->ValueString();

        attr = reg->GetAttributesByName(attr_decription_string);
        if (attr == nullptr) {
            MSG_A("[rp_spi] Missing attribute write in register\n");
            return  -1;
        }
        reg_description = attr->ValueString();
        
        if (prepareHeader(false,1 ,reg_addr, header, header_length) != 0) {
                MSG_A("[rp_spi] ERROR prepare header for %s\n",reg_description.c_str());
        }

        if (rp_read_from_spi(bus_name.c_str(),header, header_length, data) != 0) {
            /* Error process */
            MSG_A("[rp_spi] ERROR read value of %s to spi\n",reg_description.c_str());
        }
        MSG_A("[rp_spi] Addr: 0x%.2X\tval: 0x%.2X\t%s\n",reg_addr,data,reg_description.c_str());
    }

    delete doc;
    return 0;
}

int rp_spi_compare(const char *configuration_file){
   int header_length = 0;
    string bus_name = "";

    XMLDocument *doc = readFile(configuration_file);
    if (doc == nullptr) return -1;

    if (read_header_length(doc,header_length,bus_name)!= 0) return  -1; 

    XMLNode *bus_reg_set = doc->FindFirstNodeByName(bus_node_reg_set_name);
    if (bus_reg_set == nullptr){
        MSG_A("[rp_spi] Missing node reg_set in configuration file\n");
        return  -1;
    }

    bool equal_values = true;
    XMLAttribute *attr = nullptr;
    auto reg_set_nodes = bus_reg_set->GetChildNodes();
    for (auto reg: *reg_set_nodes){
        int  reg_addr = 0 ;
        int  reg_value = 0;
        std::string reg_write_mode;
        int reg_default = 0;
        std::string reg_description ;
        char data;
        char header[2] = {0, 0};

        if (readAttributeValue(reg,attr_address_string,reg_addr)!=0) return -1;
    
        if (readAttributeValue(reg,attr_value_string,reg_value)!=0) return -1;

        if (readAttributeValue(reg,attr_default_string,reg_default)!=0) return -1;  

        attr = reg->GetAttributesByName(attr_write_string);
        if (attr == nullptr) {
            MSG_A("[rp_spi] Missing attribute write in register\n");
            return  -1;
        }
        reg_write_mode = attr->ValueString();

        attr = reg->GetAttributesByName(attr_decription_string);
        if (attr == nullptr) {
            MSG_A("[rp_spi] Missing attribute write in register\n");
            return  -1;
        }

        if ((reg_write_mode == "value") || (reg_write_mode == "default")) {
            /* Write data to i2c */
            char data_in_file = 0;

            if (reg_write_mode == "value") {
                data_in_file = (char)reg_value;
            }

            if (reg_write_mode == "default") {
                data_in_file = (char)reg_default;
            }

            reg_description = attr->ValueString();
        
            if (prepareHeader(false,1 ,reg_addr, header, header_length) != 0) {
                MSG_A("[rp_spi] ERROR prepare header for %s\n",reg_description.c_str());
            }

            if (rp_read_from_spi(bus_name.c_str(),header, header_length, data) != 0) {
                /* Error process */
                MSG_A("[rp_spi] ERROR read value of %s to spi\n",reg_description.c_str());
            }
            if (data_in_file != data) equal_values = false;
            MSG("[rp_spi] Addr: 0x%.2X\tvalue in i2c: 0x%.2X\tvalue in xml: 0x%.2X\t%s\n",reg_addr,data,data_in_file,reg_description.c_str());
        }
    }

    delete doc;
    
    if (equal_values)
    return 0;
        else 
    return 1;
}


int rp_spi_load_via_fpga(const char *configuration_file){
    string bus_name = "";
    unsigned int   fpga_device_addr = 0;
    unsigned short device_addr = 0;
    
    XMLDocument *doc = readFile(configuration_file);
    if (doc == nullptr) return -1;

    XMLNode *bus_node = doc->FindFirstNodeByName(bus_node_name);
    if (bus_node == nullptr){
        MSG_A("[rp_spi] Missing node bus_name in configuration file\n");
        return  -1;
    }
    bus_name =  XMLString::toString(bus_node->GetInnerText());

    XMLNode *bus_node_dev_addr = doc->FindFirstNodeByName(bus_node_fpga_base_name);
    if (bus_node_dev_addr == nullptr){
        MSG_A("[rp_spi] Missing node device_on_bus in configuration file\n");
        return  -1;
    }else{
        XMLAttribute *attr = bus_node_dev_addr->GetAttributesByName(attr_address_string);
        if (attr == nullptr) {
            MSG_A("[rp_spi] Missing attribute address in device_on_bus\n");
            return  -1;
        }
        sscanf(attr->ValueString().c_str(), "%x", &fpga_device_addr);
    }

    bus_node_dev_addr = doc->FindFirstNodeByName(bus_node_dev_addr_name);
    if (bus_node_dev_addr == nullptr){
        MSG_A("[rp_spi] Missing node device_on_bus in configuration file\n");
        return  -1;
    }else{
        XMLAttribute *attr = bus_node_dev_addr->GetAttributesByName(attr_address_string);
        if (attr == nullptr) {
            MSG_A("[rp_spi] Missing attribute address in device_on_bus\n");
            return  -1;
        }
        sscanf(attr->ValueString().c_str(), "%x", &device_addr);
    }

    XMLNode *bus_reg_set = doc->FindFirstNodeByName(bus_node_reg_set_name);
    if (bus_reg_set == nullptr){
        MSG_A("[rp_spi] Missing node reg_set in configuration file\n");
        return  -1;
    }

    XMLAttribute *attr = nullptr;
    auto reg_set_nodes = bus_reg_set->GetChildNodes();
    for (auto reg: *reg_set_nodes){
        int  reg_addr = 0 ;
        int  reg_value = 0;
        std::string reg_write_mode;
        int reg_default = 0;
        std::string reg_description ;

        if (readAttributeValue(reg,attr_address_string,reg_addr)!=0) return -1;
    
        if (readAttributeValue(reg,attr_value_string,reg_value)!=0) return -1;

        if (readAttributeValue(reg,attr_default_string,reg_default)!=0) return -1;   

        attr = reg->GetAttributesByName(attr_write_string);
        if (attr == nullptr) {
            MSG_A("[rp_spi] Missing attribute write in register\n");
            return  -1;
        }
        reg_write_mode = attr->ValueString();

        attr = reg->GetAttributesByName(attr_decription_string);
        if (attr == nullptr) {
            MSG_A("[rp_spi] Missing attribute write in register\n");
            return  -1;
        }
        reg_description = attr->ValueString();


        if ((reg_write_mode == "value") || (reg_write_mode == "default")) {
            /* Write data to spi */
            char data = 0;
            if (reg_write_mode == "value") {
                data = (char)reg_value;
            }

            if (reg_write_mode == "default") {
                data = (char)reg_default;
            }

            if (rp_spi_fpga::rp_write_to_spi_fpga(bus_name.c_str(),fpga_device_addr ,device_addr , reg_addr , data) != 0) {
                /* Error process */
                MSG_A("[rp_spi] ERROR write value of %s to spi\n",reg_description.c_str());
            }
            MSG("[rp_spi] Success write %s value 0x%.2X by address 0x%.2X \n",reg_description.c_str(),data,reg_addr);
        }else{
            MSG("[rp_spi] Skip write %s to i2c\n",reg_description.c_str());
        }
    }

    delete doc;
    return 0;
}

}