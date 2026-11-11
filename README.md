# Redis Clone in C++

An in-memory key-value store built from scratch in C++, inspired by Redis. Built to understand low-level networking, event-driven servers, and durability by implementing them directly, with no external database or networking library.

## Features

- **TCP server on raw POSIX sockets**: `socket()`, `bind()`, `listen()`, `accept()`, `recv()`, `send()`, with `SO_REUSEADDR` for clean restarts
- **Single-threaded `poll()` event loop** handling many simultaneous clients, the same core pattern used by Redis and nginx
- **Commands**: `SET key value [EX seconds]`, `GET key`, `DEL key`, `SAVE`
- **Correct TCP framing**: incoming bytes are buffered per client and split strictly on newlines (a real bug found and fixed during testing, see below)
- **Input validation**: malformed commands return clear errors instead of corrupting state
- **LRU eviction**: hash map + doubly linked list gives O(1) access and eviction at a configurable capacity (200,000 keys)
- **TTL / key expiry**: lazy expiry on access plus an active background sweep
- **Snapshot persistence**: `SAVE` writes a binary snapshot (`dump.rdb`), loaded automatically on startup
- **Write-ahead log**: every SET and DEL is appended to `wal.log`, replayed on startup after the snapshot loads, and truncated after `SAVE`. Verified by writing keys, deleting one, killing the server with `kill -9`, restarting, and confirming the data was recovered.

## Benchmarks

Measured on a single client connection using a Python client (`bench.py`, `reload_bench.py`), 20,000 requests per run:

| Operation | Throughput | p50 latency | p99 latency |
|-----------|-----------|-------------|-------------|
| SET | ~10.8k req/sec | 0.076 ms | 0.209 ms |
| GET | ~13.3k req/sec | 0.056 ms | 0.225 ms |

Reload of a 100,000-key snapshot (4.2 MB) on startup: **75 ms**.

## Known Limitations

- The WAL is flushed to the OS on every write but not `fsync`ed, so it survives a process crash (`kill -9`) but not a power loss
- Benchmarks use one connection and a Python client, so they measure round-trip latency rather than peak server throughput
- No RESP protocol; commands are plain text over TCP

## A Real Bug Worth
cat > README.md <<'EOF'
# Redis Clone in C++

An in-memory key-value store built from scratch in C++, inspired by Redis. Built to understand low-level networking, event-driven servers, and durability by implementing them directly, with no external database or networking library.

## Features

- **TCP server on raw POSIX sockets**: `socket()`, `bind()`, `listen()`, `accept()`, `recv()`, `send()`, with `SO_REUSEADDR` for clean restarts
- **Single-threaded `poll()` event loop** handling many simultaneous clients, the same core pattern used by Redis and nginx
- **Commands**: `SET key value [EX seconds]`, `GET key`, `DEL key`, `SAVE`
- **Correct TCP framing**: incoming bytes are buffered per client and split strictly on newlines (a real bug found and fixed during testing, see below)
- **Input validation**: malformed commands return clear errors instead of corrupting state
- **LRU eviction**: hash map + doubly linked list gives O(1) access and eviction at a configurable capacity (200,000 keys)
- **TTL / key expiry**: lazy expiry on access plus an active background sweep
- **Snapshot persistence**: `SAVE` writes a binary snapshot (`dump.rdb`), loaded automatically on startup
- **Write-ahead log**: every SET and DEL is appended to `wal.log`, replayed on startup after the snapshot loads, and truncated after `SAVE`. Verified by writing keys, deleting one, killing the server with `kill -9`, restarting, and confirming the data was recovered.

## Benchmarks

Measured on a single client connection using a Python client (`bench.py`, `reload_bench.py`), 20,000 requests per run:

| Operation | Throughput | p50 latency | p99 latency |
|-----------|-----------|-------------|-------------|
| SET | ~10.8k req/sec | 0.076 ms | 0.209 ms |
| GET | ~13.3k req/sec | 0.056 ms | 0.225 ms |

Reload of a 100,000-key snapshot (4.2 MB) on startup: **75 ms**.

## Known Limitations

- The WAL is flushed to the OS on every write but not `fsync`ed, so it survives a process crash (`kill -9`) but not a power loss
- Benchmarks use one connection and a Python client, so they measure round-trip latency rather than peak server throughput
- No RESP protocol; commands are plain text over TCP

## A Real Bug Worth Mentioning

During robustness testing, `SET name` (missing a value) silently returned `OK` while storing an empty string, and caused the next command in the same batch to be misread. Root cause: TCP does not guarantee that one `recv()` maps to one command, because multiple lines can arrive concatenated in a single read. The fix was to buffer incoming bytes per client and process only one complete newline-terminated command at a time.

## Tech

C++17, POSIX sockets, `poll()`

## Running It

```bash
g++ -std=c++17 -O2 server.cpp -o server
./server        # listens on port 6380
```

In another terminal:
```bash
python3 test_client.py          # basic SET/GET/DEL checks
python3 robustness_test.py      # validation and edge-case suite
python3 ttl_test.py             # key expiry
python3 wal_check.py "SET a 1"  # send commands manually
python3 bench.py                # throughput / latency
python3 reload_bench.py         # 100k-key snapshot reload time
```

## Architecture

```
Client 1 ─┐
Client 2 ─┼──► poll() event loop ──► Command Parser ──► LRU Store ──► WAL (wal.log)
Client N ─┘      (watches all fds)   (SET/GET/DEL/SAVE)  (hash map +      + Snapshot (dump.rdb)
                                                          linked list)
```
