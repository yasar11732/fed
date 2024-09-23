#ifndef FED_STR
#define FED_STR

#include "fed.h"
#include <string.h>
#include <stdbool.h>

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
    unsigned int i;
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
    int res = snprintf(dest, FED_MAXPATH, "%s", src);
    return (res > -1) && ((unsigned int)res < FED_MAXPATH);
}

static bool copypath2(char *dest, const char *src1, const char *src2) {
    int res = snprintf(dest, FED_MAXPATH, "%s%s", src1, src2);
    return (res > -1) && ((unsigned int)res < FED_MAXPATH);
}

static inline bool copyurl(char * dest, const char * src) {
    int res = snprintf(dest, FED_MAXURL, "%s", src);
    return (res > -1) && ((unsigned int)res < FED_MAXPATH);
}

static inline bool freadurl(char *dest, FILE *f) {

    // Ensure that the last element was not initially null
    dest[FED_MAXURL-1] = 'x';

    bool success = notnull(fgets(dest, FED_MAXURL, f));

    if(success) {
        /*
        * To ensure the integrity of the read operation:
        * 1) If the last character is non-null, the line is complete.
        * 2) If the last character is null but preceded by a newline, the line is also considered complete.
        * 3) If the last line is exactly FED_MAXURL-1 characters and is not newline-terminated, 
        *    verify that the end of the file has been reached.
        */
        success = (dest[FED_MAXURL-1] != '\0') || (dest[FED_MAXURL-1] == '\n') || feof(f);


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

    return success;

}
#endif
