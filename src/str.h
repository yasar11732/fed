#ifndef FED_STR
#define FED_STR

#include "fed.h"
#include <string.h>
#include <stdbool.h>

static inline bool strprefix(const char *str, const char *prefix) {
    size_t len = strlen(prefix);
    return strncmp(str, prefix, len) == 0;
};

static inline bool streq(const char *s1, const char *s2) {
    return strcmp(s1, s2) == 0;
}

static inline bool notnull(const void *p) {
    return (p != NULL);
}

static inline void stripfilename(char *path) {
    int i;
    int k = -1;
    for(i = 0; i < FED_MAXPATH; i++) {
        if(path[i] == PATH_SEP) {
            path[i] = '\0';
            if(k != -1) {
                path[k] = PATH_SEP;
            }
            k = i;
        } else if(path[i] == '\0') {
            break;
        }
    }
}

static inline bool copypath(char * dest, const char * src) {
    return snprintf(dest, FED_MAXPATH, "%s", src) < FED_MAXPATH;
}

static inline bool copyurl(char * dest, const char * src) {
    return snprintf(dest, FED_MAXURL, "%s", src) < FED_MAXURL;
}

#endif
