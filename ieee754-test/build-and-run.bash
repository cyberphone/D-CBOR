#!/bin/bash

# Rudimentary Linux bash script for building and running the D-CBOR IEEE754 test.
#
# Run from any directory: $ bash path-to-this-script
#
pushd $(dirname "${BASH_SOURCE[0]}")
gcc -o demo -Os -DPLATFORM_SUPPORTS_FLOAT_CAST -I ../lib ieee754-test.c ../lib/*.c
./demo
popd
