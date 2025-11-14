#pragma once

#include <CustomParameters.h>
#include <DataManager.h>

#ifdef __cplusplus
extern "C" {
#endif

const char* rp_app_desc(void);
int rp_app_init(void);
int rp_app_exit(void);
void sendFilesInfo();

#ifdef __cplusplus
}
#endif
