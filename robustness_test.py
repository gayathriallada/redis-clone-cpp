import socket

HOST = "127.0.0.1"
PORT = 6380

def send_and_check(sock, command, expected, description):
    sock.sendall((command + "\n").encode())
    response = sock.recv(1024).decode().strip()
    status = "PASS" if response == expected else "FAIL"
    print(f"[{status}] {description}")
    print(f"    sent:     {command!r}")
    print(f"    expected: {expected!r}")
    print(f"    got:      {response!r}")
    print()

def main():
    sock = socket.create_connection((HOST, PORT))

    send_and_check(sock, "SET name gayathri", "OK", "Basic SET")
    send_and_check(sock, "GET name", "gayathri", "Basic GET")
    send_and_check(sock, "DEL name", "1", "Basic DEL")
    send_and_check(sock, "GET name", "(nil)", "GET after DEL")

    send_and_check(sock, "SET name", "ERROR: SET requires a key and a value", "SET missing value")
    send_and_check(sock, "GET", "ERROR: GET requires a key", "GET missing key")
    send_and_check(sock, "DEL", "ERROR: DEL requires a key", "DEL missing key")
    send_and_check(sock, "BLAHBLAH test", "ERROR: unknown command", "Unknown command")

    send_and_check(sock, "SET check123 works", "OK", "Server still healthy - SET")
    send_and_check(sock, "GET check123", "works", "Server still healthy - GET")

    sock.close()
    print("All tests completed.")

if __name__ == "__main__":
    main()
def test_eviction():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(("127.0.0.1", 6380))

    def send(cmd):
        s.sendall((cmd + "\n").encode())
        return s.recv(1024).decode().strip()

    send("SET a 1")
    send("SET b 2")
    send("SET c 3")
    # With capacity 1000 in production, eviction won't trigger here —
    # this just confirms basic multi-key SET/GET still works after LRU wiring
    result_a = send("GET a")
    result_c = send("GET c")

    if result_a == "1" and result_c == "3":
        print("[PASS] LRU-backed store still handles multiple keys correctly")
    else:
        print(f"[FAIL] Expected a=1, c=3, got a={result_a}, c={result_c}")

    s.close()

test_eviction()
def test_ttl():
    import time
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(("127.0.0.1", 6380))

    def send(cmd):
        s.sendall((cmd + "\n").encode())
        return s.recv(1024).decode().strip()

    send("SET ttlkey val EX 2")
    immediate = send("GET ttlkey")
    time.sleep(3)
    after = send("GET ttlkey")

    if immediate == "val" and after == "(nil)":
        print("[PASS] TTL expiry works correctly")
    else:
        print(f"[FAIL] Expected val then (nil), got {immediate} then {after}")

    s.close()

test_ttl()
