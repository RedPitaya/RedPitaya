
#include <XMLDocument.h>
#include <XMLReader.h>
#include <XMLNode.h>
#include <iostream>
#include <fstream>
#include <clocale>
#include <stdarg.h>
#include "rp-i2c.h"
#include "i2c/i2c.h"
#include <unistd.h>

#include <linux/i2c-dev.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>

using namespace std;
using namespace XML;

namespace rp_i2c {

static XML::XMLString bus_node_name("bus_name");
static XML::XMLString bus_node_dev_addr_name("device_on_bus");
static XML::XMLString bus_node_reg_set_name("reg_set");
static XML::XMLString attr_address_string("address");
static XML::XMLString attr_value_string("value");
static XML::XMLString attr_write_string("write");
static XML::XMLString attr_default_string("default");
static XML::XMLString attr_decription_string("decription");

bool g_enable_verbous = false;
#define MSG(args...) if (g_enable_verbous) fprintf(stdout,args);
#define MSG_A(args...) fprintf(stdout,args);

void rp_i2c_enable_verbous(){
    g_enable_verbous = true;
}

void rp_i2c_disable_verbous(){
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

int readAttributeValue(XMLNode *node,XMLString &name,int &value){
    XMLAttribute *attr = nullptr;
    attr = node->GetAttributesByName(name);
    if (attr == nullptr) {
        MSG_A("[rp_i2c] Missing attribute %s in register\n",name.getText());
        return  -1;
    }
    sscanf(attr->ValueString().c_str(), "%x", &value);
    return 0;
}

int rp_write_to_i2c(const char* i2c_dev_path,int i2c_dev_address,int i2c_dev_reg_addr, unsigned short i2c_val_to_write, bool force){
    return write_to_i2c(i2c_dev_path,i2c_dev_address,i2c_dev_reg_addr,i2c_val_to_write , force);
}

int rp_read_from_i2c(const char* i2c_dev_path,int i2c_dev_address,int i2c_dev_reg_addr, char &value, bool force){
    return read_from_i2c(i2c_dev_path,i2c_dev_address,i2c_dev_reg_addr,value , force);
}

int read_address(XMLDocument *doc,int &device_addr, string &bus_name){
    XMLNode *bus_node = doc->FindFirstNodeByName(bus_node_name);
    if (bus_node == nullptr){
        MSG_A("[rp_i2c] Missing node bus_name in configuration file\n");
        return  -1;
    }

    XMLNode *bus_node_dev_addr = doc->FindFirstNodeByName(bus_node_dev_addr_name);
    if (bus_node_dev_addr == nullptr){
        MSG_A("[rp_i2c] Missing node device_on_bus in configuration file\n");
        return  -1;
    }else{
        XMLAttribute *attr = bus_node_dev_addr->GetAttributesByName(attr_address_string);
        if (attr == nullptr) {
            MSG_A("[rp_i2c] Missing attribute address in device_on_bus\n");
            return  -1;
        }
        sscanf(attr->ValueString().c_str(), "%x", &device_addr);
    }

    bus_name =  XMLString::toString(bus_node->GetInnerText());

    return 0;
}

int rp_i2c_load(const char *configuration_file, bool force){
    int device_addr = 0;
    string bus_name = "";

    XMLDocument *doc = readFile(configuration_file);
    if (doc == nullptr) return -1;

    if (read_address(doc,device_addr,bus_name)!= 0) return  -1;   

    XMLNode *bus_reg_set = doc->FindFirstNodeByName(bus_node_reg_set_name);
    if (bus_reg_set == nullptr){
        MSG_A("[rp_i2c] Missing node reg_set in configuration file\n");
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
            MSG_A("[rp_i2c] Missing attribute write in register\n");
            return  -1;
        }
        reg_write_mode = attr->ValueString();

        attr = reg->GetAttributesByName(attr_decription_string);
        if (attr == nullptr) {
            MSG_A("[rp_i2c] Missing attribute write in register\n");
            return  -1;
        }
        reg_description = attr->ValueString();


        if ((reg_write_mode == "value") || (reg_write_mode == "default")) {
            /* Write data to i2c */
            char data[1] = "";

            if (reg_write_mode == "value") {
                data[0] = (char)reg_value;
            }

            if (reg_write_mode == "default") {
                data[0] = (char)reg_default;
            }

            if (rp_write_to_i2c(bus_name.c_str(),device_addr, reg_addr, data[0],force) != 0) {
                /* Error process */
                MSG_A("[rp_i2c] ERROR write value of %s to i2c\n",reg_description.c_str());
            }
            MSG("[rp_i2c] Success write %s value 0x%.2X by address 0x%.2X \n",reg_description.c_str(),data[0],reg_addr);
        }else{
            MSG("[rp_i2c] Skip write %s to i2c\n",reg_description.c_str());
        }
    }

    delete doc;
    return 0;
}

int rp_i2c_print(const char *configuration_file, bool force){
    int device_addr = 0;
    string bus_name = "";

    XMLDocument *doc = readFile(configuration_file);
    if (doc == nullptr) return -1;

    if (read_address(doc,device_addr,bus_name)!= 0) return  -1; 

    XMLNode *bus_reg_set = doc->FindFirstNodeByName(bus_node_reg_set_name);
    if (bus_reg_set == nullptr){
        MSG_A("[rp_i2c] Missing node reg_set in configuration file\n");
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

        if (readAttributeValue(reg,attr_address_string,reg_addr)!=0) return -1;
    
        if (readAttributeValue(reg,attr_value_string,reg_value)!=0) return -1;

        if (readAttributeValue(reg,attr_default_string,reg_default)!=0) return -1;  

        attr = reg->GetAttributesByName(attr_write_string);
        if (attr == nullptr) {
            MSG_A("[rp_i2c] Missing attribute write in register\n");
            return  -1;
        }
        reg_write_mode = attr->ValueString();

        attr = reg->GetAttributesByName(attr_decription_string);
        if (attr == nullptr) {
            MSG_A("[rp_i2c] Missing attribute write in register\n");
            return  -1;
        }
        reg_description = attr->ValueString();
        
        if (rp_read_from_i2c(bus_name.c_str(),device_addr, reg_addr, data ,force) != 0) {
            /* Error process */
            MSG_A("[rp_i2c] ERROR read value of %s to i2c\n",reg_description.c_str());
        }
        MSG_A("[rp_i2c] Addr: 0x%.2X\tval: 0x%.2X\t%s\n",reg_addr,data,reg_description.c_str());
    }

    delete doc;
    return 0;
}

int rp_i2c_compare(const char *configuration_file, bool force){
   int device_addr = 0;
    string bus_name = "";

    XMLDocument *doc = readFile(configuration_file);
    if (doc == nullptr) return -1;

    if (read_address(doc,device_addr,bus_name)!= 0) return  -1; 

    XMLNode *bus_reg_set = doc->FindFirstNodeByName(bus_node_reg_set_name);
    if (bus_reg_set == nullptr){
        MSG_A("[rp_i2c] Missing node reg_set in configuration file\n");
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

        if (readAttributeValue(reg,attr_address_string,reg_addr)!=0) return -1;
    
        if (readAttributeValue(reg,attr_value_string,reg_value)!=0) return -1;

        if (readAttributeValue(reg,attr_default_string,reg_default)!=0) return -1;  

        attr = reg->GetAttributesByName(attr_write_string);
        if (attr == nullptr) {
            MSG_A("[rp_i2c] Missing attribute write in register\n");
            return  -1;
        }
        reg_write_mode = attr->ValueString();

        attr = reg->GetAttributesByName(attr_decription_string);
        if (attr == nullptr) {
            MSG_A("[rp_i2c] Missing attribute write in register\n");
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
        
            if (rp_read_from_i2c(bus_name.c_str(),device_addr, reg_addr, data , force) != 0) {
                /* Error process */
                MSG_A("[rp_i2c] ERROR read value of %s to i2c\n",reg_description.c_str());
            }
            if (data_in_file != data) equal_values = false;
            MSG("[rp_i2c] Addr: 0x%.2X\tvalue in i2c: 0x%.2X\tvalue in xml: 0x%.2X\t%s\n",reg_addr,data,data_in_file,reg_description.c_str());
        }
    }

    delete doc;
    
    if (equal_values)
    return 0;
        else 
    return 1;
}

}