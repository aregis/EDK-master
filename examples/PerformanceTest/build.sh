rm -rf ../../../../build &&
mkdir -p ../../../../build &&
pushd ../../../../build &&
echo "generate build files" &&
cmake .. &&
echo "build" &&
cmake --build . --target PerformanceTest