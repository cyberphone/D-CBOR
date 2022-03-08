// print-buffer.c

#include <stdio.h>

#include "print-buffer.h"

void printUsefulBufC(UsefulBufC *usefulBufC, char *string) {
    int length = (int)usefulBufC->len;
    printf("%s [%d]:\n", string, length);
    for (int i = 0; i < length; i++) {
        printf("%02x", ((uint8_t*)usefulBufC->ptr)[i]);
    }
    printf("\n\n");
}
