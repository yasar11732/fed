#ifndef FED_STR
#define FED_STR

#include "fed.h"
#include <string.h>
#include <stdbool.h>

static inline _Bool strprefix(const char *str, const char *prefix) {
    size_t len = strlen(prefix);
    return strncmp(str, prefix, len) == 0;
};

static inline _Bool streq(const char *s1, const char *s2) {
    return strcmp(s1, s2) == 0;
}

static inline _Bool notnull(const void *p) {
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

static inline _Bool copypath(char * restrict dest, const char * restrict src) {
    return snprintf(dest, FED_MAXPATH, "%s", src) < FED_MAXPATH;
}

#endif
