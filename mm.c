#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * BEFORE GETTING STARTED:
 *
 * Familiarize yourself with the functions and constants/variables
 * in the following included files.
 * This will make the project a LOT easier as you go!!
 *
 * The diagram in Section 4.1 (Specification) of the handout will help you
 * understand the constants in mm.h
 * Section 4.2 (Support Routines) of the handout has information about
 * the functions in mminline.h and memlib.h
 */
#include "./memlib.h"
#include "./mm.h"
#include "./mminline.h"

block_t *prologue;
block_t *epilogue;

// rounds up to the nearest multiple of WORD_SIZE
static inline long align(long size) {
    return (((size) + (WORD_SIZE - 1)) & ~(WORD_SIZE - 1));
}

/*
 *                             _       _ _
 *     _ __ ___  _ __ ___     (_)_ __ (_) |_
 *    | '_ ` _ \| '_ ` _ \    | | '_ \| | __|
 *    | | | | | | | | | | |   | | | | | | |_
 *    |_| |_| |_|_| |_| |_|___|_|_| |_|_|\__|
 *                       |_____|
 *
 * initializes the dynamic storage allocator (allocate initial heap space)
 * arguments: none
 * returns: 0, if successful
 *         -1, if an error occurs
 */
int mm_init(void) {
    flist_first = NULL;
    // create space for prologue and epilogue, each has TAG_SIZE
    block_t *newblock = (block_t *)mem_sbrk(2 * TAGS_SIZE);
    if (newblock == NULL) {
        fprintf(stderr, "mem_sbrk");
        return -1;
    }
    prologue = newblock;
    block_set_size_and_allocated(prologue, TAGS_SIZE, 1);
    epilogue = block_next(prologue);
    block_set_size_and_allocated(epilogue, TAGS_SIZE, 1);
    return 0;
}

/*
 * create a new memory block of the specified size by calling mem_sbrk()
 * this function is typically called in mm_malloc when a suitable block is not
 * found in the free list
 *
 * arguments: size: size of the new block
 * returns: NULL, if an error occurs or size is less than MINBLOCKSIZE
 *          a pointer to the block, if successful
 */
block_t *create_new_block(long size) {
    // size is too small
    if (size < MINBLOCKSIZE) {
        return NULL;
    }
    // create a "new" block
    if (mem_sbrk(size) == (void *)-1) {
        fprintf(stderr, "mem_sbrk");
        return NULL;
    }
    // set metadata for the new block
    block_t *block = epilogue;
    block_set_size_and_allocated(block, size, 0);
    epilogue = block_next(block);
    block_set_size_and_allocated(epilogue, TAGS_SIZE, 1);
    insert_free_block(block);
    return block;
}

/*
 * split a block if the remainder is large enough to create a new free block
 *
 * arguments: blockToSplit, a pointer to the block to be split
 *            size, the requested size of the first block after splitting
 * returns: nothing
 */
void splitting(block_t *blockToSplit, long size) {
    long remainder = block_size(blockToSplit) - size;
    // remainder should be at least MINBLOCKSIZE to be useful
    if (remainder >= MINBLOCKSIZE) {
        block_set_size_and_allocated(blockToSplit, size, 1);
        block_t *freeblock = block_next(blockToSplit);
        block_set_size_and_allocated(freeblock, remainder, 0);
        insert_free_block(freeblock);
    }
}

/*     _ __ ___  _ __ ___      _ __ ___   __ _| | | ___   ___
 *    | '_ ` _ \| '_ ` _ \    | '_ ` _ \ / _` | | |/ _ \ / __|
 *    | | | | | | | | | | |   | | | | | | (_| | | | (_) | (__
 *    |_| |_| |_|_| |_| |_|___|_| |_| |_|\__,_|_|_|\___/ \___|
 *                       |_____|
 *
 * allocates a block of memory and returns a pointer to that block's payload
 * arguments: size: the desired PAYLOAD size for the block
 * returns: a pointer to the newly-allocated block's payload (whose size
 *          is a multiple of ALIGNMENT), or NULL if an error occurred
 */
void *mm_malloc(long size) {
    (void)size;  // avoid unused variable warnings
    // TODO
    block_t *block = NULL;
    if (size <= 0)
        return NULL;
    else if (size < 16)
        size = MINBLOCKSIZE;
    else
        size = align(size + TAGS_SIZE);
    // find first suitable free block
    block_t *cur = flist_first;
    if (cur) {
        do {
            if (block_size(cur) >= size) {
                block = cur;
                break;
            }
            cur = block_flink(cur);
        } while (cur != flist_first);
    }

    // a suitable block is not found (free list is null or free block is too
    // small)
    if (block == NULL) {
        block = create_new_block(size);
    }
    pull_free_block(block);
    splitting(block, size);
    block_set_allocated(block, 1);
    return block->payload;
}

/*
 * combine adjacent free blocks to reduce fragmentation
 * this function is called in mm_free()
 *
 * arguments: block, a pointer to the block to be coalesced
 * returns: NULL, if block is not free
 *          pointer to the block after coalescing, if successful
 */
block_t *coalescing(block_t *block) {
    if (block_allocated(block)) {
        return NULL;
    }
    block_t *next = block_next(block);
    block_t *prev = block_prev(block);
    if (!block_allocated(next)) {
        long combined_size = block_size(block) + block_size(next);
        block_set_size_and_allocated(block, combined_size, 0);
        pull_free_block(next);
    }
    if (!block_allocated(prev)) {
        long combined_size = block_size(block) + block_size(prev);
        block_set_size_and_allocated(prev, combined_size, 0);
        pull_free_block(block);
        block = prev;
    }
    return block;
}

/*                              __
 *     _ __ ___  _ __ ___      / _|_ __ ___  ___
 *    | '_ ` _ \| '_ ` _ \    | |_| '__/ _ \/ _ \
 *    | | | | | | | | | | |   |  _| | |  __/  __/
 *    |_| |_| |_|_| |_| |_|___|_| |_|  \___|\___|
 *                       |_____|
 *
 * frees a block of memory, enabling it to be reused later
 * arguments: ptr: pointer to the block's payload
 * returns: nothing
 */
void mm_free(void *ptr) {
    (void)ptr;  // avoid unused variable warnings
    // TODO
    if (ptr == NULL) return;
    block_t *free = payload_to_block(ptr);
    block_set_allocated(free, 0);
    insert_free_block(free);
    coalescing(free);
}

/*
 *                                            _ _
 *     _ __ ___  _ __ ___      _ __ ___  __ _| | | ___   ___
 *    | '_ ` _ \| '_ ` _ \    | '__/ _ \/ _` | | |/ _ \ / __|
 *    | | | | | | | | | | |   | | |  __/ (_| | | | (_) | (__
 *    |_| |_| |_|_| |_| |_|___|_|  \___|\__,_|_|_|\___/ \___|
 *                       |_____|
 *
 * reallocates a memory block to update it with a new given size
 * arguments: ptr: a pointer to the memory block's payload
 *            size: the desired new payload size
 * returns: a pointer to the new memory block's payload
 */
void *mm_realloc(void *ptr, long size) {
    (void)ptr, (void)size;  // avoid unused variable warnings
    // TODO
    if (ptr == NULL) {
        return mm_malloc(align(size));
    }
    if (size == 0) {
        mm_free(ptr);
        return ptr;
    }

    block_t *original_block = payload_to_block(ptr);
    long original_block_size = block_size(original_block);
    long requested_size = 0;
    // update size
    if (size < 16) {
        requested_size = MINBLOCKSIZE;
    } else {
        requested_size = align(size + TAGS_SIZE);
    }
    // compare sizes
    if (original_block_size >= requested_size) {
        return ptr;
    } else {
        block_t *next = block_next(original_block);
        block_t *prev = block_prev(original_block);
        long available_size = original_block_size;
        // update available_size
        if (!block_allocated(prev))
            available_size = block_size(original_block) + block_size(prev);
        if (!block_allocated(next))
            available_size = block_size(original_block) + block_size(next);

        // check if expansion in place is possible
        if (available_size >= requested_size) {
            // check if next is free
            if (!block_allocated(next)) {
                available_size = block_size(original_block) + block_size(next);
                if (available_size >= requested_size) {
                    pull_free_block(next);
                    block_set_size_and_allocated(original_block, available_size,
                                                 1);
                    return original_block->payload;
                }
            }
            // check if prev is free
            if (!block_allocated(prev)) {
                available_size = block_size(original_block) + block_size(prev);
                if (available_size >= requested_size) {
                    pull_free_block(prev);
                    block_set_size_and_allocated(prev, available_size, 1);
                    memmove(prev->payload, original_block->payload,
                            block_size(original_block) - TAGS_SIZE);
                    return prev->payload;
                }
            }
            // check if both prev and next are free
            if (!block_allocated(prev) && !block_allocated(next)) {
                available_size = block_size(original_block) + block_size(next) +
                                 block_size(prev);
                if (available_size >= requested_size) {
                    pull_free_block(prev);
                    pull_free_block(next);
                    block_set_size_and_allocated(prev, available_size, 1);
                    memmove(prev->payload, original_block->payload,
                            block_size(original_block) - TAGS_SIZE);
                    return prev->payload;
                }
            }
        } else {
            // call malloc to find a new block
            block_t *newblockPtr = (block_t *)mm_malloc(align(size));
            if (!newblockPtr) {
                fprintf(stderr, "malloc error");
                return NULL;
            }
            memmove(newblockPtr, original_block->payload,
                    block_size(original_block) - TAGS_SIZE);
            // free the original block
            mm_free(ptr);
            return newblockPtr;
        }
    }
    return NULL;
}
