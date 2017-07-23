/**
 * $Id$
 *
 * @brief Red Pitaya HTTP utility library.
 *
 * @Author Ales Bardorfer <ales.bardorfer@redpitaya.com>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef REDPITAYA_HTTP_H
#define REDPITAYA_HTTP_H


/** HTTP response container */
typedef struct {
    char *data;
    size_t size;
} http_resp_t;


typedef http_resp_t post_resp_t;
typedef http_resp_t get_resp_t;


int post(const char *msg, const char *url, http_resp_t *resp);
int get(const char *url, http_resp_t *resp);
char *url_encode(char *str);
char *url_decode(char *str);

#endif /* REDPITAYA_HTTP_H */
