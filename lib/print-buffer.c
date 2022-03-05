// print-buffer.c

#include "d-cbor.h"

// For demonstration purposes only...

#include <stdio.h>

void printCborBuffer(CBOR_BUFFER *cborBuffer) {
    if (cborBuffer->length) {
        printf("length=%d\n", cborBuffer->pos);
        for (int i = 0; i < cborBuffer->pos; i++) {
            printf("%02x", (int)cborBuffer->data[i]);
        }
    } else {
        printf("Buffer overflow");
    }
    printf("\n");
}