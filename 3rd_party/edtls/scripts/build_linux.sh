#!/bin/bash

set -e
set -o pipefail

BUILD_DIR=build_linux

rm -rf $BUILD_DIR
mkdir $BUILD_DIR
cd $BUILD_DIR
cmake  -DBUILD_DTLS_LIB=On -DBUILD_GTEST=On ..
cmake --build . -- -j 8
ctest -C Debug -V
