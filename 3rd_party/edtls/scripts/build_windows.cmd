SET BUILD_ENV=%1
SET BUILD_DST=%2
SET BUILD_SRC=%3

cmake -G %BUILD_ENV% -B%BUILD_DST% -H%BUILD_SRC% -DBUILD_DTLS_LIB=On -DBUILD_GTEST=On
cmake --build %BUILD_DST%

ctest --build-noclean --extra-verbose --debug --build-and-test %BUILD_SRC% %BUILD_DST% --build-generator %BUILD_ENV% --test-command ctest -C Debug -V || EXIT /B 1