// d-cbor-4-constrained-device.c

#include <stdio.h>

#include "src/d-cbor.h"

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

// ["precomputed rocks", true]
const unsigned char precomputedCbor[] = { 
    0x82, 0x71, 0x70, 0x72, 0x65, 0x63, 0x6f, 0x6d, 0x70, 0x75, 
    0x74, 0x65, 0x64, 0x20, 0x72, 0x6f, 0x63, 0x6b, 0x73, 0xf5 };

// A couple of blob objects
const char blob1[40] = { 4, 6, 7, 8, 9, 10 };
const char blob2[]   = { -1, 5 };

#define BUFFER_SIZE 30

void main() {
    // Buffer setup, here using the stack for storage.
    unsigned char outputBuffer[BUFFER_SIZE];
    CBOR_BUFFER cborBuffer;
    cborBuffer.data = outputBuffer;
    cborBuffer.length = BUFFER_SIZE;
    cborBuffer.pos = 0;

    // Generate deterministic CBOR using ordered map keys.
#ifndef D_CBOR_NO_DOUBLE
    addMap(&cborBuffer, 5);  // {#,#,#,#,#}
#else
    addMap(&cborBuffer, 4);  // {#,#,#,#}
#endif
      addInt(&cborBuffer, 1);  // key: 1
      addBstr(&cborBuffer, blob1, sizeof(blob1));
      addInt(&cborBuffer, 2);  // key: 2
      addBstr(&cborBuffer, blob2, sizeof(blob2));
      addInt(&cborBuffer, 3);  // key: 3
      addArray(&cborBuffer, 3);  // [#,#,#]
        addInt(&cborBuffer, 9223372036854775807l);
        addInt(&cborBuffer, -523);
        addTstr(&cborBuffer, "Hello D-CBOR world!");
      addInt(&cborBuffer, 4);  // key: 4
      addRawCbor(&cborBuffer, precomputedCbor, sizeof(precomputedCbor));
#ifndef D_CBOR_NO_DOUBLE
      addInt(&cborBuffer, 5);  // key: 5
      addArray(&cborBuffer, 4);  // [#,#,#]
      addDouble(&cborBuffer, 35.6);
      addDouble(&cborBuffer, 3.4028234663852886e+38);
      addDouble(&cborBuffer, 3.4028234663852889e+38);
      addDouble(&cborBuffer, 5.9604644775390625e-8);
#endif

    // Do something with the generated CBOR.
    printCborBuffer(&cborBuffer);
}