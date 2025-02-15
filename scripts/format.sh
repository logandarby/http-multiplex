#!/usr/bin/env bash

echo "Formatting files..."
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
find ${SCRIPT_DIR}/../src -iname '*.h' -o -iname '*.cpp' | xargs clang-format -i
echo "Done"

