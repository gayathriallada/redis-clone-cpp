import socket, time

def send(s, cmd):
    s.sendall((cmd + "\n").encode())
    return s.recv(1024).decode().strip()

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(("127.0.0.1", 6380))

print("SET with 2s TTL:", send(s, "SET temp hello EX 2"))
print("GET immediately (should exist):", send(s, "GET temp"))

print("Waiting 3 seconds for it to expire...")
time.sleep(3)

print("GET after expiry (should be nil):", send(s, "GET temp"))

s.close()
