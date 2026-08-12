# Redis Clone in C++

An in-memory key-value store built from scratch in C++, inspired by Redis — built to understand low-level networking, event-driven servers, and systems programming concepts by implementing them directly, without any external database or networking library.

## Status: In Progress (Weekends 1–3 of 8 complete)

### Implemented so far

- **TCP server foundation** — raw POSIX sockets: `socket()`, `bind()`, `listen()`, `accept()`, `recv()`, `send()`
- **Multi-client event loop** — a single-threaded `poll()`-based loop handling many simultaneous client connections, the same core architectural pattern used internally by Redis and nginx
- **Command protocol** — `SET key value`, `GET key`, `DEL key`, backed by a `std::unordered_map` for O(1) average-case operations
- **Correct TCP message framing** — incoming data is buffered per client and split strictly on newlines, since a single `recv()` call is not guaranteed to contain exactly one command (a real bug found and fixed during development — see below)
- **Input validation** — missing values, missing keys, and unknown commands are rejected with clear error messages instead of corrupting state or crashing
- **Automated test suite** (`robustness_test.py`) — 10 passing test cases covering the happy path, malformed input, and server health after errors

### Planned (not yet implemented)

- LRU eviction under a configurable memory limit
- TTL / key expiry (`SET key value EX <seconds>`)
- Disk persistence (snapshotting + reload on startup)
- Write-ahead log for crash recovery
- Benchmarking (throughput/latency numbers)

## A Real Bug Worth Mentioning

During the robustness testing pass, `SET name` (missing a value) was found to silently return `OK` while storing an empty string — and worse, it caused the *next* command sent in the same batch to be misread as invalid. Root cause: TCP doesn't guarantee that one `recv()` call maps to exactly one client command — multiple lines can arrive concatenated in a single read. The fix was to buffer incoming bytes per client and only process one complete newline-terminated command at a time, rather than assuming each `recv()` was a clean, single command.

## Tech

C++, POSIX Sockets, `poll()`

## Running It

```bash
g++ server.cpp -o server
./server
```

In another terminal:
```bash
python3 test_client.py          # basic SET/GET/DEL verification
python3 robustness_test.py      # full validation + edge-case suite
```

## Architecture

```
Client 1 ─┐
Client 2 ─┼──► poll() event loop ──► Command Parser ──► In-Memory Store
Client N ─┘      (watches all fds)     (SET/GET/DEL)     (std::unordered_map)
```
