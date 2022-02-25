// demos.c : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>

#include "src/d-cbor.h"

void printCborBuffer(CBOR_BUFFER *cborBuffer) {
    printf("l=%d\n", cborBuffer->pos);
    for (int i = 0; i < cborBuffer->pos; i++) {
        printf("%02x", (int) cborBuffer->data[i]);
    }
    printf("\n");
}

#define BUFFER_SIZE 300
unsigned char outputBuffer[BUFFER_SIZE];

int main()
{
    printf("Hello %d World!\n", myfunc());

    CBOR_BUFFER cborBuffer;
    cborBuffer.data = outputBuffer;
    cborBuffer.length = BUFFER_SIZE;
    cborBuffer.pos = 0;
    encodeTagAndN(&cborBuffer, 0, 1000ul);
    printCborBuffer(&cborBuffer);

}

