#ifdef NDEBUG
#undef NDEBUG
#endif

#include "transfer_mem.h"
#include <assert.h>
#include <stdio.h>

int main(void) {

    transfer_t *returned_transfers[FED_MAXPARALLEL];

    puts("Test can alloc FED_MAXPARALLEL");
    for(size_t i = 0; i < FED_MAXPARALLEL; i++) {
        returned_transfers[i] = new_transfer();
        assert(notnull(returned_transfers[i]));
    }

    puts("Test alloc_mask is clear");
    assert(alloc_mask == 0);

    puts("Test no duplicates returned");
    for(size_t i = 0; i < FED_MAXPARALLEL; i++) {
        for(size_t k = i+1; k < FED_MAXPARALLEL; k++) {
            assert(returned_transfers[i] != returned_transfers[k]);
        }
    }
    puts("Test allocation more return null");
    assert(new_transfer() == NULL);
    
    puts("Test freeing all resets alloc mask");
    for(size_t i = 0; i < FED_MAXPARALLEL; i++) {
        free_transfer(returned_transfers[i]);
    }
    assert(alloc_mask == 0xFFFFFFFF);

    puts("Test returned value is cleared.");
    // make sure all available transfers are used first
    for(size_t i = 0; i < FED_MAXPARALLEL; i++) {
        returned_transfers[i] = new_transfer();
        assert(notnull(returned_transfers[i]));
    }
    returned_transfers[0]->cbData = 42;
    free_transfer(returned_transfers[0]);
    // this will return the same one we freed
    // because there isn't another free transfer
    returned_transfers[0] = new_transfer();
    assert(returned_transfers[0]->cbData == 0);
}
