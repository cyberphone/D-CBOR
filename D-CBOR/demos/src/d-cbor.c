// D-CBOR Core

const unsigned __int64 MASK_LOWER_32        = 0x00000000fffffffful;

const int MT_UNSIGNED = 0x00;
const int MT_NEGATIVE = 0x20;
const int MT_BYTE_STRING = 0x40;
const int MT_TEXT_STRING = 0x60;
const int MT_ARRAY = 0x80;
const int MT_MAP = 0xa0;
const int MT_TAG_EXTENSION = 0xc0;
const int MT_BIG_UNSIGNED = 0xc2;
const int MT_BIG_SIGNED = 0xc3;
const int MT_FALSE = 0xf4;
const int MT_TRUE = 0xf5;
const int MT_NULL = 0xf6;
const int MT_FLOAT16 = 0xf9;
const int MT_FLOAT32 = 0xfa;
const int MT_FLOAT64 = 0xfb;

#include "d-cbor.h"

#include <stdio.h>
#include <string.h>

static unsigned char* getChunk(CBOR_BUFFER *cborBuffer, int size) {
    if (cborBuffer->pos + size >= cborBuffer->length) {
        printf("BUFFER FAILED");
    }
    unsigned char* ptr = cborBuffer->data + cborBuffer->pos;
    cborBuffer->pos += size;
    return ptr;
}

void addRaw(CBOR_BUFFER* cborBuffer, void* cbor, int sizeofCbor) {
    memcpy(getChunk(cborBuffer, sizeofCbor), cbor, sizeofCbor);
}

void encodeTagAndValue(CBOR_BUFFER *cborBuffer, int tag, int length, unsigned __int64 value) {
    unsigned char* ptr = getChunk(cborBuffer, length + 1);
    *ptr = (unsigned char) tag;
    while (length > 0) {
        ptr[length--] = (unsigned char)value;
        value >>= 8;
    }
}

void encodeTagAndN(CBOR_BUFFER *cborBuffer, int majorType, unsigned __int64 n) {
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

void addInt(CBOR_BUFFER* cborBuffer, __int32 value) {
    int tag = MT_UNSIGNED;
    if (value < 0) {
        tag = MT_NEGATIVE;
        value = ~value;
    }
    encodeTagAndN(cborBuffer, tag, (unsigned __int64)value);
}

void addString(CBOR_BUFFER* cborBuffer, char* utf8String) {
    int length = strlen(utf8String);
    encodeTagAndN(cborBuffer, MT_TEXT_STRING, length);
    addRaw(cborBuffer, utf8String, length);
}

void addBinary(CBOR_BUFFER* cborBuffer, void* blob, int sizeofBlob) {
    encodeTagAndN(cborBuffer, MT_BYTE_STRING, sizeofBlob);
    addRaw(cborBuffer, blob, sizeofBlob);
}

void addArray(CBOR_BUFFER* cborBuffer, __int32 elements) {
    encodeTagAndN(cborBuffer, MT_ARRAY, (unsigned __int64)elements);
}

void addMap(CBOR_BUFFER* cborBuffer, __int32 elements) {
    encodeTagAndN(cborBuffer, MT_MAP, (unsigned __int64)elements);
}