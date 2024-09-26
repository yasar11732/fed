#ifndef FED_PARSE_ARGS
#define FED_PARSE_ARGS

#include "fed.h"
#include "str.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

static bool parse_args(fed *f,int argc, char *argv[])
{
    int i;
    bool success = notnull(f);

    for(i = 1; success && (i < argc); i++) {
        if(streq("-u", argv[i]) && (i+1 < argc)) {
            i++;
            success = path1cpy(f->pathUrls, argv[i]);
        } else if(streq("-d", argv[i]) && (i+1 < argc)) {
            i++;
            success = path1cpy(f->pathDB, argv[i]);
        } else if(streq("-t", argv[i]) && (i+1 < argc)) {
            i++;
            long t = strtol(argv[i], (char**)NULL, 10);
            if(t > 0 && t < LONG_MAX && t <= INT_MAX) { // a signed int is guaranteed to this big  
                f->numListed = (int)t;
            } else {
                success = false;
            }
        } else if(streq("-asc", argv[i])) {
            f->orderListed = ASC;
        } else if(streq("-desc", argv[i])) {
            f->orderListed = DESC;
        } else {
            success = false;
        }
    }
    return success;
}



#endif
