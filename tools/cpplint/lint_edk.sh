#!/bin/bash

cd $( dirname "${BASH_SOURCE[0]}" )/../..

FAKE_GIT_DIR1="libhuestream/.git"
FAKE_GIT_DIR2="tools/.git"
FAKE_GIT_DIR3="examples/.git"
mkdir $FAKE_GIT_DIR1
mkdir $FAKE_GIT_DIR2
mkdir $FAKE_GIT_DIR3
INCLUDE="."
EXCLUDE="cmake-build-debug|cmake|tools|libhuestream\\/test|libhuestream\\/3rd_party|examples\\/huestream_example_gui_win\\/resource.h|examples\\/huestream_example_gui_win\\/stdafx.h|wrappers"

FILES=`find . -name '*.cpp' -or -name '*.h' | egrep "\.\/($INCLUDE)" | egrep -v "$EXCLUDE"`

$1 --filter=-whitespace/ending_newline,-whitespace/end_of_line,-whitespace/line_length,-build/c++11,-whitespace/indent,-runtime/indentation_namespace $FILES 

rmdir $FAKE_GIT_DIR1
rmdir $FAKE_GIT_DIR2
rmdir $FAKE_GIT_DIR3

