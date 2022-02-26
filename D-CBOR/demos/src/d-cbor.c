// D-CBOR Core

#include <stdio.h>
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


static unsigned char* getChunk(CBOR_BUFFER *cborBuffer, int size) {
    if (cborBuffer->pos + size >= cborBuffer->length) {
        printf("BUFFER FAILED");
    }
    unsigned char* ptr = cborBuffer->data + cborBuffer->pos;
    cborBuffer->pos += size;
    return ptr;
}

void addRawCbor(CBOR_BUFFER* cborBuffer, const void* rawCbor, int sizeofRawCbor) {
    memcpy(getChunk(cborBuffer, sizeofRawCbor), rawCbor, sizeofRawCbor);
}

void encodeTagAndValue(CBOR_BUFFER *cborBuffer, int tag, int length, uint64_t value) {
    unsigned char* ptr = getChunk(cborBuffer, length + 1);
    *ptr = (unsigned char) tag;
    while (length > 0) {
        ptr[length--] = (unsigned char)value;
        value >>= 8;
    }
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

void addBstr(CBOR_BUFFER* cborBuffer, const void* blob, int sizeofBlob) {
    encodeTagAndN(cborBuffer, MT_BYTE_STRING, sizeofBlob);
    addRawCbor(cborBuffer, blob, sizeofBlob);
}

void addArray(CBOR_BUFFER* cborBuffer, int elements) {
    encodeTagAndN(cborBuffer, MT_ARRAY, (uint64_t)elements);
}

void addMap(CBOR_BUFFER* cborBuffer, int keys) {
    encodeTagAndN(cborBuffer, MT_MAP, (uint64_t)keys);
}