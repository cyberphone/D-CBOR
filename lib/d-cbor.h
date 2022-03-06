#pragma once

#include <stdint.h>

typedef struct {
    int length;
    int pos;
    uint8_t *data;
} CBOR_BUFFER;

void addInt(CBOR_BUFFER* cborBuffer, int64_t value);

void addTstr(CBOR_BUFFER* cborBuffer, const uint8_t* utf8String);

void addBstr(CBOR_BUFFER* cborBuffer, const uint8_t* byteString, int length);

void addRawBytes(CBOR_BUFFER* cborBuffer, const uint8_t* bytePointer, int length);

void addArray(CBOR_BUFFER* cborBuffer, int elements);

void addMap(CBOR_BUFFER* cborBuffer, int keys);

void encodeTagAndValue(CBOR_BUFFER* cborBuffer, int tag, int length, uint64_t value);

void printCborBuffer(CBOR_BUFFER* cborBuffer);

void addMappedInt(CBOR_BUFFER* cborBuffer, int key, int value);

void addMappedTstr(CBOR_BUFFER* cborBuffer, int key, const uint8_t* utf8String);

void addMappedBstr(CBOR_BUFFER* cborBuffer, int key, const uint8_t* byteString, int length);

#ifndef CBOR_NO_DOUBLE
// Note: the implementation is in "ieee754.c"
void addDouble(CBOR_BUFFER* cborBuffer, double value);
#endif

#ifdef INDEFINITE_LENGTH_EMULATION
void insertArray(CBOR_BUFFER* cborBuffer, int savePos, int elements);
#endif
