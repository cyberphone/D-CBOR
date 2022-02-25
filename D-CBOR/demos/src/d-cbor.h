#pragma once

typedef struct {
    int length;
    int pos;
    unsigned char *data;
} CBOR_BUFFER;

void addInt(CBOR_BUFFER* cborBuffer, __int32 value);

void addString(CBOR_BUFFER* cborBuffer, char* utf8String);

void addBinary(CBOR_BUFFER* cborBuffer, void *blob, int sizeofBlob);

void addRaw(CBOR_BUFFER* cborBuffer, void* cbor, int sizeofCbor);

void addArray(CBOR_BUFFER* cborBuffer, __int32 elements);

void addMap(CBOR_BUFFER* cborBuffer, __int32 elements);
