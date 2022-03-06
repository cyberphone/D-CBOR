#!/bin/bash

# Rudimentary Linux bash script for building and running the D-CBOR signature demo.
#
# Run from any directory: $ bash path-to-this-script
#
pushd $(dirname "${BASH_SOURCE[0]}")
gcc -o demo -Os -DPLATFORM_SUPPORTS_FLOAT_CAST -I ../lib -I ed25519/src signature-demo.c signer.c ../lib/*.c ed25519/src/sign.c ed25519/src/verify.c ed25519/src/keypair.c ed25519/src/sha512.c ed25519/src/sc.c ed25519/src/ge.c ed25519/src/fe.c
./demo
popd
