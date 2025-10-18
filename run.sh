#!/bin/bash

cd ./build
rm -rf *
cmake ..
make
./http_server
cd ..