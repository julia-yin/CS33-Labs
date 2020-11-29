/*
 *  * Implementation of malloc with an explicit free list aka a doubly linked list of free blocks
 *   * using first-fit search. My implementation is adapted from the textbook's implementation of
 *    * an implicit list with the majority of the macros and helper functions referenced from the
 *     * textbook. I altered it so that an explicit free list is used instead for better performance.
 *      *
 *       * Each allocated block consists of a 4-byte header and footer that store the block's size and
 *        * allocation status, along with the payload. Each free block consists of a 4-byte header and
 *         * footer storing the size and allocation status, along with 4-byte next and previous pointers
 *          * that help form the explicit free list of free blocks. Thus, the minimum block size is 16 blocks,
 *           * as each free block must be at least 16 bytes to store the header, footer, and pointers.
 *            *
 *             * My implementation uses a pointer to the start of the free block list to help traverse.
 *              * Each free block is connected to the next and previous blocks on this list via 4-byte
 *               * pointers stored in the payload area of each free block. When a block that "fits" an allocation
 *                * request is found, it is then removed from the explicit free list by updating the next and
 *                 * previous pointers of the blocks directly next to it. When a block is added to the free list,
 *                  * it is added to the very beginning of the list, and the start pointer is updated accordingly.
 *                   */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 *  *  *  * NOTE TO STUDENTS: Before you do anything else, please
 *   *   *   * provide your team information in the following struct.
 *    *    *    ********************************************************/
team_t team = {
    /* Team name */
    "ID: 005311394",
    /* First member's full name */
    "Julia Yin",
    /* First member's email address */
    "juliayin@ucla.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* Constants and Macros */
#define WSIZE          4       /* Word and header/footer size (bytes) */
#define DSIZE          2*WSIZE /* Double word size (bytes) */
#define CHUNKSIZE      (1<<12) /* Extend heap by this amount (bytes) */
#define MIN_BLOCK_SIZE 4*WSIZE /* Minimum block size: header + footer + 2 pointers */

#define MAX(x, y)     ((x) > (y)? (x) : (y))    /* Returns the maximum value between two inputs */
#define ROUNDUP(s, a) ((((s)+((a)-1))/(a))*(a)) /* Round size s up to a-byte alignment */

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc)  ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)       (*(unsigned int *)(p))
#define PUT(p, val)  (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)       ((char *)(bp) - WSIZE)
#define FTRP(bp)       ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp)  ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp)  ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* Given block ptr bp, get the next and previous block pointers (next stored first, then prev)*/
#define GET_NEXT(bp)   (*(char **)(bp))
#define GET_PREV(bp)   (*(char **)(bp + WSIZE))

/* Given block ptr bp, set the next and previous block pointers to pointer np */
#define SET_NEXT(bp, np)   (GET_NEXT(bp) = np)
#define SET_PREV(bp, np)   (GET_PREV(bp) = np)

/* Global variables */
static char *heap_listp = 0;       /* Pointer to first block in heap */
static char *free_list_startp = 0; /* Pointer to beginning of free list */

/* Function prototypes for internal helper routines */
static void *extend_heap(size_t words);
static void place(void *bp, size_t asize, char coal);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);
static int mm_check();

/* Function prototypes for explicit free list */
static void insert_into_free_list(void *bp); /* inserts block bp into the free list */
static void remove_from_free_list(void *bp); /* removes the block bp from the free list */


/*
 *  * Function: mm_init
 *   * Checks: returns NULL if initializing or extending the heap gives an error.
 *    * 1. Initialize the memory manager by extending the heap by 16 bytes for the
 *     *    4-byte alignment padding, 8-byte prologue and 4-byte epilogue
 *      * 2. Set the heap_listp pointer to point to the beginning of the heap, directly
 *       *    after the prologue header.
 *        * 3. Extend the heap by CHUNKSIZE (4k bytes).
 *         */
int mm_init(void)
{
    free_list_startp = NULL;

    /* Create the initial empty heap */
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
        return -1;
    PUT(heap_listp, 0);                          /* Alignment padding */
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1)); /* Prologue header */
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1)); /* Prologue footer */
    PUT(heap_listp + (3*WSIZE), PACK(0, 1));     /* Epilogue header */
    heap_listp += (2*WSIZE);                     /* Points directly after header of prologue */

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL)
        return -1;
    return 0;
}

/*
 *  * Function: mm_malloc
 *   * Checks: returns NULL if the size = 0, or if extending the heap by the
 *    *         extend size gives an error.
 *     * 1. Takes a parameter of size and adjusts it to include the overhead and
 *      *    alignment requirements (multiple of 8 bytes).
 *       * 2. If the free list is nonempty, search the free list for a block that is
 *        *    sufficiently large and place the new allocated size into the free block.
 *         * 3. If no free block is found, extend the heap by extendsize and place the
 *          *    new block into the extended region.
 *           * 4. Return a pointer to the start of the newly allcoated block.
 *            */
void *mm_malloc(size_t size)
{
    size_t asize;      /* Adjusted block size */
    size_t extendsize; /* Amount to extend heap if no fit */
    char *bp;
    
    /* If size parameter is 0, return immediately */
    if (size == 0)
        return NULL;

    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= DSIZE)
        asize = 2*DSIZE;
    else
        asize = ROUNDUP(size + DSIZE, DSIZE);

    /* Search the free list for a fit, place if fit is found */
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize, 0);
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize,CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;
    place(bp, asize, 1);
    return bp;
}

/*
 *  * Function: m_free
 *   * Checks: return 0 if the block bp has allocation bit set to 0.
 *    * 1. Free the block by changing the allocation bit in the header
 *     *    and footer to 0.
 *      * 2. Call coalesce, which will either add the newly freed block to
 *       *    the free list or coalesce with nearby blocks.
 *        */
void mm_free(void *bp)
{
    if (GET_ALLOC(HDRP(bp)) == 0)
        return;

    size_t size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp); /* coalesce will add the newly freed block to the linked list */
}

/*
 *  * Function: coalesce
 *   * Checks: four different cases
 *    * 1. If next and prev blocks are both allocated, insert bp into the free list
 *     *    and return a pointer to bp.
 *      * 2. If next is free, remove the next block from the free list and update bp's
 *       *    header and next's footer with the new size. Add bp to the free list and return
 *        *    a pointer to bp.
 *         * 3. If prev is free, update the header of the previous block to the new size.
 *          *    Update the footer of bp. Point bp to the previous block and return a pointer to bp.
 *           * 4. If both next and prev are free, remove next from the free list and update prev's
 *            *    header and next's footer with the new size. Point bp to prev and return a pointer to bp.
 *             */
static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    /* Case 1: next and prev are allocated */
    if (prev_alloc && next_alloc) {
        insert_into_free_list(bp);
        return bp;
    }

    /* Case 2: next is free, remove next from free list */
    else if (prev_alloc && !next_alloc) {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        remove_from_free_list(NEXT_BLKP(bp));
        PUT(HDRP(bp), PACK(size, 0)); /* Header of current block updated with new size */
        PUT(FTRP(bp), PACK(size,0));  /* Footer of current (coalesced) block updated with new size */
        insert_into_free_list(bp);
    }

    /* Case 3: prev is free, update the header of prev to the new size */
    else if (!prev_alloc && next_alloc) {
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));            /* Footer of current block updated with new size */
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0)); /* Header of previous block updated with new size */
        bp = PREV_BLKP(bp);
    }

    /* Case 4: both are free, remove next and add bp once moved to point to prev free block */
    else {
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        remove_from_free_list(NEXT_BLKP(bp));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0)); /* Header of previous block updated with new size */
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0)); /* Footer of next block updated with new size */
        bp = PREV_BLKP(bp);
    }
   
    return bp; /* bp points to the very beginning of the coalesced block (prev for cases 3-4) */
}

/*
 *  * Function: mm_realloc
 *   * Checks: If size = 0, free the block bp. If bp = NULL, allocate a block of the
 *    *         right size.
 *     * 1. Allocate a new block of desired new size, and if it fails, return NULL.
 *      * 2. Copy over the data from the old block to the new block. If the new size is
 *       *    smaller, copy in as much as will fit.
 *        * 3. Free the old block. Return a pointer to the new block of new size.
 *         */
void *mm_realloc(void *bp, size_t size)
{
    size_t oldsize;
    void *newbp;

    /* If size == 0 then this is just free, and we return NULL. */
    if (size == 0) {
        mm_free(bp);
        return 0;
    }

    /* If oldptr is NULL, then this is just malloc. */
    if (bp == NULL) {
        return mm_malloc(size);
    }

    /* newptr will be nonzero if malloc succeeds, 0 if it fails (there isn't a sufficiently
 *      * large free block to allocate)
 *           */
    newbp = mm_malloc(size);

    /* If realloc() fails the original block is left untouched  */
    if (!newbp) {
        return NULL;
    }

    /* Copy the old data. */
    oldsize = GET_SIZE(HDRP(bp)) - DSIZE;
    if (size < oldsize)
        oldsize = size;
    memcpy(newbp, bp, oldsize);

    /* Free the old block. */
    mm_free(bp);

    return newbp;
}

/*
 *  * Function: mm_check
 *   * Description: Checks for the below errors. Returns -1 if any error is present after
 *    *              checking through all the possible errors.
 *     * 1. Check that the prologue header/footer stores a size of 16 bytes and marked as allocated.
 *      * 2. Check that each block's pointer is aligned to 8 bytes (last 3 bits are 0)
 *       * 3. Check that each block is a multiple of 8 bytes and is at least 16 bytes in size to
 *        *    maintain alignment.
 *         * 4. Check that each block's header matches its footer.
 *          * 5. Check that all free blocks are on the free list, and all allocated blocks aren't on the
 *           *    free list.
 *            * 6. Check that all free blocks do not have a free block right in front of it (prev != free)
 *             *     A) Uses a local variable prev_free that stores a 1 if the previous block was free. Checks
 *              *        against the current block's allocation bit
 *               * 7. Check that the epilogue is of size 0 and marked as allocated.
 *                *
 *                 * Checkheap can be called before and after functions mm_malloc, mm_realloc, and mm_init for the
 *                  * most accurate results. Calling checkheap within functions place, coalesce, extend_heap, or
 *                   * find_fit may result in false errors as those functions are necessary to return the heap back to
 *                    * a fully-functioning state. The print messages are designed to narrow down possible errors and
 *                     * give detailed information about which block contained the error.
 *                      */
static int mm_check()
{
    char *bp = 0;
    char *fbp = 0;
    char alloc;
    char prev_free = 0;
    int error = 0;

    /* Check that the prologue is 16 bytes and allocated */
    if ((GET_SIZE(HDRP(heap_listp)) != DSIZE) || !GET_ALLOC(HDRP(heap_listp))) {
        printf("Bad prologue header\n");
        error = -1;
    }

    /* Check every block in the heap */
    for (bp = heap_listp + DSIZE; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        alloc = GET_ALLOC(HDRP(bp));
        
        /* Check that each pointer is aligned to 8 bytes */
        if (((long)bp & 0x7) != 0) {
            printf("Block %p is not a multiple of 8\n", bp);
            error = -1;
        }
        
        /* Check that each block is a multiple of 8 bytes and is at least 16 bytes in size */
        if (((GET_SIZE(HDRP(bp))%8) != 0) || (GET_SIZE(HDRP(bp)) < MIN_BLOCK_SIZE)) {
            printf("Block %p size %d is not a multiple of 8 or less than 16 \n", bp, GET_SIZE(HDRP(bp)));
            error = -1;
        }
        
        /* Check that the header matches the footer */
        if (GET(HDRP(bp)) != GET(FTRP(bp))) {
            printf("Error: header of block %p does not match footer\n", bp);
            error = -1;
        }
        
        
        /* Check that all allocated blocks aren't in free list, and all free blocks are in free list */
        for (fbp = free_list_startp; fbp != NULL; fbp = GET_NEXT(fbp)) {
            if (bp == fbp) {
                break;
            }
        }
        
        /* Error occurs if an allocated block has a match on the free list, or the free
 *          * block isn't on the free list
 *                   */
        if ((alloc == 0 && fbp == NULL) || (alloc == 1 && fbp != NULL)) {
            prev_free = 0;
            printf("Block %p allocate in error: %d \n", bp, alloc);
            error = -1;
        }
        
        /* If a free block's previous block is also free, coalescing error */
        else if (alloc == 0) {
            if (prev_free == 1) {
                printf("Block %p must be coalesced with previous\n", fbp);
                error = -1;
            }
            prev_free = 1;
        }
        else
            prev_free = 0;
    }
    
    /* Check that the epilogue is size 0 and set as allocated */
    if ((GET_SIZE(HDRP(bp)) != 0) || !(GET_ALLOC(HDRP(bp)))) {
        printf("Bad epilogue header\n");
        error = -1;
    }
    
    return error;
}


/* Helper Functions */

/*
 *  * Function: extend_heap
 *   * 1. Adjust size (in words) to an even number to help maintain alignment
 *    * 2. Call mem_sbrk (library function) to extend the heap by size bytes. If this
 *     *    fails, return NULL.
 *      * 3. Set the new free block's header and footer with the new size. The header is the
 *       *    epilogue of the old heap.
 *        * 4. Set the new epilogue header with size 0 and allocation bit 1.
 *         * 5. Coalesce the new block if the previous block was free.
 *          * 6. Return a pointer to the new block.
 *           */
static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));         /* Free block header */
    PUT(FTRP(bp), PACK(size, 0));         /* Free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* New epilogue header */
    
    /* Coalesce if the previous block was free */
    bp = coalesce(bp);
    
    return bp;
}

/*
 *  * Function: place
 *   * Checks: two cases (split and no split)
 *    * 1. If the difference between the desired size and free block size is greater than minimum
 *     *    block size (16 bytes), split the block.
 *      *     A) Update the header and new footer of the free block with the desired size.
 *       *     B) Remove the current block from the free list. Move the block pointer to point to the next
 *        *        block, aka the remainder block.
 *         *     C) Update the header and footer of the remainder block to the remainder size. If place was
 *          *        called after extend_heap (coal = 1), call coalesce. If place was called after find_fit (coal = 0),
 *           *        simply insert the block into the free list.
 *            * 2. Otherwise, allocate the block by updating the header and footer and remove it from the free list.
 *             */
static void place(void *bp, size_t asize, char coal)
{
    size_t csize = GET_SIZE(HDRP(bp));

    /* if remainder is greater than or equal to minimum block size, split the free block into
 *      * the allocated portion and the free portion
 *           */
    if ((csize - asize) >= (2*DSIZE)) {
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        remove_from_free_list(bp);      /* remove allocated block from free list */
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize-asize, 0));
        PUT(FTRP(bp), PACK(csize-asize, 0));
        
        if (coal == 1) /* only called when place is used after extend heap */
            coalesce(bp);
        else {
            insert_into_free_list(bp);
        }
    } else {
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
        remove_from_free_list(bp);      /* remove allocated block from free list */
    }
}

/*
 *  * Function: find_fit
 *   * 1. Using first-fit search, traverse the free list to find a block that is asize or larger.
 *    * 2. If a block is found, return the pointer to that block. If no block is found, return NULL.
 *     */
static void *find_fit(size_t asize)
{
    /* First-fit search */
    void *bp;
    
    for (bp = free_list_startp; bp != NULL; bp = GET_NEXT(bp)) {
        if (asize <= GET_SIZE(HDRP(bp))) {
            return bp;
        }
    }
    return NULL; /* No fit */
}

/*
 *  * Function: insert_into_free_list
 *   * Description: Inserts a newly freed block bp into the beginning of the free list.
 *    * 1. Set the next and previous pointers of the current block bp to point to the current free
 *     *    list start and NULL respectively.
 *      * 2. If the current free_list_startp is NULL, the free list is currently empty. Update the start
 *       *    pointer to point to bp and return.
 *        * 3. Otherwise, update the current start pointer's prev to point to block bp. Update the start
 *         *    pointer to point to bp as the new start of the free list.
 *          */
static void insert_into_free_list(void *bp)
{
    SET_NEXT(bp, free_list_startp); /* bp comes before current start block */
    SET_PREV(bp, NULL);             /* bp has no previous block (at top) */
    
    if (free_list_startp == NULL) { /* if list initially empty, */
        free_list_startp = bp;
        return;
    }
    
    /* list not empty, startp is a valid pointer */
    SET_PREV(free_list_startp, bp); /* current start block comes after bp */
    free_list_startp = bp;          /* point start pointer to bp which is new start */
}

/*
 *  * Function: remove_from_free_list
 *   * Checks: three cases (first block, last block, middle block)
 *    * 1. If bp is the first block in the list, set the start pointer to point to the next block.
 *     *    If bp is the only block on the list, set the previous pointer of the start block to NULL.
 *      * 2. If bp is the last block in the list, set the next pointer of the previous block to NULL.
 *       * 3. If bp is neither the first nor last block, update the next pointer of the previous block to
 *        *    point to bp's next. Update the prev pointer of the next block to bp's prev.
 *        */
static void remove_from_free_list(void *bp)
{
    /* Case 1: bp is the first block in the list */
    if (!GET_PREV(bp)) {
        free_list_startp = GET_NEXT(bp);
        if (free_list_startp != NULL)
            SET_PREV(free_list_startp, NULL);
    }
    /* Case 2: bp is the last block in the list */
    else if (GET_PREV(bp) && !GET_NEXT(bp)) {
        SET_NEXT(GET_PREV(bp), NULL);
    }
    /* Case 3: bp is somewhere in the list */
    else {
        /* set the next pointer of the prev block to next of current block (essentially
 *          * removing current block from list)
 *                   */
        SET_NEXT(GET_PREV(bp), GET_NEXT(bp));
        /* set prev pointer of next block to current prev (NULL if current bp is at front of
 *          * list)
 *                   */
        SET_PREV(GET_NEXT(bp), GET_PREV(bp));
    }
}


