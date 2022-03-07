// constrained-device-demo.c

#include "d-cbor.h"

#ifndef CBOR_NO_DOUBLE
#include <math.h>
#endif

// ["precomputed rocks", true]
static const uint8_t precomputedCbor[] = { 
    0x82, 0x71, 0x70, 0x72, 0x65, 0x63, 0x6f, 0x6d, 0x70, 0x75, 
    0x74, 0x65, 0x64, 0x20, 0x72, 0x6f, 0x63, 0x6b, 0x73, 0xf5 };

// A couple of blob objects
static const uint8_t blob1[40] = { 4, 6, 7, 8, 9, 10, 'C', 'B', 'O', 'R'};
static const uint8_t blob2[]   = { -1, 5 };

#define BUFFER_SIZE 300

int main(int argc, const char* argv[]) {
    (void)argc; // Avoid unused parameter error

    // Buffer setup, here using the stack for storage.
    unsigned char outputBuffer[BUFFER_SIZE];
    CBOR_BUFFER cborBuffer;
    cborBuffer.data = outputBuffer;
    cborBuffer.length = BUFFER_SIZE;
    cborBuffer.pos = 0;

    // Generate deterministic CBOR using prearranged map key sorting.
#ifndef CBOR_NO_DOUBLE
    addMap(&cborBuffer, 5);  // {#,#,#,#,#}
#else
    addMap(&cborBuffer, 4);  // {#,#,#,#}
#endif
      addInt(&cborBuffer, 1);  // key: 1
      addBstr(&cborBuffer, blob1, sizeof(blob1));
      addInt(&cborBuffer, 2);  // key: 2
      addBstr(&cborBuffer, blob2, sizeof(blob2));
      addInt(&cborBuffer, 3);  // key: 3
#ifdef INDEFINITE_LENGTH_EMULATION
      int savePos = cborBuffer.pos;
#else
      addArray(&cborBuffer, 3);  // [#,#,#]
#endif
        addInt(&cborBuffer, 9223372036854775807l);
        addInt(&cborBuffer, -523);
        addTstr(&cborBuffer, "Hello D-CBOR world!");
#ifdef INDEFINITE_LENGTH_EMULATION
      insertArray(&cborBuffer, savePos, 3);
#endif
      addInt(&cborBuffer, 4);  // key: 4
      addRawBytes(&cborBuffer, precomputedCbor, sizeof(precomputedCbor));
#ifndef CBOR_NO_DOUBLE
      addInt(&cborBuffer, 5);  // key: 5
      addArray(&cborBuffer, 8);  // [#,#,#,#,#,#,#,#]
        addDouble(&cborBuffer, 35.6);
        addDouble(&cborBuffer, 3.4028234663852886e+38);
        addDouble(&cborBuffer, -3.4028234663852889e+38);
        addDouble(&cborBuffer, 5.9604644775390625e-8);
        addDouble(&cborBuffer, 0.0);
        addDouble(&cborBuffer, -0.0);
        addDouble(&cborBuffer, NAN);
        addDouble(&cborBuffer, INFINITY);
#endif

    // Do something with the generated CBOR.
    printCborBuffer(&cborBuffer);
}