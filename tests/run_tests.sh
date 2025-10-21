#!/bin/bash

cd "$(dirname "$0")/.."
rm -rf build/*.so

targets=("logger" "file_storage")
for target in ${targets[@]}; do
    gcc -fPIC -shared -Iinclude -o "build/test_${target}.so" "src/${target}.c"
done

pytest --rootdir=.

rm -rf log.txt