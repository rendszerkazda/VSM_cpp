#!/bin/bash
# Configures, builds and tests the project.
# Usage: ./build.sh [Debug|Release]
set -e

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_TYPE=${1:-Debug}
export PATH="$HOME/.local/bin:$PATH"

echo -e "\n== Building in ${BUILD_TYPE} mode =="
cmake -S "$ROOT_DIR" -B "$ROOT_DIR/build" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
cmake --build "$ROOT_DIR/build" -j

echo -e "\n== Running tests =="
ctest --test-dir "$ROOT_DIR/build" --output-on-failure
