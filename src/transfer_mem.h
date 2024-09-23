#ifndef FED_TRANSFER_MEM_H
#define FED_TRANSFER_MEM_H

/*
* Functions related to allocating and freeing transfers
*/

#include "fed.h"
#include "str.h"

#include <stdint.h>
#include <stdbool.h>

static transfer_t transfer_pool[FED_MAXPARALLEL];
static uint32_t alloc_mask = 0xFFFFFFFFul;

static inline bool isBitSet(uint32_t val, unsigned int bit) {
	return (val >> bit) & 1;
}

static inline uint32_t unsetBit(uint32_t val, unsigned int bit) {
	return val & ~(1 << bit);
}

static inline uint32_t setBit(uint32_t val, unsigned int bit) {
	return val | (1 << bit);
}

static transfer_t *new_transfer(char *url) {
    unsigned int bit;
	
    // early exit if there is no free transfer
	if(alloc_mask == 0)
		return NULL;
	
	for(bit = 0; bit < 32; bit++) {
		if(isBitSet(alloc_mask, bit)) {
			if(copyurl(transfer_pool[bit].url, url)) {
                alloc_mask = unsetBit(alloc_mask, bit);
                transfer_pool[bit].cbData = 0;
                return &transfer_pool[bit];
            } else {
                return NULL;
            }
		}
	}

    // never reached, probably
    return NULL;
}

static void free_transfer(transfer_t *t) {
    alloc_mask = setBit(alloc_mask, (unsigned int)(t - transfer_pool));
}

#endif
