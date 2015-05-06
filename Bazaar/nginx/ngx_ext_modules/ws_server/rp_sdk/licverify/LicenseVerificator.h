#pragma once

#include <string>

extern "C" int verify_app_license(const char* app_id);

std::string GetLicensePath(void);
std::string GetIDFilePath(void);

