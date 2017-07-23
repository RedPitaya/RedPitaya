/**
 * $Id: http.c 1111 2014-02-14 09:13:19Z ales.bardorfer $
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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <curl/curl.h>

#include "http.h"


/* Debug channel */
#ifdef DEBUG
#  define PDEBUG(args) fprintf(stderr, args)
#else
#  define PDEBUG(args...) {}
#endif


/** CURL response/reception callback function */
size_t get_curl_response(void *buffer, size_t size, size_t nmemb, void *userp)
{
    http_resp_t *resp = userp;
    size_t current = resp->size;

    size *= nmemb;
    resp->size += size;

    resp->data = realloc(resp->data, resp->size + 1);
    if (resp->data == NULL)
        return 0;

    memcpy(resp->data + current, buffer, size);
    resp->data[resp->size] = '\0';

    return size;
}

/*----------------------------------------------------------------------------*/
/**
 * @brief POST a message to a specific URL and get the response.
 *
 * Function issues an HTTP POST to a specific URL, carying a specific message.
 * The response is returned to the user.
 * Caller is responsible to free resp->data!
 *
 * @param[in]    msg   Message (body) to post.
 * @param[in]    url   URL to post to.
 * @param[in]    resp  Response container. Remember to free resp->data!
 *
 * @retval   0 Success
 * @retval < 0 Failure
 */
int post(const char *msg, const char *url, http_resp_t *resp)
{
    CURL *curl = NULL;
    int ret = 0;

    curl_global_init(CURL_GLOBAL_NOTHING);
    curl = curl_easy_init();
    if (curl == NULL) {
        PDEBUG("CURL init failed.\n");
        ret = -1;
        goto out;

    }
    if (curl_easy_setopt(curl, CURLOPT_URL, url) != CURLE_OK) {
        PDEBUG("CURL set URL failed.\n");
        ret = -1;
        goto out;
    }
    if (curl_easy_setopt(curl, CURLOPT_POSTFIELDS, msg) != CURLE_OK) {
        PDEBUG("CURL set POST failed.\n");
        ret = -1;
        goto out;
    }
    if (curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
                         get_curl_response) != CURLE_OK) {
        PDEBUG("CURL set response function failed.\n");
        ret = -1;
        goto out;
    }
    if (curl_easy_setopt(curl, CURLOPT_WRITEDATA, resp) != CURLE_OK) {
        PDEBUG("CURL set response data failed.\n");
        ret = -1;
        goto out;
    }

    if (curl_easy_perform(curl)) {
        PDEBUG("CURL easy perform failed.\n");
        ret = -1;
        goto out;
    }

 out:
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return ret;
}


/*----------------------------------------------------------------------------*/
/**
 * @brief GET data from a specific URL.
 *
 * Function issues an HTTP GET to a specific URL.
 * The response is returned to the user.
 * Caller is responsible to free resp->data!
 *
 * @param[in]    url   URL to get data from.
 * @param[in]    resp  Response container. Remember to free resp->data!
 *
 * @retval   0 Success
 * @retval < 0 Failure
 */
int get(const char *url, http_resp_t *resp)
{
    CURL *curl = NULL;
    int ret = 0;

    curl_global_init(CURL_GLOBAL_NOTHING);
    curl = curl_easy_init();
    if (curl == NULL) {
        PDEBUG("CURL init failed.\n");
        ret = -1;
        goto out;

    }
    if (curl_easy_setopt(curl, CURLOPT_URL, url) != CURLE_OK) {
        PDEBUG("CURL set URL failed.\n");
        ret = -1;
        goto out;
    }
    if (curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
                         get_curl_response) != CURLE_OK) {
        PDEBUG("CURL set response function failed.\n");
        ret = -1;
        goto out;
    }
    if (curl_easy_setopt(curl, CURLOPT_WRITEDATA, resp) != CURLE_OK) {
        PDEBUG("CURL set response data failed.\n");
        ret = -1;
        goto out;
    }

    if (curl_easy_perform(curl)) {
        PDEBUG("CURL easy perform failed.\n");
        ret = -1;
        goto out;
    }

 out:
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return ret;
}


/*----------------------------------------------------------------------------*/
/*   URL encoding & decoding                                                  */
/*----------------------------------------------------------------------------*/


/** Converts a hex character to its integer value */
char from_hex(char ch) {
  return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}


/** Converts an integer value to its hex character */
char to_hex(char code) {
  static char hex[] = "0123456789abcdef";
  return hex[code & 15];
}


/** Returns a url-encoded version of str
 * IMPORTANT: be sure to free() the returned string after use
 */
char *url_encode(char *str) {
  char *pstr = str, *buf = malloc(strlen(str) * 3 + 1), *pbuf = buf;
  while (*pstr) {
    if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~') 
      *pbuf++ = *pstr;
    else if (*pstr == ' ') 
      *pbuf++ = '+';
    else 
      *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15);
    pstr++;
  }
  *pbuf = '\0';
  return buf;
}


/** Returns a url-decoded version of str
 * IMPORTANT: be sure to free() the returned string after use
 */
char *url_decode(char *str) {
  char *pstr = str, *buf = malloc(strlen(str) + 1), *pbuf = buf;
  while (*pstr) {
    if (*pstr == '%') {
      if (pstr[1] && pstr[2]) {
        *pbuf++ = from_hex(pstr[1]) << 4 | from_hex(pstr[2]);
        pstr += 2;
      }
    } else if (*pstr == '+') { 
      *pbuf++ = ' ';
    } else {
      *pbuf++ = *pstr;
    }
    pstr++;
  }
  *pbuf = '\0';
  return buf;
}
