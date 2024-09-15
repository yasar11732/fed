#ifndef FED_STR
#define FED_STR

#include "fed.h"
#include <string.h>
#include <stdbool.h>

static inline _Bool streq(const char *s1, const char *s2) {
    return strcmp(s1, s2) == 0;
}

static inline _Bool notnull(const void *p) {
    return (p != NULL);
}


static inline void stripfilename(char *path) {
    int i;
    char *prev = NULL;
    for(i = 0; i < MAX_PACKAGE_NAME;i++) {
        if(path[i] == PATH_SEP) {
            path[i] = '\0';
            if(notnull(prev)) {
                *prev = PATH_SEP;
            }
            prev = &path[i];
        } else if(path[i] == '\0') {
            break;
        }
    }
}

#endif
