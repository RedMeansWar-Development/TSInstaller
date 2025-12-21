#!/bin/bash

echo "Building TS3 Plugin Packager..."
echo ""

mkdir -p build
cd build

echo "Running CMake..."
cmake ..
if [ $? -ne 0 ]; then
    echo "CMake configuration failed!"
    exit 1
fi

echo ""
echo "Building project..."
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)
if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo ""
echo "Build successful!"
echo "Executable location: build/bin/ts3_packager"
echo ""