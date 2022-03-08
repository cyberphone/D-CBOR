// csf-verifier.c

// Verifies a CSF compatible signature in a CBOR map.

// Decoder support: https://github.com/laurencelundblade/QCBOR
// Ed25519 support: https://github.com/orlp/ed25519

#include <assert.h>
#include <ed25519.h>

#include <qcbor/qcbor_decode.h>
#include <qcbor/qcbor_spiffy_decode.h>

#include "print-buffer.h"

// CSF (CBOR Signature Format)
static const int CSF_ALGORITHM_LABEL   = 1;
static const int CSF_KEY_ID_LABEL      = 3;
static const int CSF_PUBLIC_KEY_LABEL  = 4;
static const int CSF_SIGNATURE_LABEL   = 7;

// COSE
static const int COSE_KEY_TYPE_LABEL   = 1;
static const int COSE_OKP_X_LABEL      = -2;
static const int COSE_OKP_CRV_LABEL    = -1;

static const int COSE_KEY_TYPE_OKP     = 1;
static const int COSE_EDDSA_ALG        = -8; // In CSF = Ed25519
static const int COSE_CRV_ED25519      = 6;

int csfVerifier(QCBORDecodeContext* pCtx, int key) {
    // This map entry should contain the CSF object.  Step into it.
    QCBORDecode_EnterMapFromMapN(pCtx, key);

      // Begin CSF algorithm.
    
      // Save the position of the CSF map object.
      // Due to the limited number of CSF elements, it is just a single byte.
      int signatureMap = pCtx->InBuf.cursor - 1;
      // Retrieve signature algorithm.
      // COSE deviation: in CSF/FIDO/PKIX, EDDSA => Ed25519.
      int64_t algorithm;
      QCBORDecode_GetInt64InMapN(pCtx, CSF_ALGORITHM_LABEL, &algorithm);
      assert(algorithm == COSE_EDDSA_ALG);
      // Retrieve COSE public key.
      QCBORDecode_EnterMapFromMapN(pCtx, CSF_PUBLIC_KEY_LABEL);
        // Retrieve COSE key type.
        int64_t keyType;
        QCBORDecode_GetInt64InMapN(pCtx, COSE_KEY_TYPE_LABEL, &keyType);
        assert(keyType == COSE_KEY_TYPE_OKP);
        // OKP type.  Retrieve COSE curve.
        int64_t curveId;
        QCBORDecode_GetInt64InMapN(pCtx, COSE_OKP_CRV_LABEL, &curveId);
        assert(curveId == COSE_CRV_ED25519);
        // Ed25519 public key.  Retrieve COSE X coordinate.
        UsefulBufC x;
        QCBORDecode_GetByteStringInMapN(pCtx, COSE_OKP_X_LABEL, &x);
        assert(x.len == 32);
      // Done with public key.  Step out of public key map.
      QCBORDecode_ExitMap(pCtx);
 
      // Retrieve the signature value.
      UsefulBufC signature;
      int beforeSignature = pCtx->InBuf.cursor;
      QCBORDecode_GetByteStringInMapN(pCtx, CSF_SIGNATURE_LABEL, &signature);
      assert(signature.len == 64);
      assert(pCtx->InBuf.cursor == pCtx->InBuf.UB.len);
 
    // Done with the signature map.  Step out of it.
    QCBORDecode_ExitMap(pCtx);
    assert(QCBORDecode_GetError(pCtx) == QCBOR_SUCCESS);

    // Finalize the CSF algorithm.

    // Remove the signature data and key.
    pCtx->InBuf.UB.len = beforeSignature;
    // Update the signature map object to reflect this change.
    ((uint8_t*)pCtx->InBuf.UB.ptr)[signatureMap]--;
    // Now the buffer holds the data that originally was signed.

    // For debug and demo purposes only...
    printUsefulBufC(&pCtx->InBuf.UB, "Unsigned CBOR data");

    // Finally, does the signature also verify?
    return ed25519_verify(signature.ptr, pCtx->InBuf.UB.ptr, beforeSignature, x.ptr);

    // Note: verifying that the public key is trusted is out of scope for this demo.
}