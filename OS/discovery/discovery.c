/**
 * $Id: discovery.c 1252 2014-02-23 21:07:38Z ales.bardorfer $
 *
 * @brief Red Pitaya IP discovery utility.
 *
 * @Author Ales Bardorfer <ales.bardorfer@redpitaya.com>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>

#include "redpitaya/http.h"
#include "redpitaya/system.h"
#include "redpitaya/version.h"
#include "cJSON.h"

/**
 * GENERAL DESCRIPTION:
 *
 * Red Pitaya uses DHCP protocol to obtain the IP address and network
 * configuration in general. But since Red Pitaya is a network attached
 * device, it might be hard to know which IP it was assigned by the DHCP
 * server on your network. While you can consult the DHCP server logs, or
 * your home router (acting as DHCP server) connection status to figure out
 * the IP address of your Red Pitaya, this service provides the easy way of
 *accessing you Red Pitaya on your local network.
 *
 * The code below gathers the IP numbers of your NIC interfaces on Red Pitaya
 * and sends it to the Red Pitaya discovery server, along with your unique
 * MAC address identifyer - printed on the Ethernet connector.
 * This way, you can access your Red Pitaya in an easy way via the Red Pitaya
 * IP discovery web page: http://discovery.redpitaya.com.
 */

const char* c_nic_list = "/sys/class/net";
const char* c_primary_nic = "/sys/class/net/eth0/address";
const char* c_default_url = "http://discovery.redpitaya.com/update";


int main(int argc, char **argv)
{
    char mac[18];
    cJSON *json = NULL;
    cJSON *ips  = NULL;
    char  *js   = NULL;
    char  *jsp  = NULL;
    struct dirent *ep;
    DIR *dp = NULL;
    int ret = 0;

    /* Report version */
    fprintf(stdout, "%s version %s-%s\n", argv[0], VERSION_STR, REVISION_STR);

    /* Get MAC */
    sprintf(mac, "00:00:00:00:00:00");
    if (get_mac(c_primary_nic, mac)) {
        fprintf(stderr, "Cannot obtain MAC address.\n");
        ret = -1;
        goto out;
    }

    /* Create json template */
    json = cJSON_CreateObject();
    if (!json) {
        fprintf(stderr, "Cannot create JSON main object.\n");
        ret = -1;
        goto out;
    }
    cJSON_AddItemToObject(json, "mac", cJSON_CreateString(mac));
    cJSON_AddItemToObject(json, "ips", ips = cJSON_CreateObject());
    if (!ips) {
        fprintf(stderr, "Cannot create JSON IPs object.\n");
        ret = -1;
        goto out;
    }

    /* Populate with IPs of all the relevant interfaces */
    if((dp = opendir(c_nic_list)) == NULL) {
        fprintf(stderr, "No network interfaces found.\n");
        ret = -1;
        goto out;
    }

    while((ep = readdir (dp))) {
        const char *nic = ep->d_name;

        /* Skip irrelevant lo interface */
        if (!strncmp(nic, "lo", 2))
            continue;

        struct in_addr ip;
        if (!get_ip(nic,  &ip)) {
            cJSON_AddItemToObject(ips, nic,
                                  cJSON_CreateString(inet_ntoa(ip)));
        }
    }

    js = cJSON_Print(json);
    cJSON_Minify(js);

    jsp = malloc(strlen(js) + 8);
    if (!jsp) {
        fprintf(stderr, "Cannot malloc() payload.\n");
        goto out;
    }
    sprintf(jsp, "payload=%s", js);

    char *url = (char *)c_default_url;
    if (argc > 1) {
        url = argv[1];
    }

    post_resp_t resp = { NULL, 0 };
    ret = post(jsp, url, &resp);
    if (ret) {
        fprintf(stderr, "Cannot access %s.\n", url);
        if (resp.data) free(resp.data);
        goto out;
    }

    /* Analyze response */
    if (resp.data) {
        if (strncmp(resp.data, "OK", 2)) {
            fprintf(stderr, "Error sending data to discovery server.\n");
        } else {
            fprintf(stdout, "%s\n", resp.data);
        }
        free(resp.data);
    }

 out:
    if (jsp)  free(jsp);
    if (js)   free(js);
    if (json) cJSON_Delete(json);
    closedir(dp);
    
    return ret;
}
