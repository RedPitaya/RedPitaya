/**
 * $Id$
 *
 * @brief Red Pitaya system utility library.
 *
 * @Author Ales Bardorfer <ales.bardorfer@redpitaya.com>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef REDPITAYA_SYSTEM_H
#define REDPITAYA_SYSTEM_H

#include <netinet/in.h>

int get_mac(const char* nic, char *mac);
int get_ip(const char *nic, struct in_addr *ip);

#endif /* REDPITAYA_SYSTEM_H */
