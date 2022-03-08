// print-buffer.c

#include "d-cbor.h"

// For demonstration purposes only...

#include <stdio.h>

void printCborBuffer(CBOR_BUFFER *cborBuffer, char* string) {
    if (cborBuffer->length) {
        printf("%s [%d]:\n", string, cborBuffer->pos);
        for (int i = 0; i < cborBuffer->pos; i++) {
            printf("%02x", (int)cborBuffer->data[i]);
        }
    } else {
        printf("BUFFER OVERFLOW!");
    }
    printf("\n\n");
}