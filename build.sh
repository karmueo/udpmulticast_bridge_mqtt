#!/bin/bash

# 编译脚本
echo "Building mqtt_sender..."

# 创建build目录
mkdir -p build
cd build

# 运行CMake
cmake ..

# 编译
make -j$(nproc)

echo "Build complete! Executable is located at: build/mqtt_sender"
