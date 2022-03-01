#!/bin/bash

# Rudimentary Linux bash script for building and running the D-CBOR demo.
#
# Run from any directory: $ bash path-to-this-script
#
pushd $(dirname "${BASH_SOURCE[0]}")
gcc -o demo -Os -DPLATFORM_SUPPORTS_FLOAT_CAST -I ../lib d-cbor-4-constrained-device.c ../lib/*.c
./demo
popd
