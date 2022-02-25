#pragma once

int myfunc();

typedef struct {
    int length;
    int pos;
    unsigned char *data;
} CBOR_BUFFER;


void encodeTagAndValue(CBOR_BUFFER *cborBuffer, int tag, int length, unsigned __int64 value);

void encodeTagAndN(CBOR_BUFFER *cborBuffer, int majorType, unsigned __int64 n);
