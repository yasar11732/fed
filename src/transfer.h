#ifndef FED_TRANSFER_H
#define FED_TRANSFER_H
#include "fed.h"
#include "str.h"
#include "xmlparse.h"
#include "transfer_mem.h"



static size_t header_cb(char *buffer, size_t size, size_t nitems, void *userdata) {

    (void)size;

    transfer_t *t = (transfer_t *)userdata;
    
    if(strprefix(buffer,"etag:") || strprefix(buffer,"ETag:")) {
        copy_header_value(t->etag, FED_MAXETAG, buffer, nitems);
    }

    if(strprefix(buffer,"last-modified:") || strprefix(buffer,"Last-Modified:")) {
        copy_header_value(t->lastmodified, FED_MAXTIMESTRING, buffer, nitems);
    }
    
    return nitems;
}

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

static inline bool freadurl(char *dest, FILE *f) {
    assert(notnull(dest)); // LCOV_EXCL_LINE
    assert(notnull(f)); // LCOV_EXCL_LINE

    // to test for null later
    dest[FED_MAXURL - 1] = 'x';

    bool success = notnull(fgets(dest, FED_MAXURL, f));

    if(success) {
        /*
        * To ensure the integrity of the read operation:
        * 1) If the last character is non-null, the line is complete.
        * 2) If the last character is null but preceded by a newline,
        *    the line is also considered complete.
        * 3) If the last line in file was exactly FED_MAXURL-1 characters
        *    and is not newline-terminated, verify that the end of the
        *    file has been reached.
        */
        success = (dest[FED_MAXURL-1] != '\0') || (dest[FED_MAXURL-2] == '\n') || feof(f);


        /*
        * Due to reading a partial line, additional characters
        * must be consumed until the end-of-line delimiter is
        * reached, ensuring proper alignment for the subsequent line.
        */
        if(!success) {
            int c;
            do {
                c = fgetc(f);
            } while((c > 0) && (c !='\n'));
        }
    }

    /*
    * Remove newline character
    */
    if(success) {
        char *p = dest;
        while(*p) {
            if((*p == '\r') || (*p == '\n')) {
                *p = '\0';
                break;
            }
            p++;
        }
    }

    return success;

}



static bool add_transfer(fed *f) {
    assert(notnull(f)); // LCOV_EXCL_LINE
    
    transfer_t *t = new_transfer();
    bool success = notnull(t);

    CURL *eh = NULL;

    if(success) {
        init_transfer(t);
        t->fed = f;
        success = freadurl(t->url, f->fileUrls);
    }

    if(success) {
        success = get_feed_details(t);
    }

    if(success) {
        if(t->etag[0] != '\0') {
            char buf_etag[FED_MAXETAG + 15];
            snprintf(buf_etag, FED_MAXETAG + 15, "If-None-Match: %s", t->etag);
            t->headers = curl_slist_append(t->headers, buf_etag);
        }

        if(t->lastmodified[0] != '\0') {
            char buf_lastmodified[FED_MAXTIMESTRING + 19];
            snprintf(buf_lastmodified, FED_MAXTIMESTRING + 19, "If-Modified-Since: %s", t->lastmodified);
            t->headers = curl_slist_append(t->headers, buf_lastmodified);
        }
    }

    if(success) {
        success = notnull(eh = curl_easy_init());
    }

    if(success) {
        // curl_easy_setopt(eh, CURLOPT_PROXY,"http://127.0.0.1:8080");
        // curl_easy_setopt(eh, CURLOPT_SSL_VERIFYPEER, 0L);
        success = curl_ok(curl_easy_setopt(eh, CURLOPT_URL, t->url));
    }

    if(success) {
        success = curl_ok(curl_easy_setopt(eh, CURLOPT_PRIVATE, t));
    }

    if(success) {
      success = curl_ok(curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, write_cb));
    }

    if (success) {
        success = curl_ok(curl_easy_setopt(eh, CURLOPT_WRITEDATA, t));
    }

    if(success) {
      success = curl_ok(curl_easy_setopt(eh, CURLOPT_HEADERFUNCTION, header_cb));
    }

    if (success) {
        success = curl_ok(curl_easy_setopt(eh, CURLOPT_HEADERDATA, t));
    }

    if(success) {

        /*
        * Optional options (no pun intended, probably)
        */

        if(t->headers != NULL) {
            (void)curl_easy_setopt(eh, CURLOPT_HTTPHEADER, t->headers);
        }

       (void)curl_easy_setopt(eh, CURLOPT_ACCEPT_ENCODING, "");
       (void)curl_easy_setopt(eh, CURLOPT_TIMEOUT, 20L);
       (void)curl_easy_setopt(eh, CURLOPT_FOLLOWLOCATION, 1L);

#if LIBCURL_VERSION_NUM >= 0x075500
       (void)curl_easy_setopt(eh, CURLOPT_PROTOCOLS_STR, "http,https");
#elif LIBCURL_VERSION_NUM >= 0x071304
        (void)curl_easy_setopt(eh, CURLOPT_PROTOCOLS, CURLPROTO_HTTP|CURLPROTO_HTTPS);
#endif

       (void)curl_easy_setopt(eh, CURLOPT_AUTOREFERER, 1L);
       (void)curl_easy_setopt(eh, CURLOPT_MAXREDIRS, 10L);
       (void)curl_easy_setopt(eh, CURLOPT_CONNECTTIMEOUT_MS, 2000L);
       (void)curl_easy_setopt(eh, CURLOPT_USERAGENT, "feed aggregator");
    }

    if(success) {
        success = curl_multi_add_handle(f->mh, eh) == CURLM_OK;
    }

    if(success) {
        f->runningHandles++;
    }

    if(!success && notnull(eh)) {
        curl_easy_cleanup(eh);
    }

    if(!success && notnull(t)) {
        free_transfer(t);
    }

    return success;
}

static inline void add_transfers(fed *f)
{
  while((f->runningHandles < FED_MAXPARALLEL) && (feof(f->fileUrls) == 0)) {
    add_transfer(f);
  }
}

#endif
