#!/bin/bash

./build.sh

./build/http_server &
SERVER_PID=$!
sleep 1

echo "=== Uploading file (POST) ==="
curl -v -X POST -T ToDo.txt http://127.0.0.1:8080/ToDo.txt
echo

echo "Stopping server (SIGINT)..."
kill -2 $SERVER_PID
wait $SERVER_PID 2>/dev/null || true
echo "Server stopped!"




./build/http_server &
SERVER_PID=$!
sleep 1

echo "=== Downloading file (GET) ==="
curl -v http://127.0.0.1:8080/ToDo.txt
echo

echo "Stopping server (SIGINT)..."
kill -2 $SERVER_PID
wait $SERVER_PID 2>/dev/null || true
echo "Server stopped!"




./build/http_server &
SERVER_PID=$!
sleep 1

echo "=== Deleting file (DELETE) ==="
curl -v -X DELETE http://127.0.0.1:8080/ToDo.txt
echo

echo "Stopping server (SIGINT)..."
kill -2 $SERVER_PID
wait $SERVER_PID 2>/dev/null || true
echo "All done!"