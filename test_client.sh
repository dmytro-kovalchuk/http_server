#!/bin/bash

./build.sh

./build/http_server &
SERVER_PID=$!
sleep 1

curl -v --keepalive-time 2 \
  -H "Connection: keep-alive" \
  -X POST -T ToDo.txt http://127.0.0.1:8080/ToDo.txt \
  --next \
  -H "Connection: keep-alive" \
  http://127.0.0.1:8080/ToDo.txt \
  --next \
  -H "Connection: close" \
  -X DELETE http://127.0.0.1:8080/ToDo.txt

kill -2 $SERVER_PID
wait $SERVER_PID 2>/dev/null || true
echo "Server stopped!"