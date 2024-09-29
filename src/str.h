#ifndef FED_STR
#define FED_STR

#include "fed.h"
#include <string.h>
#include <stdbool.h>
#include <assert.h>

static inline bool strprefix(const char *str, const char *prefix) {
    size_t len = strlen(prefix);
    return strncmp(str, prefix, len) == 0;
}

static inline bool streq(const char *s1, const char *s2) {
    return strcmp(s1, s2) == 0;
}

static inline bool notnull(const void *p) {
    return (p != NULL);
}

static inline void stripfilename(char *path) {
    assert(notnull(path));  // LCOV_EXCL_LINE

    unsigned int i;
    char *prev = NULL;
    for(i = 0; i < FED_MAXPATH; i++) {
        if(path[i] == PATH_SEP) {
            path[i] = '\0';
            if(prev != NULL) {
                *prev = PATH_SEP;
            }
            prev = &path[i];
        } else if(path[i] == '\0') {
            return;
        }
    }

    // if control reaches here, we have a bug
    assert(false);  // LCOV_EXCL_LINE
}


static inline bool pathncat(char *dest, size_t argc, ...)
{
    assert(notnull(dest));  // LCOV_EXCL_LINE
    assert(argc > 0);  // LCOV_EXCL_LINE

    bool success = true;
    
    size_t len = strlen(dest);
    assert(len < FED_MAXPATH);  // LCOV_EXCL_LINE

    char *pos = &dest[len];
    size_t left = FED_MAXPATH - len - 1; // -1 for null
    
    va_list argv;
    va_start(argv, argc);

    for(size_t i = 0; i < argc; i++) {
        const char *src = va_arg(argv, const char *);
        assert(notnull(src));  // LCOV_EXCL_LINE

        size_t len_src = strlen(src);
        
        if(len_src < left) {

            /*
            * This if statement prevents an empty string
            * from being prefixed with a path separator.
            */
            if (pos != dest) {
                *pos = PATH_SEP;
                pos++;
                left--;
            }

            // this strcpy is safe because we
            // checked sizes earlier.
            strcpy(pos, src);
            pos += len_src;
            left -= len_src;
        } else {
            success = false;
            break;
        }
    }
    
    *pos = '\0';

    va_end(argv);

    return success;
}

static inline bool path1cat(char *dest, const char *src1) {
    return pathncat(dest, 1, src1);
}
static inline bool path2cat(char *dest, const char *src1, const char *src2) {
    return pathncat(dest, 2, src1, src2);
}

static inline bool path3cat(char *dest, const char *src1, const char *src2, const char *sr3) {
    return pathncat(dest, 3, src1, src2, sr3);
}

static inline bool path1cpy(char * dest, const char * src) {
    dest[0] = '\0';
    return pathncat(dest, 1, src);
}

static inline bool path2cpy(char *dest, const char *src1, const char *src2) {
    dest[0] = '\0';
    return pathncat(dest, 2, src1, src2);
}

static inline bool path3cpy(char *dest, const char *src1, const char *src2, const char *src3) {
    dest[0] = '\0';
    return pathncat(dest, 3, src1, src2, src3);
}

static inline bool copyurl(char * dest, const char * src) {
    int res = snprintf(dest, FED_MAXURL, "%s", src);
    return (unsigned int)res < FED_MAXURL;
}

#endif
