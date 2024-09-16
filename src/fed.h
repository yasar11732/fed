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

#include <stdio.h> // for FILE definition
#include "sqlite3.h"

#define FED_MAXPATH 260

#define FED_NUM_ARTICLES 32 // default value for -t switch

// program context
typedef struct {
    char pathUrls[FED_MAXPATH];
    char pathDB[FED_MAXPATH];
    FILE *fileUrls;
    sqlite3 *conSqlite;
    unsigned int numListed;
    enum {
        ASC, DESC
    } orderListed;
} fed;

#endif
