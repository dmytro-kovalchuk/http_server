#!/bin/bash

cd "$(dirname "$0")/.."
rm -rf build/*.so

gcc -fPIC -shared -Iinclude -o build/test_logger.so src/logger.c src/config.c
gcc -fPIC -shared -Iinclude -o build/test_file_storage.so src/file_storage.c src/logger.c src/config.c
gcc -fPIC -shared -Iinclude -o build/test_server.so src/*.c
gcc -fPIC -shared -Iinclude -o build/test_http_communication.so src/http_communication.c src/logger.c src/file_storage.c src/config.c
gcc -fPIC -shared -Iinclude -o build/test_config.so src/config.c

pytest --rootdir=.

rm -rf storage/test.txt