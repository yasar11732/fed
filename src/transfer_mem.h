/*
 * SPDX-FileCopyrightText: 2024 Yaşar Arabacı <yasar11732@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef FED_TRANSFER_MEM_H
#define FED_TRANSFER_MEM_H

/*
* Functions responsible for managing transfer allocation and deallocation.
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

static transfer_t *new_transfer(void) {

    unsigned int bit;

    if(alloc_mask == 0)
        return NULL;

    for(bit = 0; bit < 32; bit++) { // LCOV_EXCL_LINE
		if(isBitSet(alloc_mask, bit)) {
            alloc_mask = unsetBit(alloc_mask, bit);
            init_transfer(&transfer_pool[bit]);
            return &transfer_pool[bit];
		}
	}
    // not reachable, added to make compiler happy
    return NULL; // LCOV_EXCL_LINE
}

static void free_transfer(transfer_t *t) {
    assert(t >= transfer_pool); // LCOV_EXCL_LINE
    assert((t - transfer_pool) < 32);  // LCOV_EXCL_LINE
    alloc_mask = setBit(alloc_mask, (unsigned int)(t - transfer_pool));
}

#endif
