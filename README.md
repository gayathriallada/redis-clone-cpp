# Redis Clone (C++)

An in-memory key-value store built from scratch in C++, modeled conceptually
on Redis. No external libraries or frameworks — the TCP server, event loop,
command parser, and storage engine are all hand-written using raw POSIX
socket calls.

## Why this project

Most applications use Redis without ever touching what's underneath. This
project exists to understand that layer first-hand — how a server accepts
connections, serves many clients without blocking, and handles malformed
input and disconnects without falling over.

## What's built (Weekends 1–3)

**TCP Server Foundation**
- Raw socket creation, binding, listening via socket()/bind()/listen()
- Client handling via accept(), recv(), send(), close()

**Multi-Client Event Loop**
- poll()-based event loop — single thread serving many simultaneous clients
- Same core architectural pattern real Redis and nginx use internally

**Command Protocol**
- `SET key value` → OK
- `GET key` → value, or (nil) if missing
- `DEL key` → 1 (deleted) or 0 (not found)
- Backed by `std::unordered_map<std::string, std::string>`

**Robustness**
- Per-client buffering: correctly handles partial and multi-command TCP reads
  (a single recv() call is never assumed to be exactly one command)
- Malformed command handling: missing args, unknown commands, empty lines —
  all rejected cleanly with clear error messages, no crash
- Clean client disconnect handling, including mid-command disconnects — no
  hangs, no leaked file descriptors, server remains healthy afterward

**Testing**
- Automated Python test client (`robustness_test.py`) — 10 test cases
  covering the happy path, malformed input, and post-error server health,
  all passing

## The hardest bug I hit

Early in Weekend 3, `SET name` (missing value) returned `OK` instead of an
error — and the *next* command sent right after (`GET name`) was misread as
"unknown command," even though GET was working fine seconds earlier.

Root cause: the server read one `recv()` buffer and assumed it contained
exactly one command. But TCP is a byte stream, not a message stream — two
commands sent close together can arrive in a single `recv()` call. Because
`istringstream`'s `>>` treats newlines as whitespace, a value-read that
should have failed instead silently consumed the next line's command word.

Fix: added a per-client buffer that persists across `recv()` calls, splitting
strictly on `\n` and only processing one complete line at a time, leaving
partial data buffered until the rest arrives.

## Architecture

Client 1, Client 2, ... Client N
|
v
poll() event loop (watches all client fds + listening socket)
|
v
Command Parser (istringstream, per-line)
|
v
In-Memory Store (std::unordered_map)


## Running it

```bash
g++ server.cpp -o server
./server
```

Server listens on port 6380. Test manually with `nc 127.0.0.1 6380`, or run
the automated suite:

```bash
python3 robustness_test.py
```

## Roadmap (not yet built)

- Weekend 4: LRU eviction
- Weekend 5: TTL / key expiry
- Weekend 6: Disk persistence
- Weekend 7: Write-ahead log (stretch)
- Weekend 8: Benchmarking + final polish
