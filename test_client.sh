#!/bin/bash

./build.sh

./build/http_server &
SERVER_PID=$!
sleep 1

make_requests() {
    local file="file$1.txt"

    timeout 3 curl -s -o /dev/null -H "Connection: keep-alive" \
         -w "Client $1 POST: %{http_code}\n" \
         -X POST -T "$file" "http://127.0.0.1:8080/$file"

    timeout 3 curl -s -o /dev/null -H "Connection: keep-alive" \
         -w "Client $1 GET: %{http_code}\n" \
         "http://127.0.0.1:8080/$file"

    timeout 3 curl -s -o /dev/null -H "Connection: close" \
         -w "Client $1 DELETE: %{http_code}\n" \
         -X DELETE "http://127.0.0.1:8080/$file"
}

NUM_CLIENTS=10
PIDS=()

for i in $(seq 1 $NUM_CLIENTS); do
    touch "file$i.txt"
    make_requests "$i" &
    PIDS+=($!)
done

for pid in "${PIDS[@]}"; do
    wait $pid
done

for i in $(seq 1 $NUM_CLIENTS); do
    rm -f "file$i.txt"
done

kill -2 $SERVER_PID
wait $SERVER_PID 2>/dev/null || true
echo "Server stopped!"