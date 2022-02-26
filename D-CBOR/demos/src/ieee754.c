// D-CBOR IEEE-754 support

#include <string.h>

#include "d-cbor.h"

const int FLOAT16_SIGNIFICAND_SIZE = 10;
const int FLOAT32_SIGNIFICAND_SIZE = 23;
const int FLOAT64_SIGNIFICAND_SIZE = 52;

const int FLOAT16_EXPONENT_SIZE = 5;
const int FLOAT32_EXPONENT_SIZE = 8;
const int FLOAT64_EXPONENT_SIZE = 11;

const int FLOAT16_EXPONENT_BIAS = 15;
const int FLOAT32_EXPONENT_BIAS = 127;
const int FLOAT64_EXPONENT_BIAS = 1023;

const uint64_t FLOAT16_NOT_A_NUMBER = 0x0000000000007e00ul;
const uint64_t FLOAT16_POS_INFINITY = 0x0000000000007c00ul;
const uint64_t FLOAT16_NEG_INFINITY = 0x000000000000fc00ul;
const uint64_t FLOAT16_POS_ZERO     = 0x0000000000000000ul;
const uint64_t FLOAT16_NEG_ZERO     = 0x0000000000008000ul;

const uint64_t FLOAT64_NOT_A_NUMBER = 0x7ff8000000000000ul;
const uint64_t FLOAT64_POS_INFINITY = 0x7ff0000000000000ul;
const uint64_t FLOAT64_NEG_INFINITY = 0xfff0000000000000ul;
const uint64_t FLOAT64_POS_ZERO     = 0x0000000000000000ul;
const uint64_t FLOAT64_NEG_ZERO     = 0x8000000000000000ul;

const uint64_t ONE                  = 0x0000000000000001ul;

const int MT_FLOAT16 = 0xf9;
const int MT_FLOAT32 = 0xfa;
const int MT_FLOAT64 = 0xfb;

void addDouble(CBOR_BUFFER *cborBuffer, double value) {

    // Initial assumption: value is a plain vanilla 64-bit double.
    int tag = MT_FLOAT64;
    uint64_t bitFormat;
    memcpy(&bitFormat, &value, sizeof(uint64_t));

    // Check for possible edge cases.
    if ((bitFormat & ~FLOAT64_NEG_ZERO) == FLOAT64_POS_ZERO) {
        // Some zeroes are more zero than others :)
        tag = MT_FLOAT16;
        bitFormat = (bitFormat == FLOAT64_POS_ZERO) ? FLOAT16_POS_ZERO : FLOAT16_NEG_ZERO;
    } else if ((bitFormat & FLOAT64_POS_INFINITY) == FLOAT64_POS_INFINITY) {
        // Special "number".
        tag = MT_FLOAT16;
        bitFormat = (bitFormat == FLOAT64_POS_INFINITY) ?
            FLOAT16_POS_INFINITY : (bitFormat == FLOAT64_NEG_INFINITY) ?
                // Deterministic representation of NaN => No NaN "signaling".
                FLOAT16_NEG_INFINITY : FLOAT16_NOT_A_NUMBER;
    } else {
        // It is apparently a regular number. Does it fit in a 32-bit float?

        uint64_t signBit = bitFormat & FLOAT64_NEG_ZERO;
#ifndef D_CBOR_FLOAT_CAST
        int64_t exponent = ((bitFormat >> FLOAT64_SIGNIFICAND_SIZE) &
            ((ONE << FLOAT64_EXPONENT_SIZE) - 1)) -
            (FLOAT64_EXPONENT_BIAS - FLOAT32_EXPONENT_BIAS);
        if (exponent > ((int64_t)FLOAT32_EXPONENT_BIAS << 1)) {
            // Too big for float32 or into the space reserved for NaN and Infinity.
            goto generate;
        }

        uint64_t significand = bitFormat & ((ONE << FLOAT64_SIGNIFICAND_SIZE) - 1);
        if ((significand &
            ((ONE << (FLOAT64_SIGNIFICAND_SIZE - FLOAT32_SIGNIFICAND_SIZE)) - 1)) != 0) {
            // Losing significand bits is not an option.
            goto generate;
        }
        significand >>= (FLOAT64_SIGNIFICAND_SIZE - FLOAT32_SIGNIFICAND_SIZE);

        // Check if we need to denormalize data.
        if (exponent <= 0) {
            // The implicit "1" becomes explicit using subnormal representation.
            significand += ONE << FLOAT32_SIGNIFICAND_SIZE;
            exponent--;
            // Always perform at least one turn.
            do {
                if ((significand & 1) != 0) {
                    // Too off scale for float32.
                    // This test also catches subnormal float64 numbers.
                    goto generate;
                }
                significand >>= 1;
            } while (++exponent < 0);
        }
#else
        float floatValue = (double)((float)value);
        if (value != floatValue) {
            // After casting to float32 something got lost so we stick to float64.
            goto generate;
        }
#endif

        // New assumption: 32-bit float representation.
        tag = MT_FLOAT32;
#ifndef D_CBOR_FLOAT_CAST
        bitFormat =
            // Put possible sign bit in position.
            (signBit >> (64 - 32)) +
            // Exponent.  Put it in front of significand.
            (exponent << FLOAT32_SIGNIFICAND_SIZE) +
            // Significand.
            significand;
#else
        uint32_t floatBinary;
        memcpy(&floatBinary, &floatValue, sizeof(float));
        bitFormat = floatBinary;
#endif

        // However, we must still check if the number could fit in a 16-bit float.
#ifdef D_CBOR_FLOAT_CAST
        int64_t exponent = ((bitFormat >> FLOAT32_SIGNIFICAND_SIZE) &
            ((ONE << FLOAT32_EXPONENT_SIZE) - 1)) -
            (FLOAT32_EXPONENT_BIAS - FLOAT16_EXPONENT_BIAS);
#else
        exponent -= (FLOAT32_EXPONENT_BIAS - FLOAT16_EXPONENT_BIAS);
#endif
        if (exponent > ((int64_t)FLOAT16_EXPONENT_BIAS << 1)) {
            // Too big for float16 or into the space reserved for NaN and Infinity.
            goto generate;
        }

#ifdef D_CBOR_FLOAT_CAST
        uint64_t significand = bitFormat & ((ONE << FLOAT32_SIGNIFICAND_SIZE) - 1);
#endif
        if ((significand &
            ((ONE << (FLOAT32_SIGNIFICAND_SIZE - FLOAT16_SIGNIFICAND_SIZE)) - 1)) != 0) {
            // Losing significand bits is not an option.
            goto generate;
        }
        significand >>= (FLOAT32_SIGNIFICAND_SIZE - FLOAT16_SIGNIFICAND_SIZE);

        // Check if we need to denormalize data.
        if (exponent <= 0) {
            // The implicit "1" becomes explicit using subnormal representation.
            significand += ONE << FLOAT16_SIGNIFICAND_SIZE;
            exponent--;
            // Always perform at least one turn.
            do {
                if ((significand & 1) != 0) {
                    // Too off scale for float16.
                    // This test also catches subnormal float32 numbers.
                    goto generate;
                }
                significand >>= 1;
            } while (++exponent < 0);
        }

        // Seems like 16 bits indeed are sufficient!
        tag = MT_FLOAT16;
        bitFormat =
            // Put possible sign bit in position.
            (signBit >> (64 - 16)) +
            // Exponent.  Put it in front of significand.
            (exponent << FLOAT16_SIGNIFICAND_SIZE) +
            // Significand.
            significand;
    }
generate:
    encodeTagAndValue(cborBuffer, tag, 2 << (tag - MT_FLOAT16), bitFormat);
}