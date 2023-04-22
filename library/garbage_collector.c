#include "garbage_collector.h"

/*
    Custom memory allocator.
    This function works similar to malloc() from stdlib.h
    It takes two arguments:
        - master: this is an istance of a memoryMaster object used to store relevant information about heap memory.
        - size: the size of the block the users desires.
    It returns a void pointer to the first address of the memory created. If no memory is available it will return a NULL pointer.
*/
void* cralloc(memoryMaster* master, size_t size) {
    blockHeader* freeListItem = master->free_blocks;
    if (freeListItem != NULL) {
        // If there is a used block that is now free to use and it is of the right size then use that.
        for (freeListItem;freeListItem != NULL && freeListItem->max_size < size; freeListItem = freeListItem->next);
        if (freeListItem != NULL) {
            freeListItem->used_size = size;
            addBlockToOccupiedList(master, freeListItem);
            removeBlockFromFreeList(master, freeListItem);
            return ((void*)freeListItem) + HEADER_SIZE;
        }
    }
    // If there isn't a free block to use then create a new one.
    blockHeader* newBlock = createNewBlock(size);
    if (newBlock != NULL) {
        addBlockToOccupiedList(master, newBlock);
        return (void*)newBlock + HEADER_SIZE;
    } else {
        return NULL;
    }
}


/*
    Creates a new block of memory in the heap using sbrk().
    Blocks of memory created by this function look like this:
    ┌────────────┐
    │   Header   │
    ├────────────┤
    │ Data Block │
    └────────────┘
    It takes one parameter:
        - size: the size of the data the user needs to store. (no need to account for the header)
    It will return a pointer to the new block or a NULL pointer if there is no memory available.
*/
static blockHeader* createNewBlock(size_t size) {
    size_t newBlockSize;
    if (size < MIN_SIZE_BLOCK) {
        newBlockSize = MIN_SIZE_BLOCK;
    } else if (size % MIN_SIZE_BLOCK == 0) {
        newBlockSize = size;
    } else {
        newBlockSize = size + MIN_SIZE_BLOCK;
    }

    blockHeader* newBlock = (blockHeader*)sbrk(newBlockSize + HEADER_SIZE);
    if (newBlock != -1) {
        newBlock->next = NULL;
        newBlock->max_size = newBlockSize;
        newBlock->used_size = size;
    }
    return newBlock;
}

static void addBlockToFreeList(memoryMaster* master, blockHeader* block) {
    block->next = NULL;
    block->used_size = 0;
    blockHeader* lastBlockInTheList = master->free_blocks;
    if (lastBlockInTheList != NULL) {
        for (lastBlockInTheList; lastBlockInTheList->next != NULL; lastBlockInTheList = lastBlockInTheList->next);
        lastBlockInTheList->next = block;
    } else {
        master->free_blocks = block;
    }
}

static void addBlockToOccupiedList(memoryMaster* master, blockHeader* block) {
    block->next = NULL;
    blockHeader* lastBlockInTheList = master->occupied_blocks;
    if (lastBlockInTheList != NULL) {
        for (lastBlockInTheList; lastBlockInTheList->next != NULL; lastBlockInTheList = lastBlockInTheList->next);
        lastBlockInTheList->next = block;
    } else {
        master->occupied_blocks = block;
    }
}

static void removeBlockFromFreeList(memoryMaster* master, blockHeader* block) {
    blockHeader* freeListItem = master->free_blocks;
    if (freeListItem != NULL) {
        if (freeListItem != block) {
            for (freeListItem; freeListItem->next != block && freeListItem != NULL; freeListItem = freeListItem->next);
            if (freeListItem != NULL) {
                freeListItem->next = freeListItem->next->next;
            } else {
                return;
            }
        } else {
            master->free_blocks = master->free_blocks->next;
        }
    } else {
        return;
    }
}

static void removeBlockFromOccupiedList(memoryMaster* master, blockHeader* block) {
    blockHeader* occupiedListItem = master->occupied_blocks;
    if (occupiedListItem != NULL) {
        if (occupiedListItem != block) {
            for (occupiedListItem; occupiedListItem->next != block && occupiedListItem != NULL; occupiedListItem = occupiedListItem->next);
            if (occupiedListItem != NULL) {
                occupiedListItem->next = occupiedListItem->next->next;
            } else {
                return;
            }
        } else {
            master->occupied_blocks = master->occupied_blocks->next;
        }
    } else {
        return;
    }
}

memoryMaster* createMemoryMaster() {
    size_t size = sizeof(memoryMaster);
    memoryMaster* master = (memoryMaster*)sbrk(size);
    if ((long int)master != -1) {
        master->free_blocks = NULL;
        master->occupied_blocks = NULL;
        return master;
    } else {
        return NULL;
    }
}

void unalive(memoryMaster* master, void* pointer) {
    blockHeader* block = (blockHeader*)(pointer - HEADER_SIZE);
    removeBlockFromOccupiedList(master, block);
    addBlockToFreeList(master, block);
}

static void printList(blockHeader* head) {
    int counter = 0;
    for (head; head != NULL; head = head->next) {
        printf("\tBlock #%d at address: %p\n", counter, (void*)head);
        printf("\t==> max size: %ld, used size: %ld\n", head->max_size, head->used_size);
        printf("\t==> starting address: %p\n\n", (void*)head + HEADER_SIZE);
        counter++;
    }
}

void printMaster(memoryMaster* master) {
    printf("FREE LIST:\n");
    printList(master->free_blocks);
    printf("OCCUPIED LIST:\n");
    printList(master->occupied_blocks);
}

/*
    Garbage collector
*/

static bool isAnOccupiedBlock(memoryMaster* master, blockHeader* block) {
    void* tmp = (void*)block - HEADER_SIZE;
    blockHeader* occupiedListItem = master->occupied_blocks;
    for (occupiedListItem; occupiedListItem != NULL; occupiedListItem = occupiedListItem->next) {
        if (occupiedListItem == (blockHeader*)tmp) {
            return true;
        }
    }
    return false;
}

static void scanAndMark(memoryMaster* master, void* start, void* end) {
    blockHeader** block = NULL;
    if (end - start >= 0x8) {
        for (start; start < end; start += 0x8) {
            block = (blockHeader**)start;
            if (isAnOccupiedBlock(master, *block) == true) {
                blockHeader* tmp = *block;
                tmp = (blockHeader*)((void*)tmp - HEADER_SIZE);
                tmp->to_be_unalived = false;
            }
        }
    }
}

static void summaryJudgment(memoryMaster* master) {
    blockHeader* occupiedListItem = master->occupied_blocks;
    for (occupiedListItem; occupiedListItem != NULL; occupiedListItem = occupiedListItem->next) {
        occupiedListItem->to_be_unalived = true;
    }
}

static size_t executionDay(memoryMaster* master) {
    blockHeader* occupiedListItem = master->occupied_blocks;
    blockHeader* tmp = NULL;
    size_t freedMemory = 0;
    while (occupiedListItem != NULL) {
        tmp = occupiedListItem;
        occupiedListItem = occupiedListItem->next;
        if (tmp->to_be_unalived == true) {
            freedMemory += tmp->used_size;
            tmp->to_be_unalived = false;
            unalive(master, (void*)tmp + HEADER_SIZE);
        }
    }
    return freedMemory;
}

static void scanHeap(memoryMaster* master) {
    blockHeader* occupiedListItem = master->occupied_blocks;
    for (occupiedListItem; occupiedListItem != NULL; occupiedListItem = occupiedListItem->next) {
        scanAndMark(master, (void*)occupiedListItem + HEADER_SIZE, (void*)occupiedListItem + HEADER_SIZE + occupiedListItem->used_size);
    }
}

size_t collect(memoryMaster* master) {
    summaryJudgment(master);
    scanHeap(master);
    //stuff
    return executionDay(master);
}