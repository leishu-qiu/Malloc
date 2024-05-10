```
 __  __       _ _
|  \/  | __ _| | | ___   ___
| |\/| |/ _` | | |/ _ \ / __|
| |  | | (_| | | | (_) | (__
|_|  |_|\__,_|_|_|\___/ \___|
```

# The main code is contained in mm.c.

HOW TO RUN: Use the provided Makefile to compile the program. The main code is contained in mm.c. Run mdriver to test the memory allocation compaction.

## 1. Strategy for Maintaining Compaction:

Splitting and coalescing are key strategies used to maintain a compact heap. During splitting, the size of the remaining free block is evaluated to determine if it exceeds the minimum block size. If it does not, the block remains unsplit to avoid the creation of unusefully small free blocks. If it does exceed the minimum size, the block is split into two: one block meeting the requested size and another as a newly available free block. Coalescing involves checking adjacent blocks; if they are free, they are merged. This process is critical for consolidating free space and minimizing fragmentation in the heap.

## 2. mm_realloc() Implementation Strategy:

The mm_realloc() function starts by handling two edge cases: a null pointer triggers a call to mm_malloc(), and a size of zero invokes mm_free(). The core operation of this function involves comparing the size of the current memory block with the requested size (adjusted for TAG_SIZE). If the current block is sufficiently large or exactly matches the requested size, the existing pointer is returned unchanged.

If the current block is too small, the function checks whether it can be expanded in place by utilizing adjacent free space. This involves assessing three scenarios based on the availability of neighboring free blocks: only the next block is free, only the previous block is free, or both adjacent blocks are free. If any of these scenarios are applicable, the adjacent free blocks are coalesced to accommodate the required size expansion.

In scenarios where the previous block or both adjacent blocks are free, memmove() is used to shift the block's contents appropriately. If in-place expansion is not feasible due to insufficient space, a new block is allocated using mm_malloc(), and the contents of the original block are moved to this new block using memmove(). The original block is then released by calling mm_free().

## 3. Strategy to achieve high throughput:

The mm_malloc() function employs a first-fit search algorithm to identify a free block that closely matches the requested size. This approach minimizes the time spent searching and reduces fragmentation, enhancing the efficiency of memory usage. In-place expansion, utilized within mm_realloc(), further contributes to throughput by decreasing the frequency and cost associated with allocating new blocks and transferring data. Additionally, the coalescing process effectively merges adjacent free blocks, streamlining the allocation process and significantly reducing the time required to locate suitable memory spaces. Together, these strategies facilitate a higher overall throughput in memory management operations.
