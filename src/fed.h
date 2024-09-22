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
#include "sqlite3.h"
#include <curl/curl.h>

#define FED_MAXPATH 260u
#define FED_NUMARTICLES 32u // default value for -t switch
#define FED_MAXURL 512u
#define FED_MAXDATA 0x01000000ul
#define FED_MAXPARALLEL 32u

typedef struct {
    bool inUse;
    size_t cbData;
    char url[FED_MAXURL];
    char data[FED_MAXDATA];
} transfer_t;

// program context
typedef struct {
    unsigned int numListed;
    enum {
        ASC, DESC
    } orderListed;
    
    FILE *fileUrls;
    sqlite3 *conSqlite;
    transfer_t *transfers;
    CURLM *mh;

    char pathUrls[FED_MAXPATH];
    char pathDB[FED_MAXPATH];

} fed;

static inline void init_fed(fed *f) {
    *f = (fed){0u, ASC, NULL, NULL, NULL, NULL, {'\0'}, {'\0'}};
}

static inline void init_transfer(transfer_t *t) {
    *t = (transfer_t){false, 0, {'\0'}, {'\0'}};
}

#endif
