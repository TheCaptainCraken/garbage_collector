#include <stdio.h>
#include "library/garbage_collector.h"

int main() {
    memoryMaster* master = createMemoryMaster();

    int** pointer = cralloc(master, sizeof(int*));
    int* pointer2 = (int*)cralloc(master, sizeof(int) * 10);

    *pointer = pointer2;

    collect(master);
    printMaster(master);
    return 0;
}