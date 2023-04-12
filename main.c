#include <stdio.h>
#include "library/garbage_collector.h"

int main() {
    printf("Hello World!\n");
    printf("Checking: whta is the value of 2 + 2?\nComputer: %d!\n", testSum(2, 2));
    return 0;
}