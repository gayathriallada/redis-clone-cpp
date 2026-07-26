# Redis Clone in C++

Building an in-memory key-value store from scratch in C++, inspired by Redis — as a way to learn low-level networking, event loops, and systems programming concepts.

## Status: Work in progress

**Implemented so far:**
- TCP server (socket, bind, listen)

**Planned:**
- Client connection handling (accept)
- Basic commands: GET, SET, DEL
- LRU eviction
- TTL / key expiry
- Disk persistence + write-ahead log
- Benchmarking

## Tech
C++, POSIX Sockets

## Running it
\`\`\`
g++ se
rver.cpp -o server
./server
\`\`\`
