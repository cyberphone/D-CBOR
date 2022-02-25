// Library file

const int FLOAT16_SIGNIFICAND_SIZE = 10;
const int FLOAT32_SIGNIFICAND_SIZE = 23;
const int FLOAT64_SIGNIFICAND_SIZE = 52;

const int FLOAT16_EXPONENT_SIZE    = 5;
const int FLOAT32_EXPONENT_SIZE    = 8;
const int FLOAT64_EXPONENT_SIZE    = 11;

const int FLOAT16_EXPONENT_BIAS    = 15;
const int FLOAT32_EXPONENT_BIAS    = 127;
const int FLOAT64_EXPONENT_BIAS    = 1023;

const unsigned __int64 FLOAT16_NOT_A_NUMBER = 0x0000000000007e00ul;
const unsigned __int64 FLOAT16_POS_INFINITY = 0x0000000000007c00ul;
const unsigned __int64 FLOAT16_NEG_INFINITY = 0x000000000000fc00ul;
const unsigned __int64 FLOAT16_POS_ZERO     = 0x0000000000000000ul;
const unsigned __int64 FLOAT16_NEG_ZERO     = 0x0000000000008000ul;

const unsigned __int64 FLOAT64_NOT_A_NUMBER = 0x7ff8000000000000ul;
const unsigned __int64 FLOAT64_POS_INFINITY = 0x7ff0000000000000ul;
const unsigned __int64 FLOAT64_NEG_INFINITY = 0xfff0000000000000ul;
const unsigned __int64 FLOAT64_POS_ZERO     = 0x0000000000000000ul;
const unsigned __int64 FLOAT64_NEG_ZERO     = 0x8000000000000000ul;

const unsigned __int64 MASK_LOWER_32        = 0x00000000fffffffful;

#include "d-cbor.h"
#include <stdio.h>

static unsigned char* getChunk(CBOR_BUFFER *cborBuffer, int size) {
    if (cborBuffer->pos + size >= cborBuffer->length) {
        printf("BUFFER FAILED");
    }
    unsigned char* ptr = cborBuffer->data + cborBuffer->pos;
    cborBuffer->pos += size;
    return ptr;
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

int myfunc() {
    return 5;
}