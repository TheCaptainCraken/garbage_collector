#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define HEADER_SIZE sizeof(blockHeader)
#define MIN_SIZE_BLOCK (size_t)sizeof(void*)
#define NOT_IMPLEMENTED                                                     \
    fprintf(                                                                \
        stderr,                                                             \
        "Function %s in file %s at line %d has not been implemented yet\n", \
        __func__,                                                           \
        __FILE__,                                                           \
        __LINE__                                                            \
    );                                                                      \
    abort();                                                                

/*
*   Basically each heap block is structured in this way:
*   ┌────────────┐
*   │   Header   │
*   ├────────────┤
*   │ Data Block │
*   └────────────┘
*   The data blocks have a minimum size of 32 bits.
*
*/

typedef struct blockHeader_s {
    size_t used_size;
    size_t max_size;
    bool to_be_unalived;
    struct blockHeader_s* next;
}blockHeader;

typedef struct memoryMaster_s {
    blockHeader* free_blocks;
    blockHeader* occupied_blocks;
}memoryMaster;

memoryMaster* createMemoryMaster();
void* cralloc(memoryMaster* master, size_t size);
void unalive(memoryMaster* master, void* pointer);
size_t collect(memoryMaster* master);
void printMaster(memoryMaster* master);