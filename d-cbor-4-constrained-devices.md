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
are programmed), deploys methods that put very modest requirements on a
CBOR encoder:

- Precomputed CBOR data
- Prearranged map key ordering and array sizing
- Optionally using "[fixups](#handling-indefinite-length-data)"

These methods also consume very limited amounts of RAM beyond the actual
CBOR output buffer.
```c
#include <d-cbor.h>

// ["precomputed rocks", true]
static const uint8_t precomputedCbor[] = {
    0x82, 0x71, 0x70, 0x72, 0x65, 0x63, 0x6f, 0x6d, 0x70, 0x75,
    0x74, 0x65, 0x64, 0x20, 0x72, 0x6f, 0x63, 0x6b, 0x73, 0xf5
};

// A couple of blob objects
static const uint8_t blob1[40] = { 4, 6, 7, 8, 9, 10, 'C', 'B', 'O', 'R' };
static const uint8_t blob2[]   = { -1, 5 };

#define BUFFER_SIZE 300

int main(int argc, const char* argv[]) {
    // Buffer setup, here using the stack for storage.
    unsigned char outputBuffer[BUFFER_SIZE];
    CBOR_BUFFER cborBuffer;
    cborBuffer.data = outputBuffer;
    cborBuffer.length = BUFFER_SIZE;
    cborBuffer.pos = 0;

    // Generate deterministic CBOR using prearranged structures.
    addMap(&cborBuffer, 5);  // {#,#,#,#,#}
      addMappedBstr(&cborBuffer, 1, blob1, sizeof(blob1));  // Key: 1
      addMappedBstr(&cborBuffer, 2, blob2, sizeof(blob2));  // Key: 2
      addInt(&cborBuffer, 3);  // key: 3
        addArray(&cborBuffer, 3);  // [#,#,#]
          addInt(&cborBuffer, 9223372036854775807l);
          addInt(&cborBuffer, -523);
          addTstr(&cborBuffer, "Hello D-CBOR world!");
      addInt(&cborBuffer, 4);  // key: 4
      addRawBytes(&cborBuffer, precomputedCbor, sizeof(precomputedCbor));
      addInt(&cborBuffer, 5);  // key: 5
      addArray(&cborBuffer, 4);  // [#,#,#,#]
        addDouble(&cborBuffer, 35.6);
        addDouble(&cborBuffer, 3.4028234663852886e+38);
        addDouble(&cborBuffer, -3.4028234663852889e+38);
        addDouble(&cborBuffer, 5.9604644775390625e-8);

    // Do something with the generated CBOR.
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

void addMappedInt(CBOR_BUFFER* cborBuffer, int key, int value) {
    addInt(cborBuffer, key);
    addInt(cborBuffer, value);
}

void addMappedTstr(CBOR_BUFFER* cborBuffer, int key, const uint8_t* utf8String) {
    addInt(cborBuffer, key);
    addTstr(cborBuffer, utf8String);
}

void addMappedBstr(CBOR_BUFFER* cborBuffer, int key, const uint8_t* byteString, int length) {
    addInt(cborBuffer, key);
    addBstr(cborBuffer, byteString, length);
}
```
### Handling Indefinite-Length Data
In some cases it may not be possible to in _advance_ determine
how many items there will be in an array.  This can be dealt with
while maintaining strict D-CBOR compatibility by using "fixups".
Consider the following extension to the sample encoder:
```c
void insertArray(CBOR_BUFFER* cborBuffer, int savePos, int elements) {
    int lastPos = cborBuffer->pos;
    addArray(cborBuffer, elements);
    if (cborBuffer->length) {  // Buffer overflow protection.
        uint8_t buffer[5];  // 2^32 - 1 elements is not sufficient?
        int q = cborBuffer->pos - lastPos;  // Length in bytes of the array object.
        // Put the array object in front of its associated array elements.
        memmove(buffer, &cborBuffer->data[lastPos], q);
        memmove(&cborBuffer->data[savePos + q], &cborBuffer->data[savePos], lastPos - savePos);
        memmove(&cborBuffer->data[savePos], buffer, q);
    }
}
```
Then indefinite-length arrays would be handled like this:
```c
    // Save current buffer position:
    int savePos = cborBuffer.pos;
      add*(&cborBuffer, /* array element */);
      add*(&cborBuffer, /* array element */);
      add*(&cborBuffer, /* array element */);

      // Etc.

    // After reaching end of input:
    insertArray(&cborBuffer, savePos, /* number of array elements found */);
```
Similar techniques can be applied to indefinte-length strings as well.

### Running the Example
A runnable version of this example can be found in:
[constrained-device-demo](constrained-device-demo).

The floating point encoder resides in a separate file:
[lib/d-cbor-ieee754.c](lib/d-cbor-ieee754.c).
