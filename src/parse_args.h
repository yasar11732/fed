#ifndef FED_PARSE_ARGS
#define FED_PARSE_ARGS

#include "fed.h"
#include "str.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

static _Bool parse_args(fed *f,int argc, char *argv[])
{
    int i;
    for(i = 1; i < argc; i++) {
        if(streq("-u", argv[i]) && (i+1 < argc)) {
            i++;
            strncpy(f->pathUrls, argv[i], FED_MAXPATH);
        } else if(streq("-d", argv[i]) && (i+1 < argc)) {
            i++;
            strncpy(f->pathDB, argv[i], FED_MAXPATH);
        } else if(streq("-t", argv[i]) && (i+1 < argc)) {
            i++;
            long t = strtol(argv[i], (char**)NULL, 10);
            if(t > 0 && t <= 32767) { // a signed int is guaranteed to this big  
                f->numListed = (int)t;
            } else {
                return false;
            }
        } else if(streq("-asc", argv[i])) {
            f->orderListed = ASC;
        } else if(streq("-desc", argv[i])) {
            f->orderListed = DESC;
        } else {
            return false;
        }
    }
    return true;
}



#endif
