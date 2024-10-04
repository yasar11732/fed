#ifndef FED_SETTINGS_H
#define FED_SETTINGS_H

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define ON_WINDOWS
#endif

#ifdef ON_WINDOWS
#include <windows.h>
#define PATH_SEP '\\'
#else
#define PATH_SEP '/'
#endif // defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

#include <stdio.h> // FILE
#include <stdbool.h> // bool
#include <string.h> // memset
#include "sqlite3.h"
#include <curl/curl.h>

#define FED_MAXPATH 260u
#define FED_NUMARTICLES 32u // default value for -t switch
#define FED_MAXURL 512
#define FED_MAXETAG 128
#define FED_MAXDATA 0x01000000ul
#define FED_MAXPARALLEL 32 // If you change this, you must fix transfer_mem.h as it assumes this is 32

#define FED_MAXTITLE 256
#define FED_MAXTIMESTRING 32

// program context
typedef struct {
    unsigned int numListed;
    int runningHandles;
    enum {
        ASC, DESC
    } orderListed;
    
    FILE *fileUrls;
    sqlite3 *conSqlite;
    CURLM *mh;

    char pathUrls[FED_MAXPATH];
    char pathDB[FED_MAXPATH];

} fed;

typedef struct {
    int feed_id;
    size_t cbData;
    fed *fed;
    struct curl_slist *headers;
    char lastmodified[FED_MAXTIMESTRING];
    char etag[FED_MAXETAG];
    char url[FED_MAXURL];
    char data[FED_MAXDATA];
} transfer_t;



static inline void init_fed(fed *f) {
    *f = (fed){0u, 0, ASC, NULL, NULL, NULL, {'\0'}, {'\0'}};
}

static inline void init_transfer(transfer_t *t) {
    t->feed_id = 0;
    t->cbData = 0;
    t->fed = NULL;
    t->headers = NULL;
    t->lastmodified[0] = '\0';
    t->etag[0] = '\0';
    t->url[0] = '\0';
    t->data[0] = '\0';
}

#endif
