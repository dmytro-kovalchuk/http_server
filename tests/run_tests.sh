#!/bin/bash

cd "$(dirname "$0")/.."
rm -rf build/*.so

gcc -fPIC -shared -Iinclude -o build/test_logger.so src/logger.c
gcc -fPIC -shared -Iinclude -o build/test_file_storage.so src/file_storage.c src/logger.c
gcc -fPIC -shared -Iinclude -o build/test_server.so src/*.c

pytest --rootdir=.

rm -rf storage/test.txt