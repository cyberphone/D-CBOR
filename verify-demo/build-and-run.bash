#!/bin/bash

# Rudimentary Linux bash script for building and running the D-CBOR signature demo.
#
# Run from any directory: $ bash path-to-this-script
#
pushd $(dirname "${BASH_SOURCE[0]}")
gcc -o demo -fPIC -Os -DPLATFORM_SUPPORTS_FLOAT_CAST -I ../QCBOR/inc -I ../ed25519/src verify-demo.c csf-verifier.c print-buffer.c ../lib/*.c ../ed25519/src/sign.c ../ed25519/src/verify.c ../ed25519/src/keypair.c ../ed25519/src/sha512.c ../ed25519/src/sc.c ../ed25519/src/ge.c ../ed25519/src/fe.c ../QCBOR/src/ieee754.c ../QCBOR/src/qcbor_decode.c ../QCBOR/src/UsefulBuf.c -lm
./demo
popd
