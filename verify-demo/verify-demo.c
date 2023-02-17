// verify-demo.c

// Minimalist CSF verifier demo using https://github.com/laurencelundblade/QCBOR

#include <qcbor/qcbor_decode.h>
#include <qcbor/qcbor_spiffy_decode.h>

#include "print-buffer.h"
#include "csf-verifier.h"

/*
    {
      1: "Hello signed CBOR world!",
      2: [2.0, true],
      -1: {
        1: -8,
        4: {
          1: 1,
          -1: 6,
          -2: h'fe49acf5b92b6e923594f2e83368f680ac924be93cf533aecaf802e37757f8c9'
        },
        6: h'484740d0807f4629252dd8b8ba3ead6c2648a2642c22a971f4ca336254fe42e138f417c5e7c9e89b91eb23ca8cc5163c81944f75d07b157ac904814802dbc306'
      }
    }
*/

static const uint8_t CSF_SIGNED_CBOR[] = {
    0xa3, 0x01, 0x78, 0x18, 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x73, 0x69, 0x67, 0x6e, 0x65, 0x64,
    0x20, 0x43, 0x42, 0x4f, 0x52, 0x20, 0x77, 0x6f, 0x72, 0x6c, 0x64, 0x21, 0x02, 0x82, 0xf9, 0x40,
    0x00, 0xf5, 0x20, 0xa3, 0x01, 0x27, 0x04, 0xa3, 0x01, 0x01, 0x20, 0x06, 0x21, 0x58, 0x20, 0xfe,
    0x49, 0xac, 0xf5, 0xb9, 0x2b, 0x6e, 0x92, 0x35, 0x94, 0xf2, 0xe8, 0x33, 0x68, 0xf6, 0x80, 0xac,
    0x92, 0x4b, 0xe9, 0x3c, 0xf5, 0x33, 0xae, 0xca, 0xf8, 0x02, 0xe3, 0x77, 0x57, 0xf8, 0xc9, 0x06,
    0x58, 0x40, 0x48, 0x47, 0x40, 0xd0, 0x80, 0x7f, 0x46, 0x29, 0x25, 0x2d, 0xd8, 0xb8, 0xba, 0x3e,
    0xad, 0x6c, 0x26, 0x48, 0xa2, 0x64, 0x2c, 0x22, 0xa9, 0x71, 0xf4, 0xca, 0x33, 0x62, 0x54, 0xfe,
    0x42, 0xe1, 0x38, 0xf4, 0x17, 0xc5, 0xe7, 0xc9, 0xe8, 0x9b, 0x91, 0xeb, 0x23, 0xca, 0x8c, 0xc5,
    0x16, 0x3c, 0x81, 0x94, 0x4f, 0x75, 0xd0, 0x7b, 0x15, 0x7a, 0xc9, 0x04, 0x81, 0x48, 0x02, 0xdb,
    0xc3, 0x06
};

static const int APP_DATA_ONE_LABEL  = 1;
static const int APP_DATA_TWO_LABEL  = 2;
static const int APP_SIGNATURE_LABEL = -1;

int main(int argc, const char* argv[]) {
    (void)argc; // Avoid unused parameter error

    // Fill input buffer with pre-computed CBOR.
    UsefulBuf_MAKE_STACK_UB(inputBuffer, 300);
    UsefulOutBuf UOB;
    UsefulOutBuf_Init(&UOB, inputBuffer);
    UsefulOutBuf_AppendData(&UOB, CSF_SIGNED_CBOR, sizeof(CSF_SIGNED_CBOR));

    // Setup decoder.
    QCBORDecodeContext DecodeCtx;
    QCBORDecode_Init(&DecodeCtx, UsefulOutBuf_OutUBuf(&UOB), QCBOR_DECODE_MODE_NORMAL);

    // For debug and demo purposes only...
    printUsefulBufC(&DecodeCtx.InBuf.UB, "Signed CBOR data");

    // Decode.  Start by entering into the map.
    QCBORDecode_EnterMap(&DecodeCtx, NULL);
    ///////////////////////////////
    //  Fetch Application Data   //
    ///////////////////////////////
 
    // First map entry should be a text string.
    UsefulBufC appTextString;
    QCBORDecode_GetTextStringInMapN(&DecodeCtx, APP_DATA_ONE_LABEL, &appTextString);
    // Second map entry should be a 2-dimensional array.  Step into it.
    QCBORDecode_EnterArrayFromMapN(&DecodeCtx, APP_DATA_TWO_LABEL);
      // First array entry should be a floating point value.
      double appFloatingPoint;
      QCBORDecode_GetDouble(&DecodeCtx, &appFloatingPoint);
      // Second array entry should be a boolean.
      bool appBoolean;
      QCBORDecode_GetBool(&DecodeCtx, &appBoolean);
    // Done with the array.  Step out of it.
    QCBORDecode_ExitArray(&DecodeCtx);
    // End of application data.

    ///////////////////////////////
    //     Verify Signature      //
    ///////////////////////////////

    // The third map entry should contain the CSF object.  Verify it.
    bool result = csfVerifier(&DecodeCtx, APP_SIGNATURE_LABEL);

    // For the demo only...
    printf("Signature validation result: ");
    if (result) {
        printf("VALID\n");
    } else {
        printf("***INVALID***\n");
    }
}