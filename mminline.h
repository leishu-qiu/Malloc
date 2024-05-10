#ifndef MMINLINE_H_
#define MMINLINE_H_
#pragma GCC diagnostic ignored "-Warray-bounds"
#pragma GCC diagnostic ignored "-Wuninitialized"
#include <assert.h>
#include "mm.h"

static block_t *flist_first;  // head of circular, doubly linked free list
extern block_t *prologue;
extern block_t *epilogue;

/*
 * |_ | _  _ ;_/     _ ._  _|    -+- _  _
 * [_)|(_)(_ | \____(/,[ )(_]____ | (_](_]
 *                                     ._|
 * Returns a pointer to the block's end tag
 * (you probably won't need to use this directly)
 */
static inline long *block_end_tag(block_t *b) {
    assert(b->size >= (WORD_SIZE * 2));
    // TODO: Implement this function!
    return &b->payload[b->size / 8 - 2];
}

/*
 *
 * |_ | _  _ ;_/     _.|| _  _  _.-+- _  _|
 * [_)|(_)(_ | \____(_]||(_)(_ (_] | (/,(_]
 *
 * Returns 1 if block is allocated, 0 otherwise
 * (in other words, returns 1 if the right-most bit in b->size is set, 0
 * otherwise)
 */
static inline int block_allocated(block_t *b) {
    // TODO: Implement this function!
    return b->size & 1;
}

/*
 * |_ | _  _.;_/     _ ._  _|     _.|| _  _. _.-+- _  _|
 * [_)|(_)(_.| \____(/,[ )(_]____(_]||(_)(_.(_] | (/,(_]
 *
 * Same as the above, but checks the end tag of the block
 */
static inline int block_end_allocated(block_t *b) {
    // TODO: Implement this function!
    assert(b->size >= (WORD_SIZE * 2));
    return *(block_end_tag(b)) & 1;
}

/*
 * |_ | _  _.;_/    __*__. _
 * [_)|(_)(_.| \____) | /_(/,
 *
 * Returns the size of the entire block
 * NOTE: if the block is allocated, the right-most bit in b->size will be 1
 * Think about how to remove this bit from b->size while preserving the rest of
 * its bits (hint: -2 is 111...1110 in binary)
 */
static inline long block_size(block_t *b) {
    // TODO: Implement this function!
    return b->size & -2;
}

/*
 * |_ | _  _.;_/     _ ._  _|     __*__. _
 * [_)|(_)(_.| \____(/,[ )(_]_____) | /_(/,
 *
 * Same as block_size, but uses the end tag of the block
 * Think about what functions you've implemented above that could be useful to
 * call here
 */
static inline long block_end_size(block_t *b) {
    // TODO: Implement this function!
    return *(block_end_tag(b)) & -2;
}

/*
 * |_ | _  _.;_/     __ _ -+-     __*__. _
 * [_)|(_)(_.| \_____) (/, | _____) | /_(/,
 *
 * Sets the entire size of the block at both the beginning and the end tags
 * Make sure to set the right-most bit of the size to be 1 if the block is
 * marked as allocated
 */
static inline void block_set_size(block_t *b, long size) {
    // TODO: Implement this function
    assert((size & (ALIGNMENT - 1)) == 0);
    size |= block_allocated(b);  // preserve alloc bit
    b->size = size;
    *block_end_tag(b) = size;
}

/*
 * |_ | _  _.;_/     __ _ -+-     _.|| _  _. _.-+- _  _|
 * [_)|(_)(_.| \_____) (/, | ____(_]||(_)(_.(_] | (/,(_]
 *
 * Sets the allocated flags of the block, at both the beginning and the end
 * tags. Depending on whether the block is being marked as allocated or free,
 * you should set the rightmost bit of the size (at beginning and end tags)
 * accordingly (hint: -2 is 111...1110 in binary)
 */
static inline void block_set_allocated(block_t *b, int allocated) {
    assert((allocated == 0) || (allocated == 1));
    // TODO: Implement this function
    if (allocated) {
        b->size |= 1;
        *block_end_tag(b) |= 1;
    } else {
        b->size &= -2;
        *block_end_tag(b) &= -2;
    }
}

/*
 * |_ | _  _.;_/   __ _ -+-   __*__. _    _.._  _|   _.|| _  _. _.-+- _  _|
 * [_)|(_)(_.| \___) (/, | ___) | /_(/,__(_][ )(_]__(_]||(_)(_.(_] | (/,(_]
 *
 * Sets the entire size of the block and sets the allocated flags of the block,
 * at both the beginning and the end.
 * This is a convenience function to set the size and allocation of a block in
 * one call -- you can call functions you've already implemented here!
 */
static inline void block_set_size_and_allocated(block_t *b, long size,
                                                int allocated) {
    // TODO: Implement this function
    block_set_size(b, size);
    block_set_allocated(b, allocated);
}

/*
 * |_ | _  _.;_/    ._ ._. _ .  ,     _.|| _  _. _.-+- _  _|
 * [_)|(_)(_.| \____[_)[  (/, \/ ____(_]||(_)(_.(_] | (/,(_]
 *                  |
 * Returns 1 if the previous block is allocated, 0 otherwise
 */
static inline int block_prev_allocated(block_t *b) {
    // TODO: Implement this function
    return b->payload[-2] & 1;
}

/*
 * |_ | _  _.;_/    _ ._. _ .  ,   __*__. _
 * [_)|(_)(_.| \___[_)[  (/, \/ ___) | /_(/,
 *                  |
 * Returns the size of the previous block.
 * NOTE: similar to block_size, you need to remove the right-most bit (the
 * 'is-allocated' bit) (remember -2 is 111...1110 in binary)
 */
static inline long block_prev_size(block_t *b) {
    // TODO: Implement this function
    return b->payload[-2] & -2;
}

/*
 * |_ | _  _.;_/    ._ ._. _ .  ,
 * [_)|(_)(_.| \____[_)[  (/, \/
 *                  |
 * Returns a pointer to the previous block
 */
static inline block_t *block_prev(block_t *b) {
    // TODO: Implement this function
    return (block_t *)((char *)b - block_prev_size(b));
}

/*
 * |_ | _  _.;_/    ._  _ \./-+-
 * [_)|(_)(_.| \____[ )(/,/'\ |
 *
 * Returns a pointer to the next block
 */
static inline block_t *block_next(block_t *b) {
    // TODO: Implement this function
    return (block_t *)((char *)b + block_size(b));
}

/*
 * |_ | _  _.;_/    ._  _ \./-+-     _.|| _  _. _.-+- _  _|
 * [_)|(_)(_.| \____[ )(/,/'\ | ____(_]||(_)(_.(_] | (/,(_]
 *
 * Returns 1 if the next block is allocated; 0 if not
 */
static inline int block_next_allocated(block_t *b) {
    // TODO: Implement this function
    return block_allocated(block_next(b));
}

/*
 * |_ | _  _.;_/    ._  _ \./-+-     __*__. _
 * [_)|(_)(_.| \____[ )(/,/'\ | _____) | /_(/,
 *
 * Returns the size of the next block
 */
static inline long block_next_size(block_t *b) {
    // TODO: Implement this function
    return block_size(block_next(b));
}

/*
 * ._  _.  .| _  _. _|    -+- _     |_ | _  _.;_/
 * [_)(_]\_||(_)(_](_]____ | (_)____[_)|(_)(_.| \
 * |     ._|
 *
 * Given a pointer to the payload, returns a pointer to the block
 */
static inline block_t *payload_to_block(void *payload) {
    // TODO: Implement this function
    return (block_t *)((char *)payload - WORD_SIZE);
    ;
}

/*
 * |_ | _  _.;_/    |,|*._ ;_/
 * [_)|(_)(_.| \____| ||[ )| \
 *
 * Given the input block 'b', returns b's flink, which points to the next block
 * in the free list.
 * NOTE: if 'b' is free, b->payload[0] contains b's flink
 */
static inline block_t *block_flink(block_t *b) {
    assert(!block_allocated(b));
    // TODO: Implement this function
    return (block_t *)b->payload[0];
}

/*
 * |_ | _  _.;_/    __ _ -+-    |,|*._ ;_/
 * [_)|(_)(_.| \____) (/, | ____| ||[ )| \
 *
 * Given the inputs 'b' and 'new_flink', sets b's flink to now point
 * to new_flink, which should be the next block in the free list
 */
static inline void block_set_flink(block_t *b, block_t *new_flink) {
    assert(!block_allocated(b) && !block_allocated(new_flink));
    // TODO: Implement this function
    b->payload[0] = (long)new_flink;
}

/*
 * |_ | _  _.;_/    |_ |*._ ;_/
 * [_)|(_)(_.| \____[_)||[ )| \
 *
 * Given the input block 'b', returns b's blink, which points to the
 * previous block in the free list.
 * NOTE: if 'b' is free, b->payload[1] contains b's blink
 */
static inline block_t *block_blink(block_t *b) {
    assert(!block_allocated(b));
    // TODO: Implement this function
    return (block_t *)b->payload[1];
}

/*
 * |_ | _  _.;_/     __ _ -+-    |_ |*._ ;_/
 * [_)|(_)(_.| \_____) (/, | ____[_)||[ )| \
 *
 * Given the inputs 'b' and 'new_blink', sets b's blink to now point
 * to new_blink, which should be the previous block in the free list
 */
static inline void block_set_blink(block_t *b, block_t *new_blink) {
    assert(!block_allocated(b) && !block_allocated(new_blink));
    // TODO: Implement this function
    b->payload[1] = (long)new_blink;
}

/*
 * ._ . .||    |,._. _  _     |_ | _  _.;_/
 * [_)(_|||____| [  (/,(/,____[_)|(_)(_.| \
 * |
 *
 * Pull a block from the (circularly doubly linked) free list
 */
static inline void pull_free_block(block_t *fb) {
    assert(!block_allocated(fb));
    // TODO: Implement this function
    block_t *flink = block_flink(fb);
    block_t *blink = block_blink(fb);
    if (flink == fb) {
        flist_first = NULL;
    } else {
        block_set_flink(blink, flink);
        if (flist_first == fb) {
            flist_first = flink;  // Update the head of the free list
        }
    }
    block_set_blink(flink, blink);
}

/*
 *
 * . _  __ _ ._.-+-    |,._. _  _     |_ | _  _.;_/
 * |[ )_) (/,[   | ____| [  (/,(/,____[_)|(_)(_.| \
 *
 * Insert block into the (circularly doubly linked) free list
 */
static inline void insert_free_block(block_t *fb) {
    assert(!block_allocated(fb));
    // TODO: Implement this function
    if (flist_first != NULL) {
        block_t *last = block_blink(flist_first);
        block_set_flink(fb, flist_first);
        block_set_blink(fb, last);
        block_set_flink(last, fb);
        block_set_blink(flist_first, fb);
    } else {
        block_set_flink(fb, fb);
        block_set_blink(fb, fb);
    }
    flist_first = fb;
}

#endif  // MMINLINE_H_
