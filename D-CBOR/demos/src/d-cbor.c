// D-CBOR - Core encoder for constrained devices.

#include <string.h>

#include "d-cbor.h"

const uint64_t MASK_LOWER_32 = 0x00000000fffffffful;

const int MT_UNSIGNED      = 0x00;
const int MT_NEGATIVE      = 0x20;
const int MT_BYTE_STRING   = 0x40;
const int MT_TEXT_STRING   = 0x60;
const int MT_ARRAY         = 0x80;
const int MT_MAP           = 0xa0;
const int MT_TAG_EXTENSION = 0xc0;
const int MT_BIG_UNSIGNED  = 0xc2;
const int MT_BIG_SIGNED    = 0xc3;
const int MT_FALSE         = 0xf4;
const int MT_TRUE          = 0xf5;
const int MT_NULL          = 0xf6;

static void putCborByte(CBOR_BUFFER *cborBuffer, uint8_t byte) {
    if (cborBuffer->pos >= cborBuffer->length) {
        // Buffer overflow! Ignore call to avoid crashing hard.
        cborBuffer->length = 0;  // Indication to upper layers.
    } else {
        cborBuffer->data[cborBuffer->pos++] = byte;
    }
}

void addRawCbor(CBOR_BUFFER* cborBuffer, const uint8_t* rawCbor, int sizeofRawCbor) {
    while (--sizeofRawCbor >= 0) {
        putCborByte(cborBuffer, *rawCbor++);
    }
}

void encodeTagAndValue(CBOR_BUFFER *cborBuffer, int tag, int length, uint64_t value) {
    putCborByte(cborBuffer, (uint8_t) tag);
    uint8_t buffer[8];
    int i = length;
    while (--i >= 0) {
        buffer[i] = (uint8_t)value;
        value >>= 8;
    }
    addRawCbor(cborBuffer, buffer, length);
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

void addTstr(CBOR_BUFFER* cborBuffer, const char* utf8String) {
    int length = strlen(utf8String);
    encodeTagAndN(cborBuffer, MT_TEXT_STRING, length);
    addRawCbor(cborBuffer, utf8String, length);
}

void addBstr(CBOR_BUFFER* cborBuffer, const uint8_t* blob, int sizeofBlob) {
    encodeTagAndN(cborBuffer, MT_BYTE_STRING, sizeofBlob);
    addRawCbor(cborBuffer, blob, sizeofBlob);
}

void addArray(CBOR_BUFFER* cborBuffer, int elements) {
    encodeTagAndN(cborBuffer, MT_ARRAY, (uint64_t)elements);
}

void addMap(CBOR_BUFFER* cborBuffer, int keys) {
    encodeTagAndN(cborBuffer, MT_MAP, (uint64_t)keys);
}