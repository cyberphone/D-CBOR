/*
 *  Copyright 2006-2022 WebPKI.org (https://webpki.org).
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      https://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

            ////////////////////////////////////////
            // Determinstic CBOR IEEE-754 encoder //
            ////////////////////////////////////////

#include <string.h>

#include "d-cbor.h"

static const int FLOAT16_SIGNIFICAND_SIZE = 10;
static const int FLOAT32_SIGNIFICAND_SIZE = 23;
static const int FLOAT64_SIGNIFICAND_SIZE = 52;

static const int FLOAT16_EXPONENT_SIZE = 5;
static const int FLOAT32_EXPONENT_SIZE = 8;
static const int FLOAT64_EXPONENT_SIZE = 11;

static const int FLOAT16_EXPONENT_BIAS = 15;
static const int FLOAT32_EXPONENT_BIAS = 127;
static const int FLOAT64_EXPONENT_BIAS = 1023;

static const uint64_t FLOAT16_NOT_A_NUMBER = 0x0000000000007e00ul;
static const uint64_t FLOAT16_POS_INFINITY = 0x0000000000007c00ul;
static const uint64_t FLOAT16_NEG_INFINITY = 0x000000000000fc00ul;
static const uint64_t FLOAT16_POS_ZERO     = 0x0000000000000000ul;
static const uint64_t FLOAT16_NEG_ZERO     = 0x0000000000008000ul;

static const uint64_t FLOAT32_NEG_ZERO     = 0x0000000080000000ul;

static const uint64_t FLOAT64_NOT_A_NUMBER = 0x7ff8000000000000ul;
static const uint64_t FLOAT64_POS_INFINITY = 0x7ff0000000000000ul;
static const uint64_t FLOAT64_NEG_INFINITY = 0xfff0000000000000ul;
static const uint64_t FLOAT64_POS_ZERO     = 0x0000000000000000ul;
static const uint64_t FLOAT64_NEG_ZERO     = 0x8000000000000000ul;

static const uint64_t ONE                  = 0x0000000000000001ul;

static const int MT_FLOAT16 = 0xf9;
static const int MT_FLOAT32 = 0xfa;
static const int MT_FLOAT64 = 0xfb;

void addDouble(CBOR_BUFFER *cborBuffer, double d) {

    // Initial assumption: the number is a plain vanilla 64-bit double.
 
    int tag = MT_FLOAT64;
    uint64_t bitFormat;
    memcpy(&bitFormat, &d, sizeof(uint64_t));

    // Check for possible edge cases.

    if ((bitFormat & ~FLOAT64_NEG_ZERO) == FLOAT64_POS_ZERO) {
        // Some zeroes are apparently more zero than others...
        tag = MT_FLOAT16;
        bitFormat = (bitFormat == FLOAT64_POS_ZERO) ? FLOAT16_POS_ZERO : FLOAT16_NEG_ZERO;
        goto done;  // Ever heard about "goto free" programming? Yes, it sucks :)
    }

    if ((bitFormat & FLOAT64_POS_INFINITY) == FLOAT64_POS_INFINITY) {
        // Special "number".
        tag = MT_FLOAT16;
        bitFormat = (bitFormat == FLOAT64_POS_INFINITY) ?
            FLOAT16_POS_INFINITY : (bitFormat == FLOAT64_NEG_INFINITY) ?
                // Deterministic representation of NaN => Only "quiet" NaN is supported.
                FLOAT16_NEG_INFINITY : FLOAT16_NOT_A_NUMBER;
        goto done;
    }

    // It must be a "regular" number. Does it fit in a 32-bit float?

#ifdef PLATFORM_SUPPORTS_FLOAT_CAST
    // The following code presumes that the underlying floating point system handles
    // overflow conditions and subnormal numbers that may be the result of a conversion.  
    float f = (float)d;
    if (d != f) {
        // "Lost in translation".  Stick to float64.
        goto done;
    }
#else
    // The following code requires no support for floating point conversions,
    // it only uses integer operations.
    int64_t exponent = 
        ((bitFormat >> FLOAT64_SIGNIFICAND_SIZE) & ((ONE << FLOAT64_EXPONENT_SIZE) - 1)) -
        (FLOAT64_EXPONENT_BIAS - FLOAT32_EXPONENT_BIAS);
    if (exponent < -FLOAT32_SIGNIFICAND_SIZE || exponent > ((int64_t)FLOAT32_EXPONENT_BIAS << 1)) {
        // Too small or too big for float32, or running into float32 NaN/Infinity space.
        goto done;
    }

    uint64_t significand = bitFormat & ((ONE << FLOAT64_SIGNIFICAND_SIZE) - 1);
    if ((significand & ((ONE << (FLOAT64_SIGNIFICAND_SIZE - FLOAT32_SIGNIFICAND_SIZE)) - 1)) != 0) {
        // Losing significand bits is not an option.
        goto done;
    }
    significand >>= (FLOAT64_SIGNIFICAND_SIZE - FLOAT32_SIGNIFICAND_SIZE);

    // Check if result needs to be denormalized.
    if (exponent <= 0) {
        // The implicit "1" becomes explicit using subnormal representation.
        significand += ONE << FLOAT32_SIGNIFICAND_SIZE;
        uint64_t significandCopy = significand;
        significand >>= (1 - exponent);
        if (significandCopy != (significand << (1 - exponent))) {
            // Too off scale for float32.
            goto done;
        }
        exponent = 0;
    }
#endif

    // Yes, the number is compatible with 32-bit float representation.

    tag = MT_FLOAT32;
#ifdef PLATFORM_SUPPORTS_FLOAT_CAST
    uint32_t floatBinary;
    memcpy(&floatBinary, &f, sizeof(float));
    bitFormat = floatBinary;
#else
    bitFormat =
        // Put sign bit in position.
        ((bitFormat >> (64 - 32)) & FLOAT32_NEG_ZERO) +
        // Exponent.  Put it in front of significand.
        (exponent << FLOAT32_SIGNIFICAND_SIZE) +
        // Significand.
        significand;
#endif

    // However, the number could potentially fit in a 16-bit float as well.

#ifdef PLATFORM_SUPPORTS_FLOAT_CAST
    int64_t exponent = ((bitFormat >> FLOAT32_SIGNIFICAND_SIZE) &
        ((ONE << FLOAT32_EXPONENT_SIZE) - 1)) -
        (FLOAT32_EXPONENT_BIAS - FLOAT16_EXPONENT_BIAS);
#else
    exponent -= (FLOAT32_EXPONENT_BIAS - FLOAT16_EXPONENT_BIAS);
#endif
    if (exponent < -FLOAT16_SIGNIFICAND_SIZE || exponent > ((int64_t)FLOAT16_EXPONENT_BIAS << 1)) {
        // Too small or too big for float16, or running into float16 NaN/Infinity space.
        goto done;
    }

#ifdef PLATFORM_SUPPORTS_FLOAT_CAST
    uint64_t significand = bitFormat & ((ONE << FLOAT32_SIGNIFICAND_SIZE) - 1);
#endif
    if ((significand & ((ONE << (FLOAT32_SIGNIFICAND_SIZE - FLOAT16_SIGNIFICAND_SIZE)) - 1)) != 0) {
        // Losing significand bits is not an option.
        goto done;
    }
    significand >>= (FLOAT32_SIGNIFICAND_SIZE - FLOAT16_SIGNIFICAND_SIZE);

    // Check if result needs to be denormalized.
    if (exponent <= 0) {
        // The implicit "1" becomes explicit using subnormal representation.
        significand += ONE << FLOAT16_SIGNIFICAND_SIZE;
        uint64_t significandCopy = significand;
        significand >>= (1 - exponent);
        if (significandCopy != (significand << (1 - exponent))) {
            // Too off scale for float16.
            goto done;
        }
        exponent = 0;
    }

    // Seems like 16 bits indeed are sufficient!

    tag = MT_FLOAT16;
    bitFormat =
        // Put sign bit in position.
        ((bitFormat >> (32 - 16)) & FLOAT16_NEG_ZERO) +
        // Exponent.  Put it in front of significand.
        (exponent << FLOAT16_SIGNIFICAND_SIZE) +
        // Significand.
        significand;

done:
    encodeTagAndValue(cborBuffer, tag, 2 << (tag - MT_FLOAT16), bitFormat);
}