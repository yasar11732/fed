#ifdef NDEBUG
#undef NDEBUG
#endif

#include "transfer_mem.h"
#include <assert.h>
#include <stdio.h>

int main() {

    transfer_t *returned_transfers[FED_MAXPARALLEL];

    puts("Test can alloc FED_MAXPARALLEL");
    for(int i = 0; i < FED_MAXPARALLEL; i++) {
        returned_transfers[i] = new_transfer("");
        assert(notnull(returned_transfers[i]));
    }

    puts("Test alloc_mask is clear");
    assert(alloc_mask == 0);

    puts("Test no duplicates returned");
    for(int i = 0; i < FED_MAXPARALLEL; i++) {
        for(int k = i+1; k < FED_MAXPARALLEL; k++) {
            assert(returned_transfers[i] != returned_transfers[k]);
        }
    }

    puts("Test freeing all resets alloc mask");
    for(int i = 0; i < FED_MAXPARALLEL; i++) {
        free_transfer(returned_transfers[i]);
    }
    assert(alloc_mask == 0xFFFFFFFF);
}