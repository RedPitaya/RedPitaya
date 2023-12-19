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


#ifndef __ARB_API_H
#define __ARB_API_H

#include <string>
#include "rp.h"

#define RP_ARB_FILE_OK 0
#define RP_ARB_FILE_ERR 1
#define RP_ARB_FILE_TO_LONG 2
#define RP_ARB_FILE_PARSE_ERR 3
#define RP_ARB_FILE_CANT_RENAME 4
#define RP_ARB_FILE_SOME_NAME 5
#define RP_ARB_WRONG_INDEX 6
#define RP_ARB_ERROR_LOAD 7


int rp_ARBInit();
int rp_ARBGenFile(std::string _filename);
int rp_ARBLoadFiles();
int rp_ARBGetCount(uint32_t *_count);
int rp_ARBGetName(uint32_t _index,std::string *_name);
int rp_ARBGetFileName(uint32_t _index,std::string *_fileName);
int rp_ARBGetSignal(uint32_t _index,float *_data,uint32_t *_size);
int rp_ARBGetSignalByName(std::string _sigName,float *_data,uint32_t *_size);
int rp_ARBRenameFile(uint32_t _index,std::string _new_name);
int rp_ARBLoadToFPGA(rp_channel_t _channel, std::string _sigName);
int rp_ARBIsValid(std::string _sigName,bool *_valid);

#endif // __ARB_API_H
