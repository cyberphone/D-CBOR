#pragma once

#include <stdint.h>

typedef struct {
    int length;
    int pos;
    uint8_t *data;
} CBOR_BUFFER;

void addInt(CBOR_BUFFER* cborBuffer, int64_t value);

void addTstr(CBOR_BUFFER* cborBuffer, const uint8_t* utf8String);

void addBstr(CBOR_BUFFER* cborBuffer, const uint8_t* blob, int sizeofBlob);

void addRawCbor(CBOR_BUFFER* cborBuffer, const uint8_t* rawCbor, int sizeofRawCbor);

void addArray(CBOR_BUFFER* cborBuffer, int elements);

void addMap(CBOR_BUFFER* cborBuffer, int keys);

void encodeTagAndValue(CBOR_BUFFER* cborBuffer, int tag, int length, uint64_t value);

#ifndef CBOR_NO_DOUBLE
void addDouble(CBOR_BUFFER* cborBuffer, double value);
#endif
