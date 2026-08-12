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
