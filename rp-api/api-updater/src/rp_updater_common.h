/**
 * $Id: $
 *
 * @brief Red Pitaya library updater api
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef __UPDATER_COMMON_API_H
#define __UPDATER_COMMON_API_H

#include <string>
#include <vector>

/** Success */
#define RP_UP_OK 0

/** Error get free space */
#define RP_UP_EGFS 1

/** Error wrong index */
#define RP_UP_EWI 2

/** Error download file */
#define RP_UP_EDF 3

/** Error open file */
#define RP_UP_EOF 4

/** Error find link */
#define RP_UP_EFL 5

/** Error calculate md5 */
#define RP_UP_ECM 6

/** Error create directory */
#define RP_UP_ECD 7

/** Error unzip file */
#define RP_UP_EUF 8

/** Remote request error. */
#define RP_UP_ERR 9

#define ECOSYSTEM_DOWNLOAD_PATH "/home/redpitaya/ecosystems"
#define ECOSYSTEM_INSTALL_PATH "/opt/redpitaya"

int rp_UpdaterGetMD5(std::string fileName, std::string* hash);
int rp_UpdaterGetMD5(const std::vector<uint8_t>& data, std::string* hash);

#endif  // __UPDATER_COMMON_API_H
