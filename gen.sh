#!/bin/bash

# OPTS="--debug-output -Wno-dev"

export PKG_CONFIG_PATH=${HOME}/bin/gattlib/lib/pkgconfig
rm -fr build
mkdir -p build
cd build && cmake ${OPTS} -DCMAKE_BUILD_TYPE=debug ../
