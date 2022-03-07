// print-buffer.c

#include <stdio.h>

#include "print-buffer.h"

void printUsefulBufC(UsefulBufC *usefulBufC, char *string) {
    printf("%s:\n", string);
    for (size_t i = 0; i < usefulBufC->len; i++) {
        printf("%02x", ((uint8_t*)usefulBufC->ptr)[i]);
    }
    printf("\n\n");
}
