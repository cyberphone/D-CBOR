// signature-demo.c

#include <d-cbor.h>

#include <ed25519.h>


void addMappedInt(CBOR_BUFFER* cborBuffer, int key, int value) {
    addInt(cborBuffer, key);
    addInt(cborBuffer, value);
}

void addMappedTstr(CBOR_BUFFER* cborBuffer, int key, const uint8_t* utf8String) {
    addInt(cborBuffer, key);
    addTstr(cborBuffer, utf8String);
}

void addMappedBstr(CBOR_BUFFER* cborBuffer, int key, const uint8_t* byteString, int length) {
    addInt(cborBuffer, key);
    addBstr(cborBuffer, byteString, length);
}

static const uint8_t PRIVATE_KEY[] = {
    0xd1, 0xf9, 0x6b, 0xfb, 0xa6, 0xd7, 0xb3, 0x8e, 0x7d, 0x7f, 0xda, 0xb0, 0x02, 0xad, 0xb4, 0x66,
    0xcd, 0xcd, 0x8b, 0x34, 0xc6, 0x20, 0x41, 0xf9, 0xfe, 0xb4, 0xc3, 0x16, 0x8b, 0xa6, 0x15, 0x5e
};

static const uint8_t PUBLIC_KEY[] = {
    0xfe, 0x49, 0xac, 0xf5, 0xb9, 0x2b, 0x6e, 0x92, 0x35, 0x94, 0xf2, 0xe8, 0x33, 0x68, 0xf6, 0x80,
    0xac, 0x92, 0x4b, 0xe9, 0x3c, 0xf5, 0x33, 0xae, 0xca, 0xf8, 0x02, 0xe3, 0x77, 0x57, 0xf8, 0xc9
};

static void signBuffer(CBOR_BUFFER* cborBuffer, int signatureContainerMap, int key) {
    uint8_t public_key[32], private_key[64], signature[64];
    ed25519_create_keypair(public_key, private_key, PRIVATE_KEY);
    ed25519_sign(signature, cborBuffer->data, cborBuffer->pos, public_key, private_key);
 /*
    if (ed25519_verify(signature, cborBuffer->data, cborBuffer->pos, PUBLIC_KEY)) {
        printf("valid signature\n");
    }
    else {
        printf("invalid signature\n");
    }
    printCborBuffer(cborBuffer);
*/
    addMappedBstr(cborBuffer, key, signature, sizeof(signature));
    // This may look suspicious but the number of map elements never go above 5.
    // The signature represents a new entry in the map.
    cborBuffer->data[signatureContainerMap]++;
}

#define BUFFER_SIZE 300

void main() {
    // Buffer setup, here using the stack for storage.
    unsigned char outputBuffer[BUFFER_SIZE];
    CBOR_BUFFER cborBuffer;
    cborBuffer.data = outputBuffer;
    cborBuffer.length = BUFFER_SIZE;
    cborBuffer.pos = 0;

    addMap(&cborBuffer, 2);
      // Application data
      addMappedTstr(&cborBuffer, 9, "Hi CBOR!");

      // Lastly, the signature element
      addInt(&cborBuffer, 10);
      // Update map size after the signature has been added.
      int signatureContainerMap = cborBuffer.pos;
      addMap(&cborBuffer, 2);
      // COSE algorithm EdDSA
      addMappedInt(&cborBuffer, 1, -8);
      // COSE public key
      addInt(&cborBuffer, 4);
        addMap(&cborBuffer, 3);
          // Key type OKP
          addMappedInt(&cborBuffer, 1, 1);
          // Curve ED25519
          addMappedInt(&cborBuffer, -1, 6);
          // X coordinate
          addMappedBstr(&cborBuffer, -2, PUBLIC_KEY, sizeof(PUBLIC_KEY));
       // Finally, the signature.
       signBuffer(&cborBuffer, signatureContainerMap, 7);
       // End of signature container.

    printCborBuffer(&cborBuffer);
}