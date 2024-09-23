#ifndef FED_TRANSFER_H
#define FED_TRANSFER_H
#include "fed.h"
#include "str.h"
#include "transfer_mem.h"

static size_t write_cb(const char *data, size_t size, size_t nmemb,
                       void *userdata) {

  transfer_t *t = (transfer_t *)userdata;

  size_t realsize = size * nmemb;
  size_t copiedsize = 0;

  if((FED_MAXDATA - t->cbData) < realsize) {
    copiedsize = FED_MAXDATA - t->cbData;
  } else {
    copiedsize = realsize;
  }

  if (copiedsize > 0) {
    memcpy(&(t->data[t->cbData]), data, copiedsize);
    t->cbData += copiedsize;
  }

  return copiedsize;
}

static inline bool curl_ok(CURLcode c) {
    return c == CURLE_OK;
}

static bool add_transfer(CURLM *mh, char *url) {
    bool success = notnull(mh) && notnull(url);
    CURL *eh = NULL;
    transfer_t *t;
    if(success) {
        success = notnull(t = new_transfer(url));
    }

    if(success) {
        success = notnull(eh = curl_easy_init());
    }

    if(success) {
        success = curl_ok(curl_easy_setopt(eh, CURLOPT_URL, url));
    }

    if(success) {
        success = curl_ok(curl_easy_setopt(eh, CURLOPT_PRIVATE, t));
    }

    if(success) {
      success = curl_ok(curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, write_cb));
    }

    if(success) {
        /*
        * Optional options (no pun intended, probably)
        */
       (void)curl_easy_setopt(eh, CURLOPT_TIMEOUT, 20L);
       (void)curl_easy_setopt(eh, CURLOPT_FOLLOWLOCATION, 1L);
       (void)curl_easy_setopt(eh, CURLOPT_PROTOCOLS_STR, "http,https");
       (void)curl_easy_setopt(eh, CURLOPT_AUTOREFERER, 1L);
       (void)curl_easy_setopt(eh, CURLOPT_MAXREDIRS, 10L);
       (void)curl_easy_setopt(eh, CURLOPT_CONNECTTIMEOUT_MS, 2000L);
       (void)curl_easy_setopt(eh, CURLOPT_USERAGENT, "feed aggregator");
    }

    if(success) {
        success = curl_multi_add_handle(mh, eh) == CURLM_OK;
    }

    if(!success && notnull(eh)) {
        curl_easy_cleanup(eh);
    }

    if(!success && notnull(t)) {
        free_transfer(t);
    }

    return success;
}

#endif