#!/bin/bash

./build.sh

./build/http_server &
SERVER_PID=$!
sleep 1

make_requests() {
    local file="file$1.txt"

    curl -s -o /dev/null --keepalive-time 2 \
      -H "Connection: keep-alive" \
      -w "Client $1 POST: %{http_code}\n" \
      -X POST -T "$file" "http://127.0.0.1:8080/$file" \
      --next \
      -H "Connection: keep-alive" \
      -w "Client $1 GET: %{http_code}\n" \
      "http://127.0.0.1:8080/$file" \
      --next \
      -H "Connection: close" \
      -w "Client $1 DELETE: %{http_code}\n" \
      -X DELETE "http://127.0.0.1:8080/$file"
}

NUM_CLIENTS=3
for i in $(seq 1 $NUM_CLIENTS); do
    make_requests "$i" &
done

wait

kill -2 $SERVER_PID
wait $SERVER_PID 2>/dev/null || true
echo "Server stopped!"