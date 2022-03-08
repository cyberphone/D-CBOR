// csf-signer.c

// Adds a CSF compatible signature to a CBOR map.

#include <d-cbor.h>
#include <ed25519.h>
#include <stdio.h>

#include "csf-signer.h"

static const uint8_t PRIVATE_KEY[] = {
    0xd1, 0xf9, 0x6b, 0xfb, 0xa6, 0xd7, 0xb3, 0x8e, 0x7d, 0x7f, 0xda, 0xb0, 0x02, 0xad, 0xb4, 0x66,
    0xcd, 0xcd, 0x8b, 0x34, 0xc6, 0x20, 0x41, 0xf9, 0xfe, 0xb4, 0xc3, 0x16, 0x8b, 0xa6, 0x15, 0x5e
};

static const uint8_t PUBLIC_KEY[] = {
    0xfe, 0x49, 0xac, 0xf5, 0xb9, 0x2b, 0x6e, 0x92, 0x35, 0x94, 0xf2, 0xe8, 0x33, 0x68, 0xf6, 0x80,
    0xac, 0x92, 0x4b, 0xe9, 0x3c, 0xf5, 0x33, 0xae, 0xca, 0xf8, 0x02, 0xe3, 0x77, 0x57, 0xf8, 0xc9
};

// CSF (CBOR Signature Format)
static const int CSF_ALGORITHM_LABEL  = 1;
static const int CSF_KEY_ID_LABEL     = 3;
static const int CSF_PUBLIC_KEY_LABEL = 4;
static const int CSF_SIGNATURE_LABEL  = 7;

// COSE
static const int COSE_KEY_TYPE_LABEL  = 1;
static const int COSE_OKP_X_LABEL     = -2;
static const int COSE_OKP_CRV_LABEL   = -1;

static const int COSE_KEY_TYPE_OKP    = 1;
static const int COSE_EDDSA_ALG       = -8;  // See note in the code...
static const int COSE_CRV_ED25519     = 6;


void signBuffer(CBOR_BUFFER* cborBuffer, int key) {
    // SUPERCOP parameters
    uint8_t public_key[32], private_key[64], signature[64];
 
    // Set the application specific map key holding the signature.
    addInt(cborBuffer, key);
    // Remember to update map size after the signature has been added.
    int signatureMap = cborBuffer->pos;
    // Intially there are only 2 elements in the core signature map.
    addMap(cborBuffer, 2);
      // COSE algorithm EdDSA but in CSF and FIDO treated as Ed25519.
      addMappedInt(cborBuffer, CSF_ALGORITHM_LABEL, COSE_EDDSA_ALG);
      // COSE public key.  Applications may choose to supply a key ID instead.
      addInt(cborBuffer, CSF_PUBLIC_KEY_LABEL);
      // Public key map is 3 elements for Ed25519
      addMap(cborBuffer, 3);
        // Key type
        addMappedInt(cborBuffer, COSE_KEY_TYPE_LABEL, COSE_KEY_TYPE_OKP);
        // Curve ED25519
        addMappedInt(cborBuffer, COSE_OKP_CRV_LABEL, COSE_CRV_ED25519);
        // X coordinate
        addMappedBstr(cborBuffer, COSE_OKP_X_LABEL, PUBLIC_KEY, sizeof(PUBLIC_KEY));

      // Now we have everything in the buffer that is to be signed.
      ed25519_create_keypair(public_key, private_key, PRIVATE_KEY);
      ed25519_sign(signature, cborBuffer->data, cborBuffer->pos, public_key, private_key);

      // For the demo only...
      if (ed25519_verify(signature, cborBuffer->data, cborBuffer->pos, PUBLIC_KEY)) {
          printf("Valid signature\n");
      }
      else {
          printf("Invalid signature\n");
      }
      printCborBuffer(cborBuffer);
      // End of that...

    // Finally, add the signature blob itself.
    addMappedBstr(cborBuffer, CSF_SIGNATURE_LABEL, signature, sizeof(signature));
    // This may look suspicious but the number of CSF map elements never goes
    // above 5 and the signature blob represents a new entry in the signature map.
    if (cborBuffer->length /* overflow check */) cborBuffer->data[signatureMap]++;
}