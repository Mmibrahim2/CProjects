////////////////////////////////////////////////////////////////////////////////
// Main File:        (p3Heap.c)
// This File:        (p3Heap.c)
// Other Files:      (p3Heap.h)
// Semester:         CS 354 Lecture 01 Spring 2023
// Instructor:       deppeler
// 
// Author:           (Mohammud Ibrahim)
// Email:            (Mmibrahim2@wisc.edu)
// CS Login:         (mohammud)
//
/////////////////////////// OTHER SOURCES OF HELP //////////////////////////////
//                   fully acknowledge and credit all sources of help,
//                   other than Instructors and TAs.
//
// Persons:          Identify persons by name, relationship to you, and email.
//                   Describe in detail the the ideas and help they provided.
//
// Online sources:   avoid web searches to solve your problems, but if you do
//                   search, be sure to include Web URLs and description of 
//                   of any information you find.
//////////////////////////// 80 columns wide ///////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
// Copyright 2020-2023 Deb Deppeler based on work by Jim Skrentny
// Posting or sharing this file is prohibited, including any changes/additions.
// Used by permission SPRING 2023, CS354-deppeler
//
///////////////////////////////////////////////////////////////////////////////
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include "p3Heap.h"
 
/*
 * This structure serves as the header for each allocated and free block.
 * It also serves as the footer for each free block but only containing size.
 */
typedef struct blockHeader {           

    int size_status;

    /*
     * Size of the block is always a multiple of 8.
     * Size is stored in all block headers and in free block footers.
     *
     * Status is stored only in headers using the two least significant bits.
     *   Bit0 => least significant bit, last bit
     *   Bit0 == 0 => free block
     *   Bit0 == 1 => allocated block
     *
     *   Bit1 => second last bit 
     *   Bit1 == 0 => previous block is free
     *   Bit1 == 1 => previous block is allocated
     * 
     * Start Heap: 
     *  The blockHeader for the first block of the heap is after skip 4 bytes.
     *  This ensures alignment requirements can be met.
     * 
     * End Mark: 
     *  The end of the available memory is indicated using a size_status of 1.
     * 
     * Examples:
     * 
     * 1. Allocated block of size 24 bytes:
     *    Allocated Block Header:
     *      If the previous block is free      p-bit=0 size_status would be 25
     *      If the previous block is allocated p-bit=1 size_status would be 27
     * 
     * 2. Free block of size 24 bytes:
     *    Free Block Header:
     *      If the previous block is free      p-bit=0 size_status would be 24
     *      If the previous block is allocated p-bit=1 size_status would be 26
     *    Free Block Footer:
     *      size_status should be 24
     */
} blockHeader;         

/* Global variable - DO NOT CHANGE NAME or TYPE. 
 * It must point to the first block in the heap and is set by init_heap()
 * i.e., the block at the lowest address.
 */
blockHeader *heap_start = NULL;     

/* Size of heap allocation padded to round to nearest page size.
 */
int alloc_size;

/*
 * Additional global variables may be added as needed below
 * TODO: add global variables needed by your function
 */

 
/* 
 * Function for allocating 'size' bytes of heap memory.
 * Argument size: requested size for the payload
 * Returns address of allocated block (payload) on success.
 * Returns NULL on failure.
 *
 * This function must:
 * - Check size - Return NULL if size < 1 
 * - Determine block size rounding up to a multiple of 8 
 *   and possibly adding padding as a result.
 *
 * - Use BEST-FIT PLACEMENT POLICY to chose a free block
 *
 * - If the BEST-FIT block that is found is exact size match
 *   - 1. Update all heap blocks as needed for any affected blocks
 *   - 2. Return the address of the allocated block payload
 *
 * - If the BEST-FIT block that is found is large enough to split 
 *   - 1. SPLIT the free block into two valid heap blocks:
 *         1. an allocated block
 *         2. a free block
 *         NOTE: both blocks must meet heap block requirements 
 *       - Update all heap block header(s) and footer(s) 
 *              as needed for any affected blocks.
 *   - 2. Return the address of the allocated block payload
 *
 *   Return if NULL unable to find and allocate block for required size
 *
 * Note: payload address that is returned is NOT the address of the
 *       block header.  It is the address of the start of the 
 *       available memory for the requesterr.
 *
 * Tips: Be careful with pointer arithmetic and scale factors.
 */
void* balloc(int size) {     
    //TODO: Your code goes in here.
	//check size
    if (size < 1 || size > alloc_size) {
        return NULL;
    }
    
    // Add header size
    size = size + sizeof(blockHeader);
    
    // Align the req size to 8 byte
    while (size % 8 != 0) {
        ++size;
    }
    
    // Check if heap is initialized
    if (heap_start == NULL) {
        return NULL;
    }
    
    // Search for free block of exact size or larger block that can split
    blockHeader *curr = heap_start;
    void* payload = NULL;
    void* largerPayload = NULL;
    int largerPayloadSize = 0;
    // loop till end of heap
    while (curr->size_status != 1) {
        int block_size = curr -> size_status;
        // Round down to multiple of 4
        block_size = block_size - (block_size & 3);
        // check if a block is free
        if ((curr -> size_status & 1) != 1) {
        // exact match
        if (block_size == size) {
            payload = (void*)(curr) + sizeof(blockHeader);
            break;
        }
        // block larger than req size
        if (block_size > size) {
            if (largerPayloadSize > block_size || largerPayloadSize == 0) {
                largerPayload = (void*)(curr) + sizeof(blockHeader);
                largerPayloadSize = block_size;
            }
        }

        }

        // Move to the next block
        curr = (blockHeader*)(((void*)curr) + block_size);
    }

    // If exact found is found
    if(payload) {
        // Update header of current a-bit to 1
        blockHeader* payloadHeader = (blockHeader*)(payload - sizeof(blockHeader));
        payloadHeader->size_status += 1;

        // Get size of payload block
        int payloadBlockSize = payloadHeader->size_status -1;
		
		// check if odd
		if (payloadBlockSize & 2) {
			payloadBlockSize -= 2;
		}
		
		//Get to next block
		blockHeader* nextBlock = (blockHeader*)(((void*)payloadHeader) + payloadBlockSize);
		
		// end?
		if (nextBlock -> size_status != 1) {
			nextBlock -> size_status += 2;
		}
			return payload;
		}
	else if (largerPayload != NULL) {

			blockHeader* payloadHeader = (blockHeader*)(largerPayload - sizeof(blockHeader));
            // set a-bit to 1               
            payloadHeader->size_status += 1;
			int prebits = (payloadHeader->size_status&3);
			int payloadSize = payloadHeader->size_status -1;
        
        // check if odd, make even   
        if (payloadSize & 2) {
            payloadSize -= 2;
        }
        // Size
        payloadHeader->size_status = size + prebits;
        //Switch to next 
        blockHeader* nextBlock = (blockHeader*)(((void*)payloadHeader) + size);

        //creating new header 
        nextBlock -> size_status = payloadSize - size + 2;
        blockHeader* footer = (blockHeader*)(((void*)nextBlock)  - sizeof(blockHeader));
        
        // setting footer
        footer -> size_status = payloadSize - size;
        return largerPayload;
    }
	
    return NULL;
} 
 
/* 
 * Function for freeing up a previously allocated block.
 * Argument ptr: address of the block to be freed up.
 * Returns 0 on success.
 * Returns -1 on failure.
 * This function should:
 * - Return -1 if ptr is NULL.
 * - Return -1 if ptr is not a multiple of 8.
 * - Return -1 if ptr is outside of the heap space.
 * - Return -1 if ptr block is already freed.
 * - Update header(s) and footer as needed.
 */                    
int bfree(void *ptr) {    
    //TODO: Your code goes in here.
	// check if null
	if (ptr == NULL) {
        return -1;
	}
	// check if multiple of 8
    if ((unsigned int)ptr % 8 != 0) {
        return -1;
    }
	// check if outside the heap space
    if (ptr < (void *)heap_start || ptr > ((void *)heap_start + alloc_size)) {
        return -1;
    }
	// Get block
    blockHeader *block_ptr = (blockHeader *)ptr - 1;
	// check if free
    if ((block_ptr->size_status & 1) == 0) {
        return -1;
    }
	// a-bit = 0
    block_ptr->size_status -= 1;
    int block_size = block_ptr->size_status & (~3);
    blockHeader* next_block_ptr = (blockHeader*)((void*)block_ptr + block_size);
    next_block_ptr -> size_status -= 2;
    return 0;
}
 

/*
 * Function for traversing heap block list and coalescing all adjacent 
 * free blocks.
 *
 * This function is used for user-called coalescing.
 * Updated header size_status and footer size_status as needed.
 */
int coalesce() {
    //TODO: Your code goes in here.
	blockHeader* current_block = heap_start; 
    int num_coalesced = 0; 
	// loop through heap
    while (current_block->size_status != 1) {
		// Get size
        int current_block_size = current_block->size_status & -4; 
        blockHeader* next_block = (blockHeader*) (((void*)current_block) + current_block_size);
		// check if both current/next blocks are free 
        if (!(next_block->size_status & 1) && !(current_block->size_status & 1)) {
			// add size of next block
            current_block->size_status += next_block->size_status & -4; 
            current_block_size = current_block->size_status & -4; // Update
            num_coalesced++; 
        }
        else { //If the next block
            current_block = next_block; // Move to the next block
        }
    }
	//DIDNT SPECIFY
    return num_coalesced; // Return the number of coalesced blocks or 0! 
}	

 
/* 
 * Function used to initialize the memory allocator.
 * Intended to be called ONLY once by a program.
 * Argument sizeOfRegion: the size of the heap space to be allocated.
 * Returns 0 on success.
 * Returns -1 on failure.
 */                    
int init_heap(int sizeOfRegion) {    
 
    static int allocated_once = 0; //prevent multiple myInit calls
 
    int   pagesize; // page size
    int   padsize;  // size of padding when heap size not a multiple of page size
    void* mmap_ptr; // pointer to memory mapped area
    int   fd;

    blockHeader* end_mark;
  
    if (0 != allocated_once) {
        fprintf(stderr, 
        "Error:mem.c: InitHeap has allocated space during a previous call\n");
        return -1;
    }

    if (sizeOfRegion <= 0) {
        fprintf(stderr, "Error:mem.c: Requested block size is not positive\n");
        return -1;
    }

    // Get the pagesize from O.S. 
    pagesize = getpagesize();

    // Calculate padsize as the padding required to round up sizeOfRegion 
    // to a multiple of pagesize
    padsize = sizeOfRegion % pagesize;
    padsize = (pagesize - padsize) % pagesize;

    alloc_size = sizeOfRegion + padsize;

    // Using mmap to allocate memory
    fd = open("/dev/zero", O_RDWR);
    if (-1 == fd) {
        fprintf(stderr, "Error:mem.c: Cannot open /dev/zero\n");
        return -1;
    }
    mmap_ptr = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (MAP_FAILED == mmap_ptr) {
        fprintf(stderr, "Error:mem.c: mmap cannot allocate space\n");
        allocated_once = 0;
        return -1;
    }
  
    allocated_once = 1;

    // for double word alignment and end mark
    alloc_size -= 8;

    // Initially there is only one big free block in the heap.
    // Skip first 4 bytes for double word alignment requirement.
    heap_start = (blockHeader*) mmap_ptr + 1;

    // Set the end mark
    end_mark = (blockHeader*)((void*)heap_start + alloc_size);
    end_mark->size_status = 1;

    // Set size in header
    heap_start->size_status = alloc_size;

    // Set p-bit as allocated in header
    // note a-bit left at 0 for free
    heap_start->size_status += 2;

    // Set the footer
    blockHeader *footer = (blockHeader*) ((void*)heap_start + alloc_size - 4);
    footer->size_status = alloc_size;
  
    return 0;
} 
                  
/* 
 * Function can be used for DEBUGGING to help you visualize your heap structure.
 * Traverses heap blocks and prints info about each block found.
 * 
 * Prints out a list of all the blocks including this information:
 * No.      : serial number of the block 
 * Status   : free/used (allocated)
 * Prev     : status of previous block free/used (allocated)
 * t_Begin  : address of the first byte in the block (where the header starts) 
 * t_End    : address of the last byte in the block 
 * t_Size   : size of the block as stored in the block header
 */                     
void disp_heap() {     
 
    int    counter;
    char   status[6];
    char   p_status[6];
    char * t_begin = NULL;
    char * t_end   = NULL;
    int    t_size;

    blockHeader *current = heap_start;
    counter = 1;

    int used_size =  0;
    int free_size =  0;
    int is_used   = -1;

    fprintf(stdout, 
	"*********************************** HEAP: Block List ****************************\n");
    fprintf(stdout, "No.\tStatus\tPrev\tt_Begin\t\tt_End\t\tt_Size\n");
    fprintf(stdout, 
	"---------------------------------------------------------------------------------\n");
  
    while (current->size_status != 1) {
        t_begin = (char*)current;
        t_size = current->size_status;
    
        if (t_size & 1) {
            // LSB = 1 => used block
            strcpy(status, "alloc");
            is_used = 1;
            t_size = t_size - 1;
        } else {
            strcpy(status, "FREE ");
            is_used = 0;
        }

        if (t_size & 2) {
            strcpy(p_status, "alloc");
            t_size = t_size - 2;
        } else {
            strcpy(p_status, "FREE ");
        }

        if (is_used) 
            used_size += t_size;
        else 
            free_size += t_size;

        t_end = t_begin + t_size - 1;
    
        fprintf(stdout, "%d\t%s\t%s\t0x%08lx\t0x%08lx\t%4i\n", counter, status, 
        p_status, (unsigned long int)t_begin, (unsigned long int)t_end, t_size);
    
        current = (blockHeader*)((char*)current + t_size);
        counter = counter + 1;
    }

    fprintf(stdout, 
	"---------------------------------------------------------------------------------\n");
    fprintf(stdout, 
	"*********************************************************************************\n");
    fprintf(stdout, "Total used size = %4d\n", used_size);
    fprintf(stdout, "Total free size = %4d\n", free_size);
    fprintf(stdout, "Total size      = %4d\n", used_size + free_size);
    fprintf(stdout, 
	"*********************************************************************************\n");
    fflush(stdout);

    return;  
} 


// end p3Heap.c (Spring 2023)                                         


