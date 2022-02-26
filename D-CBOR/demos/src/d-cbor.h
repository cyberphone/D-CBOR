#pragma once

#include <stdint.h>

typedef struct {
    int length;
    int pos;
    unsigned char *data;
} CBOR_BUFFER;

void addInt(CBOR_BUFFER* cborBuffer, int64_t value);

void addTstr(CBOR_BUFFER* cborBuffer, const char* utf8String);

void addBstr(CBOR_BUFFER* cborBuffer, const void* blob, int sizeofBlob);

void addRawCbor(CBOR_BUFFER* cborBuffer, const void* rawCbor, int sizeofRawCbor);

void addArray(CBOR_BUFFER* cborBuffer, int elements);

void addMap(CBOR_BUFFER* cborBuffer, int keys);
