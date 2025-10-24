#!/bin/bash

cd "$(dirname "$0")/.."
rm -rf build/*.so

targets=("logger" "file_storage")
for target in ${targets[@]}; do
    if [ "$target" = "logger" ]; then
        gcc -fPIC -shared -Iinclude -o "build/test_${target}.so" "src/${target}.c"
    else
        gcc -fPIC -shared -Iinclude -o "build/test_${target}.so" "src/${target}.c" "src/logger.c"
    fi
done

pytest --rootdir=.

rm -rf storage/test.txt