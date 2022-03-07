// verifier.h

#include "qcbor/qcbor_decode.h"
#include "qcbor/qcbor_spiffy_decode.h"

// Returns true for valid signatures.
int csfVerifier(QCBORDecodeContext* pCtx, int key);