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

// ["precomputed rocks", true]
const unsigned char precomputedCbor[] = { 0x82, 0x71, 0x70, 0x72, 0x65, 0x63, 0x6f, 0x6d, 0x70, 0x75, 
                                          0x74, 0x65, 0x64, 0x20, 0x72, 0x6f, 0x63, 0x6b, 0x73, 0xf5 };

#define BUFFER_SIZE 300
static unsigned char outputBuffer[BUFFER_SIZE];

int main()
{
    char blob1[40] = {4.5,6,7,8,9,10};
    char blob2[] = {-1, 5};
    CBOR_BUFFER cborBuffer;
    cborBuffer.data = outputBuffer;
    cborBuffer.length = BUFFER_SIZE;
    cborBuffer.pos = 0;
    addMap(&cborBuffer, 4);
    addInt(&cborBuffer, 1);
    addBinary(&cborBuffer, blob1, sizeof(blob1));
    addInt(&cborBuffer, 2);
    addBinary(&cborBuffer, blob2, sizeof(blob2));
    addInt(&cborBuffer, 3);
    addRaw(&cborBuffer, precomputedCbor, sizeof(precomputedCbor));
    addInt(&cborBuffer, 4);
    addArray(&cborBuffer, 3);
    addInt(&cborBuffer, 1000);
    addInt(&cborBuffer, -523);
    addString(&cborBuffer, "Hello D-CBOR world!");
    printCborBuffer(&cborBuffer);

}

