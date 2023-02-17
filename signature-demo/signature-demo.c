// signature-demo.c

// Minimalist CSF demo using the D-CBOR PoC encoder.

#include <d-cbor.h>
#include <ed25519.h>

#include "csf-signer.h"

static const int APP_DATA_ONE_LABEL  = 1;
static const int APP_DATA_TWO_LABEL  = 2;
static const int APP_SIGNATURE_LABEL = -1;

#define BUFFER_SIZE 300

int main(int argc, const char* argv[]) {
    (void)argc; // Avoid unused parameter error

    // Buffer setup, here using the stack for storage.
    unsigned char outputBuffer[BUFFER_SIZE];
    CBOR_BUFFER cborBuffer;
    cborBuffer.data = outputBuffer;
    cborBuffer.length = BUFFER_SIZE;
    cborBuffer.pos = 0;

    addMap(&cborBuffer, 3);
      // Application data.
      addMappedTstr(&cborBuffer, APP_DATA_ONE_LABEL, "Hello signed CBOR world!");
      addInt(&cborBuffer, APP_DATA_TWO_LABEL);
      addArray(&cborBuffer, 2);  // [#,#]
        addDouble(&cborBuffer, 2.0);
        addBool(&cborBuffer, 1 /* TRUE */);

      // Lastly, the signature element using an application specific key.
      signBuffer(&cborBuffer, APP_SIGNATURE_LABEL);

    printCborBuffer(&cborBuffer, "Signed CBOR");
}