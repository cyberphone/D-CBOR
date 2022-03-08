// verify-demo.c

// Minimalist CSF verifier demo using https://github.com/laurencelundblade/QCBOR

#include "qcbor/qcbor_decode.h"
#include "qcbor/qcbor_spiffy_decode.h"

#include "print-buffer.h"
#include "verifier.h"

/*
    {
      1: "Hello signed CBOR world!",
      2: [2.0, true],
      3: {
        1: -8,
        4: {
          1: 1,
          -1: 6,
          -2: h'fe49acf5b92b6e923594f2e83368f680ac924be93cf533aecaf802e37757f8c9'
        },
        7: h'92afa256cf20d60c6c7df82349267918046acf48c4c95b06fd101123ac12b62e01f0
3b3f778557892a6e07df43ab05b63d0623893781c7026b8ae00eec40a303'
      }
    }
*/

static const uint8_t CSF_SIGNED_CBOR[] = {
    0xa3, 0x01, 0x78, 0x18, 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x73, 0x69, 0x67, 0x6e, 0x65, 0x64,
    0x20, 0x43, 0x42, 0x4f, 0x52, 0x20, 0x77, 0x6f, 0x72, 0x6c, 0x64, 0x21, 0x02, 0x82, 0xf9, 0x40,
    0x00, 0xf5, 0x03, 0xa3, 0x01, 0x27, 0x04, 0xa3, 0x01, 0x01, 0x20, 0x06, 0x21, 0x58, 0x20, 0xfe,
    0x49, 0xac, 0xf5, 0xb9, 0x2b, 0x6e, 0x92, 0x35, 0x94, 0xf2, 0xe8, 0x33, 0x68, 0xf6, 0x80, 0xac,
    0x92, 0x4b, 0xe9, 0x3c, 0xf5, 0x33, 0xae, 0xca, 0xf8, 0x02, 0xe3, 0x77, 0x57, 0xf8, 0xc9, 0x07,
    0x58, 0x40, 0x92, 0xaf, 0xa2, 0x56, 0xcf, 0x20, 0xd6, 0x0c, 0x6c, 0x7d, 0xf8, 0x23, 0x49, 0x26,
    0x79, 0x18, 0x04, 0x6a, 0xcf, 0x48, 0xc4, 0xc9, 0x5b, 0x06, 0xfd, 0x10, 0x11, 0x23, 0xac, 0x12,
    0xb6, 0x2e, 0x01, 0xf0, 0x3b, 0x3f, 0x77, 0x85, 0x57, 0x89, 0x2a, 0x6e, 0x07, 0xdf, 0x43, 0xab,
    0x05, 0xb6, 0x3d, 0x06, 0x23, 0x89, 0x37, 0x81, 0xc7, 0x02, 0x6b, 0x8a, 0xe0, 0x0e, 0xec, 0x40, 
    0xa3, 0x03
};

static const int APP_DATA_ONE_LABEL  = 1;
static const int APP_DATA_TWO_LABEL  = 2;
static const int APP_SIGNATURE_LABEL = 3;

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