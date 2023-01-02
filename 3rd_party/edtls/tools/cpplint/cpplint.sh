#!/bin/bash

cd $( dirname "${BASH_SOURCE[0]}" )/../../

FAKE_GIT_DIR1="libedtls/.git"
mkdir $FAKE_GIT_DIR1
INCLUDE="libedtls"
EXCLUDE="cmake-build-debug|cmake|tools|libedtls\/edtls\/wrapper\/mbedtls\/custom_config.h"

FILES=`find . -name '*.cpp' -or -name '*.h' | egrep "\.\/($INCLUDE)" | egrep -v "$EXCLUDE"`

tools/cpplint/cpplint.py --filter=-whitespace/ending_newline,-whitespace/end_of_line,-whitespace/line_length,-build/c++11,-whitespace/indent,-runtime/indentation_namespace $FILES 

rmdir $FAKE_GIT_DIR1

