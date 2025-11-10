# Web Server

## Build
To build, use:
```bash
./build.sh
```
Or if you want manually:
```bash
mkdir -p build
cd ./build
rm -rf *
cmake ..
make
```

## Usage
To run (after build), use:
```bash
./build/http_server
```
Of if you want to build and run in one command:
```bash
./run.sh
```

## Tests
To run Pytests, use:
```bash
./tests/run_tests.sh
```

To test if server is working good, use:
```bash
./test_clients.sh
```