// signature-demo.c

#include <d-cbor.h>

#include <ed25519.h>

#include "signer.h"

#define BUFFER_SIZE 300

int main(int argc, const char* argv[]) {
    (void)argc; // Avoid unused parameter error

    // Buffer setup, here using the stack for storage.
    unsigned char outputBuffer[BUFFER_SIZE];
    CBOR_BUFFER cborBuffer;
    cborBuffer.data = outputBuffer;
    cborBuffer.length = BUFFER_SIZE;
    cborBuffer.pos = 0;

    addMap(&cborBuffer, 2);
      // Application data.
      addMappedTstr(&cborBuffer, 9, "Hello signed CBOR world!");

      // Lastly, the signature element using an application specific key.
      signBuffer(&cborBuffer, 10);

    printCborBuffer(&cborBuffer);
}