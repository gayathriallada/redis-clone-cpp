# Redis Clone in C++

![tests](https://github.com/gayathriallada/redis-clone-cpp/actions/workflows/tests.yml/badge.svg)

An in-memory key-value store built from scratch in C++, modeled on Redis. It uses no database or networking libraries, only raw POSIX sockets. It listens on port 6380 and accepts plain-text commands from any client.

## Features

- Single-threaded `poll()` event loop serving many clients at once
- `SET`, `GET`, `DEL`, with optional expiry (`SET key value EX seconds`)
- O(1) LRU eviction (hash map + doubly linked list), capacity 200,000 keys
- TTL expiry, both lazy (checked on read) and active (periodic sweep)
- Durability: `SAVE` snapshots to `dump.rdb`, and a write-ahead log (`wal.log`) is replayed on startup
- Per-client buffering, so partial and combined reads are handled correctly
- Automated tests, a Dockerfile, and CI on every push

## Architecture

```
Client 1 ─┐
Client 2 ─┼──► poll() event loop ──► Command Parser ──► LRU Store ──► WAL (wal.log)
Client N ─┘      (watches all fds)   (SET/GET/DEL/SAVE)  (hash map +      + Snapshot (dump.rdb)
                                                          linked list)
```

## Quick start

Build and run:

```bash
g++ server.cpp -o server
./server
```

Or with Docker:

```bash
docker build -t redis-clone .
docker run -p 6380:6380 -v redis-data:/data redis-clone
```

Connect from another terminal and type commands:

```bash
nc 127.0.0.1 6380
SET name gayathri
GET name
```

## Commands

| Command | Description | Reply |
|---|---|---|
| `SET key value` | Store a value | `OK` |
| `SET key value EX seconds` | Store a value that expires after the given seconds | `OK` |
| `GET key` | Read a value | the value, or `(nil)` if missing or expired |
| `DEL key` | Delete a key | `1` if deleted, `0` if not found |
| `SAVE` | Write a snapshot to `dump.rdb` | `OK` |

A missing key, missing value, bad `EX` value or unknown command returns an error message and leaves the data untouched.

## How it works

### Networking and framing
The server uses `socket()`, `bind()`, `listen()`, `accept()`, `recv()` and `send()`, with `SO_REUSEADDR` so it can restart on the same port immediately. One `poll()` loop watches every connection, so there are no threads and no locks.

A single `recv()` can return half a command or several commands. Incoming bytes are buffered per client and split only on newlines, so each complete line is processed as exactly one command.

### Storage
`lru_cache.h` pairs a hash map (key to node) with a doubly linked list ordered by recency. Lookups, inserts and evictions are O(1). Reading a key moves it to the front. When the store is full, the node at the tail (least recently used) is evicted.

### Expiry
- **Lazy:** an expired key is removed when someone tries to read it.
- **Active:** `sweepExpired()` runs periodically in the event loop and removes expired keys that nobody is reading.

### Persistence
- **Snapshot:** `SAVE` writes every key, value and expiry to `dump.rdb` in a length-prefixed binary format. It is loaded automatically on startup.
- **Write-ahead log:** every `SET` and `DEL` is appended to `wal.log` as it happens, for example `SET c 3 1794292676` and `DEL b`. On startup the server loads the snapshot, then replays the log, skipping expired entries. The log is cleared after each `SAVE`, because the snapshot then contains everything.

## Testing

Start the server in one terminal, then run the scripts from another.

| Script | What it checks |
|---|---|
| `robustness_test.py` | Normal use, malformed input, server health after errors, plus LRU, TTL and `SAVE` checks. This is the script CI runs. |
| `lru_test.py`, `test_lru.cpp` | Eviction order and edge cases |
| `ttl_test.py` | Lazy expiry |
| `active_ttl_test.py` | Active expiry: a key disappears without anyone reading it |
| `wal_check.py` | Crash recovery: write keys, delete one, `kill -9` the server, restart, and confirm `a` and `c` came back and `b` stayed deleted |
| `bench.py`, `reload_bench.py` | Throughput, latency and snapshot reload time |

GitHub Actions builds the server and runs the tests on every push, and also builds the Docker image and runs the tests against the container.

## Benchmarks

| Measurement | Result |
|---|---|
| `SET` | ~10.8k requests/sec, p99 0.21 ms |
| `GET` | ~13.3k requests/sec, p99 0.23 ms |
| Reload of a 100,000-key snapshot (4.2 MB) | 75 ms |

These were measured on a single connection with a Python client, so they show round-trip latency, not the server's peak throughput.

## Bugs found and fixed

1. **TCP framing.** `SET name` with a missing value corrupted the next command, because the server assumed one `recv()` was one command. Fixed with per-client buffering and newline framing.
2. **Port reuse.** The server could not restart right after being killed, because the port was still held. Fixed with `SO_REUSEADDR`.
3. **Benchmark validity.** The first reload benchmark reported 6 ms. The snapshot file was 42 KB instead of about 4 MB, because the cache capacity was only 1,000 and 99k keys had been silently evicted. I raised the capacity and reran it.

## Design trade-offs

- **`poll()` instead of `epoll`:** `poll()` scans every file descriptor on each call, so its cost grows with the number of connections. That is fine at tens or hundreds of clients, and it is portable and simple. For thousands of connections I would move to `epoll`.
- **Single-threaded:** no locks, and each command is atomic by construction, which keeps LRU and WAL ordering easy to reason about. The cost is one core, and a slow operation blocks every client. `SAVE` runs on the event-loop thread, so clients wait while it writes. Real Redis forks a child process for background saves.
- **WAL without `fsync`:** writes stay fast. The log survives a process crash (tested with `kill -9`) but not a power loss.
- **Plain-text protocol, not RESP:** easy to debug with `nc` or a script, but real Redis clients cannot connect.
- **Capacity counts keys, not bytes:** large values can use far more memory than small ones. Redis limits by bytes with `maxmemory`.
- **Benchmarks:** a single connection and a Python client, so they reflect round-trip latency rather than peak server throughput.

### What I would change at scale
`epoll`, a configurable `fsync` policy, background snapshots via `fork()`, RESP support, a multi-connection benchmark, then replication.
