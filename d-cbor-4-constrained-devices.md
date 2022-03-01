## D-CBOR for Constrained Devices

One might believe that devices having limited storage and processing
capabilities would be out of scope for D-CBOR.
However, constrained devices like Trusted Platform Module (TPM) chips 
already generate a flavor of deterministic encoding known as ASN.1 DER.
Compared to ASN.1, CBOR is much easier to handle, since embedding
constructs like maps and arrays only need to provide the expected
_number of elements_, while their ASN.1 counterparts require the
actual _length in bytes_ of the embedded elements to be known in advance.

A fair assumption is that constrained devices also come with equally
constrained vocabularies and only use a small set of data types.

The following example (which probably is pretty close to how TPMs currently
are programmed), deploys methods that put very modest requirements on the
CBOR encoder:

- Precomputed CBOR data
- Prearranged map key ordering and array sizing

These methods also consume very limited amounts of RAM beyond the actual
CBOR output buffer.
```c
#include "d-cbor.h"

// ["precomputed rocks", true]
static const uint8_t precomputedCbor[] = { 
    0x82, 0x71, 0x70, 0x72, 0x65, 0x63, 0x6f, 0x6d, 0x70, 0x75, 
    0x74, 0x65, 0x64, 0x20, 0x72, 0x6f, 0x63, 0x6b, 0x73, 0xf5 };

// A couple of blob objects
static const uint8_t blob1[100] = { 4, 6, 7, 8, 9, 10 };
static const uint8_t blob2[]    = { -1, 5 };

#define BUFFER_SIZE 300

void main() {
    // Buffer setup, here using the stack for storage.
    unsigned char outputBuffer[BUFFER_SIZE];
    CBOR_BUFFER cborBuffer;
    cborBuffer.data = outputBuffer;
    cborBuffer.length = BUFFER_SIZE;
    cborBuffer.pos = 0;

    // Generate deterministic CBOR using prearranged structures.
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
      addArray(&cborBuffer, 3);  // [#,#,#]
        addInt(&cborBuffer, 9223372036854775807l);
        addInt(&cborBuffer, -523);
        addTstr(&cborBuffer, "Hello D-CBOR world!");
      addInt(&cborBuffer, 4);  // key: 4
      addRawBytes(&cborBuffer, precomputedCbor, sizeof(precomputedCbor));
#ifndef CBOR_NO_DOUBLE
      addInt(&cborBuffer, 5);  // key: 5
      addArray(&cborBuffer, 4);  // [#,#,#,#]
        addDouble(&cborBuffer, 35.6);
        addDouble(&cborBuffer, 3.4028234663852886e+38);
        addDouble(&cborBuffer, -3.4028234663852889e+38);
        addDouble(&cborBuffer, 5.9604644775390625e-8);
#endif

    // Now do something with the generated CBOR...
}
```
### Compatible Encoder Sample
Header file extract:
```c
#include <stdint.h>

typedef struct {
    int length;
    int pos;
    uint8_t *data;
} CBOR_BUFFER;
```
Actual encoder:
```c
#include <string.h>

#include "d-cbor.h"

static const uint64_t MASK_LOWER_32 = 0x00000000fffffffful;

static const int MT_UNSIGNED      = 0x00;
static const int MT_NEGATIVE      = 0x20;
static const int MT_BYTE_STRING   = 0x40;
static const int MT_TEXT_STRING   = 0x60;
static const int MT_ARRAY         = 0x80;
static const int MT_MAP           = 0xa0;

static void putByte(CBOR_BUFFER *cborBuffer, uint8_t byte) {
    if (cborBuffer->pos >= cborBuffer->length) {
        // Buffer overflow! Ignore call to avoid crashing hard.
        cborBuffer->length = 0;  // Indication to upper layers.
    } else {
        cborBuffer->data[cborBuffer->pos++] = byte;
    }
}

void addRawBytes(CBOR_BUFFER* cborBuffer, const uint8_t* bytePointer, int length) {
    while (--length >= 0) {
        putByte(cborBuffer, *bytePointer++);
    }
}

void encodeTagAndValue(CBOR_BUFFER *cborBuffer, int tag, int length, uint64_t value) {
    putByte(cborBuffer, (uint8_t) tag);
    uint8_t buffer[8];
    int i = length;
    while (--i >= 0) {
        buffer[i] = (uint8_t)value;
        value >>= 8;
    }
    addRawBytes(cborBuffer, buffer, length);
}

void encodeTagAndN(CBOR_BUFFER *cborBuffer, int majorType, uint64_t n) {
    int modifier = (int)n;
    int length = 0;
    if (n > 23) {
        modifier = 27;
        length = 8;
        while (((MASK_LOWER_32 << ((length / 2) * 8)) & n) == 0) {
            modifier--;
            length >>= 1;
        }
    }
    encodeTagAndValue(cborBuffer, majorType | modifier, length, n);
}

void addInt(CBOR_BUFFER* cborBuffer, int64_t value) {
    int tag = MT_UNSIGNED;
    if (value < 0) {
        tag = MT_NEGATIVE;
        value = ~value;
    }
    encodeTagAndN(cborBuffer, tag, (uint64_t)value);
}

void addTstr(CBOR_BUFFER* cborBuffer, const uint8_t* utf8String) {
    int length = strlen(utf8String);
    encodeTagAndN(cborBuffer, MT_TEXT_STRING, length);
    addRawBytes(cborBuffer, utf8String, length);
}

void addBstr(CBOR_BUFFER* cborBuffer, const uint8_t* byteString, int length) {
    encodeTagAndN(cborBuffer, MT_BYTE_STRING, length);
    addRawBytes(cborBuffer, byteString, length);
}

void addArray(CBOR_BUFFER* cborBuffer, int elements) {
    encodeTagAndN(cborBuffer, MT_ARRAY, elements);
}

void addMap(CBOR_BUFFER* cborBuffer, int keys) {
    encodeTagAndN(cborBuffer, MT_MAP, keys);
}
```
### Running the Example
The optional floating point encoder resides in a separate file:
[lib/d-cbor-ieee754.c](lib/d-cbor-ieee754.c).

A runnable version of this example can be found in:
[d-cbor-4-constrained-device](d-cbor-4-constrained-device).
