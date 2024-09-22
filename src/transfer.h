#ifndef FED_TRANSFER_H
#define FED_TRANSFER_H
#include "fed.h"
#include "str.h"

static transfer_t global_transfers[FED_MAXPARALLEL];

static void init_transfers(transfer_t *t) {
    for(size_t i = 0; i < FED_MAXPARALLEL; i++) {
        init_transfer(&t[i]);
    }
}

static transfer_t *new_transfer(transfer_t *t, char *url) {
    /*
    * Not very efficient, but good enough for now
    */
    transfer_t *t_new = NULL;
    for(size_t i = 0; i < FED_MAXPARALLEL; i++) {
        if(!(t[i].inUse)) {
            if(copyurl(t[i].url, url)) {
                t[i].inUse = true;
                t_new = &t[i];
            }
            break;
        }
    }
    return t;
}

static void free_transfer(transfer_t *t) {
    t->inUse = false;
    t->cbData = 0u;
}

static bool add_transfer(CURLM *mh, char *url) {
    bool success = notnull(mh) && notnull(url);
    
}

#endif