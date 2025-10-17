#!/bin/bash

cd "$(dirname "$0")/.."
rm -rf build/*.so

gcc -fPIC -shared -Iinclude -o build/test_logger.so src/logger.c

pytest --rootdir=.

rm -rf log.txt