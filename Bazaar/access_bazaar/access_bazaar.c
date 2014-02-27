#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>

#include "redpitaya/http.h"
#include "cJSON.h"


int main(int argc, char **argv)
{
    int ret = 0;

    const char *c_bazaar_url  = "http://bazaar.redpitaya.com";
    const char *c_default_url = "http://bazaar.redpitaya.com/device";
    const char *c_bazaar_msg = "payload={\"version\":1.0,\"dna\":\"0020112e0bc1b014\",\"mac\":\"00:26:32:14:e0:00\",\"host\":\"rp\",\"apps\":{\"scope\":0.900000,\"spectrum\":0.800000,\"freqanalyzer\":0.700000}}";

    char *url = (char *)c_default_url;
    if (argc > 1) {
        url = argv[1];
    }
    post_resp_t resp = { NULL, 0 };
    ret = post(c_bazaar_msg, url, &resp);
    if (ret) {
        fprintf(stderr, "Cannot access %s.\n", url);
        if (resp.data) free(resp.data);
        return ret;;
    }

    if (resp.data) {
        //fprintf(stdout, "%s\n", resp.data);

        cJSON *json = cJSON_Parse(resp.data);
        if (!json) {
            fprintf(stderr, "Bazaar protocol error (no JSON found).\n");
        } else {

            cJSON *jtok = cJSON_GetObjectItem(json, "token");

            if (!jtok) {
                fprintf(stderr, "Bazaar protocol error (no token found).\n");
            } else {
                /* Redirect */
                fprintf(stdout, "HTTP/1.1 302 Found\r\nLocation: %s/token/%s\r\n",
                        c_bazaar_url, jtok->valuestring);
            }
        }

        if (json) cJSON_Delete(json);
        free(resp.data);
    }

    return ret;
}
